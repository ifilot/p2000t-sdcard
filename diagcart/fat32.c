/**************************************************************************
 *                                                                        *
 *   Author: Ivo Filot <ivo@ivofilot.nl>                                  *
 *                                                                        *
 *   P2000T-SDCARD is free software:                                      *
 *   you can redistribute it and/or modify it under the terms of the      *
 *   GNU General Public License as published by the Free Software         *
 *   Foundation, either version 3 of the License, or (at your option)     *
 *   any later version.                                                   *
 *                                                                        *
 *   P2000T-SDCARD is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#include "fat32.h"

uint16_t _bytes_per_sector = 0;
uint8_t _sectors_per_cluster = 0;
uint16_t _reserved_sectors = 0;
uint8_t _number_of_fats = 0;
uint32_t _sectors_per_fat = 0;
uint32_t _root_dir_first_cluster = 0;
uint32_t _linkedlist[16];
uint32_t _fat_begin_lba = 0;
uint32_t _SECTOR_begin_lba = 0;
uint32_t _lba_addr_root_dir = 0;
uint32_t _filesize_current_file = 0;
uint32_t _current_folder_cluster = 0;
char _basename[9];
char _ext[4];
uint8_t _current_attrib = 0;

/**
 * @brief Read the Master Boot Record
 * 
 * @param verbose whether to return verbose output to terminal
 * @return uint32_t start sector-address of the first sector
 */
uint32_t read_mbr(void) {
    read_sector(0x00000000);
    return ram_read_uint32_t(SDCACHE0 + 446 + 8);
}

/**
 * @brief Read metadata of the partition
 * 
 * @param lba0 address of the partition
 */
void read_partition(uint32_t lba0) {
    // inform the reader that we are about to read partition 1
    print("Reading partition 1");

    // read the volume ID (first sector of the partition)
    read_sector(lba0);

    // collect data
    _bytes_per_sector = ram_read_uint16_t(SDCACHE0 + 0x0B);
    _sectors_per_cluster = ram_read_byte(SDCACHE0 + 0x0D);
    _reserved_sectors = ram_read_uint16_t(SDCACHE0 + 0x0E);
    _number_of_fats = ram_read_byte(SDCACHE0 + 0x10);
    _sectors_per_fat = ram_read_uint32_t(SDCACHE0 + 0x24);
    _root_dir_first_cluster = ram_read_uint32_t(SDCACHE0 + 0x2C);
    _current_folder_cluster = _root_dir_first_cluster;
    uint16_t signature = ram_read_uint16_t(SDCACHE0 + 0x1FE);

    // print data
    // sprintf(termbuffer, "LBA partition 1:%c%08lX", COL_GREEN, lba0);
    // terminal_printtermbuffer();

    sprintf(termbuffer, "Bytes per sector:%c%i", COL_GREEN, _bytes_per_sector);
    terminal_printtermbuffer();

    // sprintf(termbuffer, "Sectors per cluster:%c%i", COL_GREEN, _sectors_per_cluster);
    // terminal_printtermbuffer();

    // sprintf(termbuffer, "Reserved sectors:%c%i", COL_GREEN, _reserved_sectors);
    // terminal_printtermbuffer();

    // sprintf(termbuffer, "Number of FATS:%c%i", COL_GREEN, _number_of_fats);
    // terminal_printtermbuffer();

    // sprintf(termbuffer, "Sectors per FAT:%c%lu", COL_GREEN, _sectors_per_fat);
    // terminal_printtermbuffer();

    // calculate the total capacity on the partition; this corresponds to the
    // each FAT holds a number of sectors
    // each sector can refer to 128 clusters (128 x 32 = 512 bytes)
    // each cluster hosts a number of sectors
    // each sector has a specific sectors size (512 bytes for FAT32)
    sprintf(termbuffer, "Partition size:%c%lu MiB", COL_GREEN, (_sectors_per_fat * _sectors_per_cluster * _bytes_per_sector) >> 13 );
    terminal_printtermbuffer();

    // sprintf(termbuffer, "Root first cluster:%c%08lX", COL_GREEN, _root_dir_first_cluster);
    // terminal_printtermbuffer();

    // sprintf(termbuffer, "Signature:%c%04X", COL_GREEN, signature);
    // terminal_printtermbuffer();

    // consolidate variables
    _fat_begin_lba = lba0 + _reserved_sectors;
    _SECTOR_begin_lba = lba0 + _reserved_sectors + (_number_of_fats * _sectors_per_fat);
    _lba_addr_root_dir = get_sector_addr(_root_dir_first_cluster, 0);

    // read first sector of first partition to establish volume name
    read_sector(_lba_addr_root_dir);

    // volume name is written as the first 11 bytes
    char volume_name[11];
    copy_from_ram(SDCACHE0, volume_name, 11);
    sprintf(termbuffer, "Volume name:%c%.11s", COL_GREEN, volume_name);
    terminal_printtermbuffer();
    memcpy(&vidmem[0x50+39-11], volume_name, 11);

    _flag_sdcard_mounted = 1;
}

/**
 * @brief Read the contents of the root folder and search for a file identified 
 *        by file id. When a negative file_id is supplied, the directory is
 *        simply scanned and the list of files are outputted to the screen.
 * 
 * @param file_id ith file in the folder
 * @return uint32_t first cluster of the file or directory
 */
uint32_t read_folder(uint32_t cluster, int16_t file_id, uint8_t casrun) {
    // build linked list for the root directory
    build_linked_list(cluster);

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over sectors
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint32_t totalfilesize = 0;
    uint8_t stopreading = 0;
    uint8_t fileblock[32];          // storage for single file
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && stopreading == 0) {
        
        // print cluster number and address
        const uint32_t caddr = get_sector_addr(_linkedlist[ctr], 0);
        // sprintf(termbuffer, "Cluster %i:%c%08lX%c%08lX", ctr+1, COL_YELLOW, _linkedlist[ctr], COL_CYAN, caddr);
        // terminal_printtermbuffer();

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr + i); // read sector data
            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector

                // grab file metadata
                copy_from_ram(j*32, fileblock, 32);

                // early exit if a zero is read
                if(fileblock[0] == 0x00) {
                    stopreading = 1;
                    break;
                }

                // continue if an unused entry is encountered 0xE5
                if(fileblock[0] == 0xE5) {
                    continue;
                }

                const uint8_t attrib = fileblock[0x0B];

                // if lower five bits of byte 0x0B of file table is unset
                // assume we are reading a file and try to decode it
                if((attrib & 0x0F) == 0x00) {

                    // capture metadata
                    fctr++;
                    const uint16_t fch = read_uint16_t(&fileblock[0x14]);
                    const uint16_t fcl = read_uint16_t(&fileblock[0x1A]);
                    const uint32_t fc = (uint32_t)fch << 16 | fcl;
                    const uint32_t filesize = read_uint32_t(&fileblock[28]);
                    totalfilesize += filesize;

                    if(file_id < 0) {
                        if(attrib & (1 << 4)) { // directory entry
                            sprintf(termbuffer, "%c%3u%c%.8s DIR       %c%08lX", COL_YELLOW, fctr, COL_WHITE, &fileblock[0x00], COL_CYAN, fc);
                            terminal_printtermbuffer();
                        } else {                // file entry
                            if(casrun == 1 && memcmp(&fileblock[0x08], "CAS", 3) == 0) {    // cas file
                                // read from SD card once more and extract CAS data
                                open_command();
                                cmd17(get_sector_addr(fc, 0));
                                fast_sd_to_ram_first_0x100(SDCACHE1);
                                close_command();

                                // grab CAS metadata
                                uint8_t casname[16];
                                uint8_t ext[3];
                                copy_from_ram(SDCACHE1 + 0x36, casname, 8);
                                copy_from_ram(SDCACHE1 + 0x47, &casname[8], 8);
                                copy_from_ram(SDCACHE1 + 0x3E, ext, 3);

                                // replace terminating characters (0x00) by spaces (0x20)
                                replace_bytes(casname, 0x00, 0x20, 16);
                                replace_bytes(ext, 0x00, 0x20, 3);

                                const uint16_t filesize = ram_read_uint16_t(SDCACHE1 + 0x32);
                                const uint8_t blocks = ram_read_byte(SDCACHE1 + 0x4F);

                                // print result for a CAS file with metadata information
                                sprintf(termbuffer, "%c%3u%c%.16s %.3s%c%2i %6u", COL_GREEN, fctr, COL_YELLOW, casname, ext, COL_CYAN, blocks, filesize);
                                terminal_printtermbuffer();
                            } else { // non-cas file or not a cas run
                                // print result for a regular file
                                sprintf(termbuffer, "%c%3u%c%.8s.%.3s%c%6lu%c%08lX", COL_GREEN, fctr, COL_WHITE, &fileblock[0x00], &fileblock[0x08], COL_YELLOW, filesize, COL_CYAN, fc);
                                terminal_printtermbuffer();
                            }
                        }

                        if(fctr % 16 == 0) {
                            print_recall("-- Press key to continue, q to quit --");
                            if(wait_for_key_fixed(3) == 1) {
                                stopreading = 1;
                                break;
                            }
                        }
                    }

                    if(fctr == file_id) {
                        store_file_metadata(j);
                        return fc;
                    }
                }
            }
        }
        ctr++;
    }

    if(file_id < 0) {
        sprintf(termbuffer, "%6u File(s) %10lu Bytes", fctr, totalfilesize);
        terminal_printtermbuffer();
    } else {
        sprintf(termbuffer, "%c File id > %6u", fctr);
        terminal_printtermbuffer();
    }

    return _root_dir_first_cluster;
}

/**
 * @brief Find a file identified by BASENAME and EXT in the folder correspond
 *        to the cluster address
 * 
 * @param cluster   cluster address
 * @param basename  first 8 bytes of the file
 * @param ext       3 byte extension of the file
 * @return uint32_t cluster address of the file or 0 if not found
 */
uint32_t find_file(uint32_t cluster, const char* basename_find, const char* ext_find) {
    // build linked list for the root directory
    build_linked_list(cluster);

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over sectors
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint8_t stopreading = 0;
    uint8_t fileblock[32];          // storage for single file
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && stopreading == 0) {
        
        // get cluster address
        const uint32_t caddr = get_sector_addr(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr + i); // read sector data
            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector

                // grab file metadata
                copy_from_ram(SDCACHE0 + j * 32, fileblock, 32);

                // early exit if a zero is read
                if(fileblock[0] == 0x00) {
                    stopreading = 1;
                    break;
                }

                // continue if an unused entry is encountered 0xE5
                if(fileblock[0] == 0xE5) {
                    continue;
                }

                uint8_t attrib = fileblock[0x0B];

                // if lower five bits of byte 0x0B of file table is unset
                // assume we are reading a file and try to decode it
                if((attrib & 0x0F) == 0x00) {
                    fctr++;

                    if(memcmp(basename_find, fileblock, 8) == 0 && 
                       memcmp(ext_find, &fileblock[8], 3) == 0) {

                        uint16_t fch = read_uint16_t(&fileblock[0x14]);
                        uint16_t fcl = read_uint16_t(&fileblock[0x1A]);
                        uint32_t fc = (uint32_t)fch << 16 | fcl;
                        uint32_t filesize = read_uint32_t(&fileblock[28]);

                        _filesize_current_file = filesize;

                        return fc;
                    }
                }
            }
        }
        ctr++;
    }

    return 0;
}

/**
 * @brief Find a file identified by BASENAME and EXT in the folder correspond
 *        to the cluster address
 * 
 * @param cluster   cluster address
 * @param basename  first 8 bytes of the file
 * @param ext       3 byte extension of the file
 * @return uint32_t cluster address of the file or 0 if not found
 */
uint32_t find_folder(uint32_t cluster, const char* basename_find) {
    // build linked list for the root directory
    build_linked_list(cluster);

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over sectors
    uint8_t stopreading = 0;
    uint8_t fileblock[32];          // storage for single file
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && stopreading == 0) {
        
        // get cluster address
        const uint32_t caddr = get_sector_addr(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr + i); // read sector data
            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector

                // grab file metadata
                copy_from_ram(j*32, fileblock, 32);

                // early exit if a zero is read
                if(fileblock[0] == 0x00) {
                    stopreading = 1;
                    break;
                }

                // continue if an unused entry is encountered 0xE5
                if(fileblock[0] == 0xE5) {
                    continue;
                }

                const uint8_t attrib = fileblock[0x0B];

                // if lower five bits of byte 0x0B of file table is unset
                // assume we are reading a file and try to decode it
                if((attrib & 0x0F) == 0x00 && attrib & (1 << 4)) {

                    if(memcmp(basename_find, fileblock, strlen(basename_find)) == 0) {
                        const uint16_t fch = read_uint16_t(&fileblock[0x14]);
                        const uint16_t fcl = read_uint16_t(&fileblock[0x1A]);
                        const uint32_t fc = (uint32_t)fch << 16 | fcl;

                        return fc;
                    }
                }
            }
        }
        ctr++;
    }

    return 0;
}

/**
 * @brief Build a linked list of sector addresses starting from a root address
 * 
 * @param nextcluster first cluster in the linked list
 */
void build_linked_list(uint32_t nextcluster) {
    // counter over clusters
    uint8_t ctr = 0;

    // clear previous linked list
    memset(_linkedlist, 0xFF, LINKEDLIST_SIZE * sizeof(uint32_t));

    // try grabbing next cluster
    while(nextcluster < 0x0FFFFFF8 && nextcluster != 0 && ctr < LINKEDLIST_SIZE) {
        _linkedlist[ctr] = nextcluster;
        read_sector(_fat_begin_lba + (nextcluster >> 7));
        uint8_t item = nextcluster & 0b01111111;
        nextcluster = ram_read_uint32_t(item * 4);
        ctr++;
    }
}

/**
 * @brief Calculate the sector address from cluster and sector
 * 
 * @param cluster which cluster
 * @param sector which sector on the cluster (0-Nclusters)
 * @return uint32_t sector address (512 byte address)
 */
uint32_t get_sector_addr(uint32_t cluster, uint8_t sector) {
    return _SECTOR_begin_lba + (cluster - 2) * _sectors_per_cluster + sector;   
}

/**
 * @brief Store entry metadata in special global variables
 * 
 * @param entry_id entry id with respect to current sector data
 * @return uint32_t pointer to first cluster
 */
uint32_t store_file_metadata(uint8_t entry_id) {
    _filesize_current_file = ram_read_uint32_t(SDCACHE0 + entry_id * 32 + 28);
    copy_from_ram(entry_id*32+0x00, _basename, 8);
    _basename[8] = 0x00; // terminating byte
    copy_from_ram(entry_id*32+0x08, _ext, 3);
    _ext[3] = 0x00; // terminating byte
    _current_attrib = ram_read_byte(SDCACHE0 + entry_id * 32 + 0x0B);

    uint16_t fch = ram_read_uint16_t(SDCACHE0 + entry_id * 32 + 0x14);
    uint16_t fcl = ram_read_uint16_t(SDCACHE0 + entry_id * 32 + 0x1A);
    uint32_t fc = (uint32_t)fch << 16 | fcl;
    return fc;
}