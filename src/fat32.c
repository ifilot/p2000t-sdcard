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
char _ext[4] = {0};
uint8_t _filename[MAX_LFN_LENGTH+1];
uint8_t _current_attrib = 0;

/**
 * @brief Read the Master Boot Record
 * 
 * @return uint32_t sector-address of the first partition
 */
uint32_t read_mbr(void) {
    // read the first sector of the SD card
    read_sector(0x00000000);

    if(ram_read_uint16_t(SDCACHE0 + 510) != 0xAA55) {
        return 0;
    } else {
        return ram_read_uint32_t(SDCACHE0 + 0x1C6);
    }
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
    _sectors_per_cluster = ram_read_uint8_t(SDCACHE0 + 0x0D);
    _reserved_sectors = ram_read_uint16_t(SDCACHE0 + 0x0E);
    _number_of_fats = ram_read_uint8_t(SDCACHE0 + 0x10);
    _sectors_per_fat = ram_read_uint32_t(SDCACHE0 + 0x24);
    _root_dir_first_cluster = ram_read_uint32_t(SDCACHE0 + 0x2C);
    _current_folder_cluster = _root_dir_first_cluster;
    uint16_t signature = ram_read_uint16_t(SDCACHE0 + 0x1FE);

    // print data
    // sprintf(termbuffer, "LBA partition 1:%c%08lX", COL_GREEN, lba0);
    // terminal_printtermbuffer();

    // sprintf(termbuffer, "Bytes per sector:%c%i", COL_GREEN, _bytes_per_sector);
    // terminal_printtermbuffer();

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
    _lba_addr_root_dir = calculate_sector_address(_root_dir_first_cluster, 0);

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
uint32_t read_folder(int16_t file_id, uint8_t casrun) {

    // build linked list for the root directory
    build_linked_list(_current_folder_cluster);

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over clusters
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint32_t totalfilesize = 0;     // collect size of files in folder
    uint8_t stopreading = 0;        // whether to break of reading procedure
    uint16_t loc = 0;               // current entry position
    uint8_t firstPos = 0;
    uint8_t attrPos = 0;            // check byte
    uint8_t k = 0;                  // unsigned temp counter
    uint8_t lfn_found = 0;

    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE && stopreading == 0) {
        
        // print cluster number and address
        uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr);            // read sector data
            loc = SDCACHE0;
            for(uint8_t j=0; j<16; j++) { // 16 file tables per sector
                // check first position
                firstPos = ram_read_uint8_t(loc);
                attrPos = ram_read_uint8_t(loc + 0x0B);    // attrib byte

                // continue if an unused entry is encountered 0xE5
                if(firstPos == 0xE5) {
                    loc += 32;  // next file entry location
                    continue;
                }

                // early exit if a zero is read
                if(firstPos == 0x00) {
                    stopreading = 1;
                    break;
                }

                // check for LFN entry
                if ((attrPos & 0x0F) == 0x0F) {
                    if (!lfn_found) {
                        lfn_found = 1;  // indicate LNF found
                        memset(_filename, 0, MAX_LFN_LENGTH+1);
                    }
                    uint8_t seq = firstPos & 0x1F;  // LFN sequence number
                    if (seq <= 2) {
                        // extract characters from LFN entry
                        for (k = 0; k < 5; k++) _filename[(seq - 1) * 13 + k] = ram_read_uint8_t(loc + 1 + k * 2);
                        for (k = 0; k < 6; k++) _filename[(seq - 1) * 13 + 5 + k] = ram_read_uint8_t(loc + 14 + k * 2);
                        for (k = 0; k < 2; k++) _filename[(seq - 1) * 13 + 11 + k] = ram_read_uint8_t(loc + 28 + k * 2);
                    }
                }

                // check for SFN entry
                if((attrPos & 0x0F) == 0x00) {
                    // capture metadata
                    fctr++;
                    const uint32_t fc = grab_cluster_address_from_fileblock(loc);
                    const uint32_t filesize = ram_read_uint32_t(loc + 0x1C);
                    totalfilesize += filesize;

                    if(file_id < 0) {
                        // copy extension (uppercase)
                        copy_from_ram(loc+0x08, _ext, 3);

                        // if no LFN found, the SFN filename needs to be formatted
                        if (!lfn_found) {
                            copy_from_ram(loc, _filename, 8);
                            memcpy(&_filename[9], _ext, 4); // copy extension (incl terminator)
                            // if file, inject dot before extension
                            _filename[8] = (attrPos & 0x10) ? '\0' : '.';
                            // remove spaces before extension
                            for (k = 7; k >= 1 && _filename[k] == ' '; k--);
                            if (k < 7) memcpy(&_filename[k+1], &_filename[8], 5);
                        }

                        if (attrPos & 0x10) {  // directory
                            sprintf(termbuffer, "%c%3u%c%-24.24s%c (dir)", 
                                COL_YELLOW, fctr, COL_WHITE, _filename, COL_CYAN);
                        } else {  // regular file
                            if(casrun == 1 &&  memcmp(_ext, "CAS", 3) == 0) {
                                // read from SD card once more and extract CAS data
                                open_command();
                                cmd17(calculate_sector_address(fc, 0));
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
                                const uint8_t blocks = ram_read_uint8_t(SDCACHE1 + 0x4F);

                                // print result for a CAS file with metadata information
                                sprintf(termbuffer, "%c%3u%c%.16s %.3s%c%2i  %6u", 
                                    COL_GREEN, fctr, COL_YELLOW, casname, ext, COL_CYAN, blocks, filesize);
                            } else { // non-cas file or not a cas run
                                sprintf(termbuffer, "%c%3u%c%-24.24s%c%6lu", 
                                    COL_GREEN, fctr, COL_WHITE, _filename, COL_YELLOW, filesize);
                            }
                        }
                        terminal_printtermbuffer();

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

                    lfn_found = 0; // reset LFN tracking
                }
                loc += 32;  // next file entry location
            }
            caddr++;    // next sector
        }
        ctr++;  // next cluster
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
    uint16_t loc = 0;               // current entry position
    uint8_t filename[11];
    uint8_t c = 0;                  // check byte
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && stopreading == 0) {
        
        // get cluster address
        uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr); // read sector data
            loc = SDCACHE0;
            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector
                // check first position
                c = ram_read_uint8_t(loc);

                // early exit if a zero is read
                if(c == 0x00) {
                    stopreading = 1;
                    break;
                }

                // continue if an unused entry is encountered 0xE5
                if(c == 0xE5) {
                    loc += 32;  // next file entry location
                    continue;
                }

                c = ram_read_uint8_t(loc + 0x0B);    // attrib byte

                // if lower five bits of byte 0x0B of file table is unset
                // assume we are reading a file and try to decode it
                if((c & 0x0F) == 0x00) {
                    fctr++;
                    copy_from_ram(loc, filename, 11);
                    if(memcmp(basename_find, filename, 8) == 0 && 
                       memcmp(ext_find, &filename[8], 3) == 0) {

                        _filesize_current_file = ram_read_uint32_t(loc + 0x1C);

                        return grab_cluster_address_from_fileblock(loc);
                    }
                }
                loc += 32;  // next file entry location
            }
            caddr++;
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
    memset(_linkedlist, 0xFF, F_LL_SIZE * sizeof(uint32_t));

    // try grabbing next cluster
    while(nextcluster < 0x0FFFFFF8 && nextcluster != 0 && ctr < F_LL_SIZE) {
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
uint32_t calculate_sector_address(uint32_t cluster, uint8_t sector) {
    return _SECTOR_begin_lba + (cluster - 2) * _sectors_per_cluster + sector;   
}

/**
 * @brief Construct sector address from file entry
 * 
 * @return uint32_t 
 */
uint32_t grab_cluster_address_from_fileblock(uint16_t loc) {
    return (uint32_t)ram_read_uint16_t(loc + 0x14) << 16 | 
                     ram_read_uint16_t(loc + 0x1A);
}

/**
 * @brief Store entry metadata in special global variables
 * 
 * @param entry_id entry id with respect to current sector data
 * @return uint32_t pointer to first cluster
 */
uint32_t store_file_metadata(uint8_t entry_id) {
    _filesize_current_file = ram_read_uint32_t(SDCACHE0 + entry_id * 32 + 28);
    _current_attrib = ram_read_uint8_t(SDCACHE0 + entry_id * 32 + 0x0B);
    return grab_cluster_address_from_fileblock(SDCACHE0 + entry_id * 32);
}

/**
 * @brief Store a file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 * @param verbose  whether to show progress
 */
void store_cas_ram(uint32_t faddr, uint16_t ram_addr) {
    build_linked_list(faddr);

    // count number of clusters
    uint8_t ctr = 0;
    uint8_t total_sectors = _filesize_current_file / 512 + 
                            (_filesize_current_file % 512 != 0 ? 1 : 0);
    uint32_t caddr = 0;
    uint16_t nbytes = 0;    // count number of bytes
    uint8_t sector_ctr = 0; // counter sector

    ctr = 0;
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && nbytes < _filesize_current_file) {

        // calculate address of sector
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors given a cluster and copy the data to RAM
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {

            if(sector_ctr == 0) {
                // program length and transfer address
                read_sector(caddr); // read sector data
                ram_write_uint16_t(0x8000, ram_read_uint16_t(SDCACHE0 + 0x0030));
                ram_write_uint16_t(0x8002, ram_read_uint16_t(SDCACHE0 + 0x0034));
                ram_transfer(0x100, ram_addr, 0x100);
                ram_addr += 0x100;
            } else {
                // open command for sending sector retrieval address
                open_command();
                cmd17(caddr);    // prime SD-card for data retrieval

                // perform fast data transfer using custom assembly routines
                switch(sector_ctr % 5) {
                    case 0:
                        // preamble is first 0x100 bytes of sector
                        fast_sd_to_ram_last_0x100(ram_addr);
                        ram_addr += 0x100;
                    break;
                    case 2:
                        // preamble is last 0x100 bytes of sector
                        fast_sd_to_ram_first_0x100(ram_addr);
                        ram_addr += 0x100;
                    break;
                    default: // 1,3,4 are complete blocks
                        fast_sd_to_ram_full(ram_addr);
                        ram_addr += 0x200;
                    break;
                }

                // close command
                close_command();
            }

            sprintf(termbuffer, "Loading %i / %i sectors", sector_ctr, total_sectors);
            terminal_redoline();

            nbytes += 0x200;
            if(nbytes >= _filesize_current_file) {
                break;
            }

            caddr++;
            sector_ctr++;
        }

        ctr++;
    }

    sprintf(termbuffer, "Done loading %i / %i sectors", 
                total_sectors, total_sectors);
    terminal_printtermbuffer();
}

/**
 * @brief Store a file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_prg_ram(uint32_t faddr, uint16_t ram_addr) {
    build_linked_list(faddr);

    // count number of clusters
    uint8_t ctr = 0;
    uint8_t cursec = 0;
    uint8_t total_sectors = _filesize_current_file / 512 + 
                            (_filesize_current_file % 512 != 0 ? 1 : 0);
    uint32_t caddr = 0;
    uint16_t nbytes = 0;    // count number of bytes

    ctr = 0;
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && nbytes < _filesize_current_file) {

        // calculate address of sector
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        //sprintf(termbuffer, "Copying program to %04X", ram_addr);
        //terminal_printtermbuffer();

        // loop over all sectors given a cluster and copy the data to RAM
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {

            // copy sector over to internal memory
            open_command();
            cmd17(caddr);
            fast_sd_to_ram_full(ram_addr);
            
            close_command();

            // increment ram pointer
            ram_addr += 0x200;

            sprintf(termbuffer, "Loading %i / %i sectors", cursec, total_sectors);
            terminal_redoline();

            nbytes += 0x200;
            if(nbytes >= _filesize_current_file) {
                break;
            }
            caddr++;
            cursec++;
        }

        ctr++;
    }

    sprintf(termbuffer, "Done loading %i / %i sectors", 
                total_sectors, total_sectors);
    terminal_printtermbuffer();
}