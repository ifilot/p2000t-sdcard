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

// filesystem variables
uint16_t _bytes_per_sector = 0;
uint8_t _sectors_per_cluster = 0;
uint16_t _reserved_sectors = 0;
uint8_t _number_of_fats = 0;
uint32_t _sectors_per_fat = 0;
uint32_t _root_dir_first_cluster = 0;
uint32_t _linkedlist[16];
uint32_t _fat_begin_lba = 0;
uint32_t _shadow_fat_begin_lba = 0;
uint32_t _sector_begin_lba = 0;
uint32_t _lba_addr_root_dir = 0;
uint32_t _filesize_current_file = 0;
uint32_t _current_folder_cluster = 0;
char _basename[9];
char _ext[4];
uint8_t _current_attrib = 0;
uint32_t _last_sector_cache = 0;
uint8_t _last_entry_id_cache = 0;

// file pointer variables
uint32_t _fptr_cluster = 0;             // first cluster of a file
uint32_t _fptr_filesize = 0;            // stored size of file
uint32_t _fptr_size_allocated = 0;      // storage space currently allocated
uint32_t _fptr_pos = 0;                 // read position
uint32_t _fptr_folder_addr = 0;         // cluster address of the folder
uint32_t _fptr_entry_sector_addr = 0;   // sector address of the file entry
uint8_t  _fptr_entry_id = 0;            // entry index

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
 * @param lba0 address of the partition (retrieved from read_mbr)
 */
void read_partition(uint32_t lba0) {
    // inform the reader that we are about to read partition 1
    print("Reading partition 1");

    // read the volume ID (first sector of the partition) and grab the data
    // describing this partition
    read_sector(lba0);

    // number of bytes per sector, typically 512
    _bytes_per_sector = ram_read_uint16_t(SDCACHE0 + 0x0B);

    // number of sectors per cluster, is determined upon SD card formatting
    _sectors_per_cluster = ram_read_uint8_t(SDCACHE0 + 0x0D);

    // number of reserved sectors, typically 2
    _reserved_sectors = ram_read_uint16_t(SDCACHE0 + 0x0E);

    // number of FATs, typically 2, the second FAT serves as a SHADOW fat
    // used for data retention
    _number_of_fats = ram_read_uint8_t(SDCACHE0 + 0x10);

    // number of sectors per FAT, this can be used to determine the start
    // position of the second FAT
    _sectors_per_fat = ram_read_uint32_t(SDCACHE0 + 0x24);

    // cluster address of the root directory
    _root_dir_first_cluster = ram_read_uint32_t(SDCACHE0 + 0x2C);

    // cluster address of the current folder
    _current_folder_cluster = _root_dir_first_cluster;

    // partition signature, should be 0x55AA
    uint16_t signature = ram_read_uint16_t(SDCACHE0 + 0x1FE);

    // calculate the total capacity on the partition
    // each FAT holds a number of sectors
    // each sector can refer to 128 clusters (128 x 32 = 512 bytes)
    // each cluster hosts a number of sectors
    // each sector has a specific sectors size (512 bytes for FAT32)
    sprintf(termbuffer, "Partition size:%c%lu MiB", COL_GREEN, (_sectors_per_fat * _sectors_per_cluster * _bytes_per_sector) >> 13 );
    terminal_printtermbuffer();

    // consolidate variables
    _fat_begin_lba = lba0 + _reserved_sectors;
    _shadow_fat_begin_lba = _fat_begin_lba + _sectors_per_fat + 1;
    _sector_begin_lba = _fat_begin_lba + (_number_of_fats * _sectors_per_fat);
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
    uint8_t c = 0;                  // check byte
    uint8_t filename[11];

    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE && stopreading == 0) {
        
        // print cluster number and address
        uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr);            // read sector data
            loc = SDCACHE0;
            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector
                // check first position
                c = ram_read_uint8_t(loc);

                // continue if an unused entry is encountered 0xE5
                if(c == 0xE5) {
                    loc += 32;  // next file entry location
                    continue;
                }

                // early exit if a zero is read
                if(c == 0x00) {
                    stopreading = 1;
                    break;
                }

                c = ram_read_uint8_t(loc + 0x0B);    // attrib byte

                // check if we are reading a file or a folder
                if((c & 0x0F) == 0x00) {

                    // capture metadata
                    fctr++;
                    const uint32_t fc = grab_cluster_address_from_fileblock(loc);
                    const uint32_t filesize = ram_read_uint32_t(loc + 0x1C);
                    totalfilesize += filesize;

                    if(file_id < 0) {
                        copy_from_ram(loc, filename, 11);
                        if(c & (1 << 4)) { // directory entry
                            sprintf(termbuffer, "%c%3u%c%.8s DIR       %c%08lX", COL_YELLOW, fctr, COL_WHITE, &filename[0x00], COL_CYAN, fc);
                            terminal_printtermbuffer();
                        } else {                // file entry
                            if(casrun == 1 && memcmp(&filename[0x08], "CAS", 3) == 0) {    // cas file
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
                                sprintf(termbuffer, "%c%3u%c%.16s %.3s%c%2i %6u", COL_GREEN, fctr, COL_YELLOW, casname, ext, COL_CYAN, blocks, filesize);
                                terminal_printtermbuffer();
                            } else { // non-cas file or not a cas run
                                // print result for a regular file
                                sprintf(termbuffer, "%c%3u%c%.8s.%.3s%c%6lu%c%08lX", COL_GREEN, fctr, COL_WHITE, &filename[0x00], &filename[0x08], COL_YELLOW, filesize, COL_CYAN, fc);
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
 * @brief Find a subfolder or a file inside current folder
 * 
 * @param search    11-byte search pattern
 * @param which     FIND_FOLDER or FIND_FILE
 * @return uint32_t cluster address
 */
uint32_t find_in_folder(const char* search, uint8_t which) {
    // build linked list for the root directory
    build_linked_list(_current_folder_cluster);

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over sectors
    uint8_t stopreading = 0;
    uint16_t loc = 0;               // current entry position
    uint8_t c = 0;                  // single byte entry for checking
    uint32_t caddr = 0;             // sector address
    uint8_t filename[11];

    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE && stopreading == 0) {
        
        // get sector address
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster && stopreading == 0; i++) {
            read_sector(caddr); // read sector data
            loc = SDCACHE0;

            for(uint16_t j=0; j<16; j++) { // 16 file tables per sector

                // grab first byte
                c = ram_read_uint8_t(loc);

                // continue if an unused entry is encountered 0xE5
                if(c == 0xE5) {
                    loc += 32;
                    continue;
                }

                // early exit if a zero is read
                if(c == 0x00) {
                    stopreading = 1;
                    break;
                }

                // grab filename to check
                copy_from_ram(loc, filename, 11);

                // grab attrib byte
                c = ram_read_uint8_t(loc + 0x0B);

                if((c & 0x0F) != 0x00) {
                    loc += 32;
                    continue;
                }

                switch(which) {
                    case F_FIND_FOLDER_NAME:
                    case F_FIND_FILE_NAME:
                        if(memcmp(search, filename, 11) == 0) {

                            // perform either file matching or folder matching
                            if(((c & (1 << 4)) && which == F_FIND_FOLDER_NAME) ||
                               (!(c & (1 << 4)) && which == F_FIND_FILE_NAME)) {
                                // if found, return cluster address
                                return grab_cluster_address_from_fileblock(loc);
                            }
                        }
                    break;
                    case F_FIND_FILE_ADDR:
                        c = ram_read_uint8_t(loc + 0x0B);
                        if(!(c & (1 << 4))) {
                            if(grab_cluster_address_from_fileblock(loc) == _fptr_cluster) {
                                store_file_metadata(j);
                                _last_sector_cache = caddr;
                                _last_entry_id_cache = j;
                                return _fptr_cluster;
                            }
                        }
                        
                    break;
                }
                
                loc += 32;  // increment to next directory entry
            }
            caddr++;        // increment to next sector
        }
        ctr++;
    }

    return 0;
}

/**
 * @brief Build a linked list of cluster addresses starting from a root address
 *        Holds up to a maximum of F_LL_SIZE entries
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
        nextcluster = ram_read_uint32_t((nextcluster & 0b01111111) * 4);
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
    return _sector_begin_lba + (cluster - 2) * _sectors_per_cluster + sector;
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
    _current_attrib = ram_read_uint8_t(SDCACHE0 + entry_id * 32 + 0x0B);
    return grab_cluster_address_from_fileblock(SDCACHE0 + entry_id * 32);
}

/**
 * @brief Create a new file in the current folder
 * 
 * @param filename 
 */
uint8_t create_new_file(const char* filename) {
    if(find_in_folder(filename, F_FIND_FILE_NAME) != 0) {
        return F_ERROR_FILE_EXISTS;
    }

    uint8_t res = create_file_entry(filename);
    switch(res) {
        case F_ERROR_DIR_FULL:
            // expand folder and try again
            return create_new_file(filename);
        break;
        case F_ERROR_CARD_FULL:
            return F_ERROR_CARD_FULL;
        break;
        case F_SUCCESS:
            return F_SUCCESS;
        break;
        default:
            return F_ERROR;
        break;
    }
}

/**
 * @brief Create a file entry in the folder
 * 
 * @param filename 11 byte file name
 * @return uint8_t whether file could be successfully created
 * 
 * When a directory does not have a free entry available to create a new file,
 * the directory needs to be expanded. In that situation, this function returns
 * an ERROR (0x01).
 */
uint8_t create_file_entry(const char* filename) {
    // build linked list for this subdirectory
    build_linked_list(_current_folder_cluster);

    uint8_t ctr = 0;                // counter over clusters
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint8_t res = 0;                // response test token
    uint32_t caddr = 0;
    uint16_t loc = 0;
    uint8_t c = 0;

    // loop over the clusters and read directory contents
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {
        
        // get cluster address
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            read_sector(caddr);                 // read sector data
            loc = SDCACHE0;
            for(uint16_t j=0; j<16; j++) {      // 16 file entries per sector

                // grab file metadata
                c = ram_read_uint8_t(loc);

                // check if the entry is available, if so, use it!
                if(c == 0x00 || c == 0xE5) {

                    // find the first available file cluster, this will garble SDCACHE
                    uint32_t filecluster = allocate_free_cluster();

                    if(filecluster != 0) { 
                        read_sector(caddr);
                        ram_set(loc, 0x00, 32);                             // wipe file entry data
                        copy_to_ram((uint8_t*)filename, loc, 11);           // write filename entry
                        ram_write_uint16_t(loc + 0x14, filecluster >> 16);  // write upper word
                        ram_write_uint16_t(loc + 0x1A, filecluster);        // write lower word
                        res = write_sector(caddr);                          // write data
                        return F_SUCCESS;
                    } else {
                        return F_ERROR_CARD_FULL;
                    }
                }

                loc += 32;
            }
            caddr++;
        }
        ctr++;
    }
    return F_ERROR_DIR_FULL;
}

/**
 * @brief Find the first available free sector from the FAT
 * 
 * @return uint32_t cluster address
 */
uint32_t allocate_free_cluster(void) {
    uint32_t addr = _fat_begin_lba;
    uint32_t ctr = 0;
    while(ctr < _sectors_per_fat) {
        read_sector(addr);
        uint16_t ramptr = 0;
        for(uint8_t i=0; i<128; i++) {
            uint32_t cluster = ram_read_uint32_t(ramptr);
            if(cluster == 0) {
                ram_write_uint32_t(ramptr, 0x0FFFFFFF);
                write_sector(addr);                         // write to primary FAT
                write_sector(addr + _sectors_per_fat);      // write to secondary FAT
                return (ctr << 7) | i;
            }
            ramptr += 4;
        }
        addr++;
        ctr++;
    }

    return 0;
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
 * @brief Set the file pointer by specifying folder address and file address
 * 
 * @param folder_addr 
 * @param file_addr 
 */
void set_file_pointer(uint32_t folder_addr, uint32_t file_addr) {
    // set cluster addresses of files
    _fptr_folder_addr = folder_addr;
    _fptr_cluster = file_addr;
    
    // grab total file size
    set_current_folder(folder_addr);
    find_in_folder("", F_FIND_FILE_ADDR);           // stores metadata
    _fptr_filesize = _filesize_current_file;        // store file size
    _fptr_entry_sector_addr = _last_sector_cache;   // store entry sector
    _fptr_entry_id = _last_entry_id_cache;          // store entry id

    // determine allocated file size
    build_linked_list(file_addr);

    uint8_t ctr = 0;
    _fptr_size_allocated = 0;
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {
        _fptr_size_allocated += _sectors_per_cluster * _bytes_per_sector;
        ctr++;
    }

    // reset pointer positions
    _fptr_pos = 0;
}

/**
 * @brief Write data to file pointer from external RAM
 * 
 * Assumes file pointer has been set
 * 
 * @param extramptr external RAM address
 * @param nrbytes   number of bytes to write
 */
void write_to_file(uint16_t extramptr, uint16_t nrbytes) {
    if(_fptr_cluster == 0) {
        print_error("No file pointer is set");
    }

    // calculate final position after writing
    uint32_t finalpos = _fptr_pos + nrbytes;

    // check if allocatable size needs to be expanded and do so if necessary
    if(finalpos > _fptr_size_allocated) {
        // print("Allocating more clusters");
        uint32_t bytes_per_cluster = _bytes_per_sector * _sectors_per_cluster;
        uint32_t required_size = finalpos - _fptr_size_allocated;
        uint8_t newclusters = required_size / bytes_per_cluster;
        if(newclusters == 0) {
            _fptr_size_allocated += bytes_per_cluster;
            newclusters++;
        }
        allocate_clusters(newclusters);
    }

    // build the linked list for the file
    build_linked_list(_fptr_cluster);

    // perform write operations
    uint32_t blockpos = 0;
    uint32_t nextblockpos = 0;
    uint16_t bytes_to_write = 0;

    // loop over the clusters
    uint8_t ctr = 0;    // cluster counter
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {

        // loop over the sectors in each cluster
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            // set position of next block
            nextblockpos = blockpos + _bytes_per_sector;

            // check if the write position is in this current block
            if(_fptr_pos >= blockpos && _fptr_pos < nextblockpos) {
                // determine how many bytes need to be written in this block
                if(finalpos < nextblockpos) {
                    bytes_to_write = finalpos - _fptr_pos;
                } else {
                    bytes_to_write = nextblockpos - _fptr_pos;
                }

                // determine sector address
                uint32_t sector_addr = calculate_sector_address(_linkedlist[ctr], i);

                // read data from sector in SDCACHE0
                read_sector(sector_addr);

                // copy new data into sector
                ram_transfer(extramptr, SDCACHE0 + (_fptr_pos - blockpos), bytes_to_write);

                // write sector from SDCACHE0
                write_sector(sector_addr);

                // increment pointers and write positions
                extramptr += bytes_to_write;
                _fptr_pos += bytes_to_write;
            }

            // check if all data has been written, if so, stop function
            if(finalpos == _fptr_pos) {

                // print("Data is written");

                // store updated file size if the file has grown in size
                if(finalpos > _fptr_filesize) {
                    read_sector(_fptr_entry_sector_addr);
                    ram_write_uint32_t(SDCACHE0 + _fptr_entry_id * 32 + 28, finalpos);
                    write_sector(_fptr_entry_sector_addr);
                }

                return;
            }

            // increment blockpos
            blockpos += _bytes_per_sector;
        }
        ctr++;
    }
}

/**
 * @brief Allocate for file pointer additional clusters
 */
void allocate_clusters(uint8_t nr_of_clusters) {
    // build the linked list for the file
    build_linked_list(_fptr_cluster);
    uint8_t ctr = 0;    // cluster counter
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {ctr++;}
    for(uint8_t i=0; i<nr_of_clusters; i++) {
        _linkedlist[ctr] = allocate_free_cluster();
        update_pointer_next_cluster(_linkedlist[ctr-1], _linkedlist[ctr]);
        ctr++;
    }
}

/**
 * @brief Update cluster pointer
 * 
 * @param src // source cluster
 * @param des // destination cluster
 */
void update_pointer_next_cluster(uint32_t src, uint32_t des) {
    uint32_t addr = _fat_begin_lba + (src >> 7);
    read_sector(addr);
    ram_write_uint32_t((src & 0b01111111) * 4, des);
    write_sector(addr);
    write_sector(addr + _sectors_per_fat);
}

/**
 * @brief Convert any invalid characters. FAT32 8.3 filenames only support
 *        uppercase characters and certain special characters. This function
 *        transforms any lowercase to uppercase characters and transforms any
 *        invalid special characters to 'X'.
 * 
 * @param filename filename to convert (only convert first 8 characters)
 */
void parse_fat32_filename(char* filename) {
    for(uint8_t j=0; j<8; j++) {
        if(filename[j] >= 'a' && filename[j] <= 'z') {
            filename[j] -= 0x20;
        }
        if(filename[j] >= 'A' && filename[j] <= 'Z') {
            continue;
        }
        if(filename[j] >= '0' && filename[j] <= '9') {
            continue;
        }
        if(filename[j] >= '#' && filename[j] <= ')') {
            continue;
        }
        if(filename[j] >= '^' && filename[j] <= '`') {
            continue;
        }
        if(filename[j] == '!' || filename[j] == '-' || filename[j] == '@') {
            continue;
        }
        if(filename[j] == '{' || filename[j] == '}' || filename[j] == '~' || filename[j] == ' ') {
            continue;
        }
        filename[j] = 'X';
    }
}

/**
 * @brief Rename a FAT32 filename by grabbing the last digit, checking
 *        if it is a digit. If so, increment it by one, if not, replace
 *        it by a zero.
 * 
 * @param filename filename to convert
 */
void rename_fat32_filename(char* filename) {
    if(!(filename[7] >= '0' && filename[7] <= '9')) {
        filename[7] = '0';
    } else {
        if(filename[7] == '9') {
            filename[7] = '0';
        } else {
            filename[7] += 1;
        }
    }
}