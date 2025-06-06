/**************************************************************************
 *                                                                        *
 *   Author(s): Ivo Filot <ivo@ivofilot.nl>                               *
 *              Dion Olsthoorn <@dionoid>                                 *
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

#include "fat32-easy.h"

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
uint8_t _num_of_pages = 1;

uint8_t _filename[MAX_LFN_LENGTH+1];
char _ext[4] = {0};
char _base_name[9] = {0};
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
    // read the volume ID (first sector of the partition)
    read_sector(lba0);

    // collect data
    _sectors_per_cluster = ram_read_uint8_t(SDCACHE0 + 0x0D);
    _reserved_sectors = ram_read_uint16_t(SDCACHE0 + 0x0E);
    _number_of_fats = ram_read_uint8_t(SDCACHE0 + 0x10);
    _sectors_per_fat = ram_read_uint32_t(SDCACHE0 + 0x24);
    _root_dir_first_cluster = ram_read_uint32_t(SDCACHE0 + 0x2C);
    _current_folder_cluster = _root_dir_first_cluster;

    // consolidate variables
    _fat_begin_lba = lba0 + _reserved_sectors;
    _SECTOR_begin_lba = lba0 + _reserved_sectors + (_number_of_fats * _sectors_per_fat);
}

uint32_t find_file(uint16_t file_id) {
    uint8_t ctr = 0;                // counter over clusters
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint16_t loc = 0;               // current entry position
    uint8_t firstPos = 0;
    uint8_t secondPos = 0;

    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {
        
        // print cluster number and address
        uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            read_sector(caddr);            // read sector data
            loc = SDCACHE0;
            for(uint8_t j=0; j<16; j++) { // 16 file tables per sector
                // check first position
                firstPos = ram_read_uint8_t(loc);
                _current_attrib = ram_read_uint8_t(loc + 0x0B);    // attrib byte

                // continue if an unused entry is encountered 0xE5
                if(firstPos == 0xE5) {
                    loc += 32;  // next file entry location
                    continue;
                }

                // early exit if a zero is read
                if(firstPos == 0x00) return _root_dir_first_cluster;

                // check for SFN entry
                if((_current_attrib & 0x0F) == 0x00) {

                    secondPos = ram_read_uint8_t(loc+1);

                    if((_current_attrib & 0x10) == 0 || firstPos != '.' || secondPos == '.') {

                        fctr++;

                        if (file_id == fctr) {
                            //_current_attrib = attrib; // store current attrib byte
                            copy_from_ram(loc, _base_name, 8);
                            copy_from_ram(loc+8, _ext, 3);
                            _filesize_current_file = ram_read_uint32_t(loc + 0x1C);
                            return grab_cluster_address_from_fileblock(loc);
                        }
                    }
                }
                loc += 32;  // next file entry location
            }
            caddr++;    // next sector
        }
        ctr++;  // next cluster
    }
    return _root_dir_first_cluster; //not found
}

/**
 * @brief Read the contents of the folder and display a page of files and folders.
 * 
 * @param page_number page number to display, 0 for calculating the number of pages
 * @return uint32_t first cluster of the file or directory
 */
void read_folder(uint8_t page_number, uint8_t count_pages) {
    if (count_pages) {
        _num_of_pages = 1; // reset page count
    }

    // loop over the clusters and read directory contents
    uint8_t ctr = 0;                // counter over clusters
    uint16_t fctr = 0;              // counter over directory entries (files and folders)
    uint16_t loc = 0;               // current entry position
    uint8_t firstPos = 0;
    uint8_t lfn_found = 0; 
    uint8_t display_next_file = 0; // whether to display next file
    uint16_t prev_ctr_start_fctr = 0;
    uint32_t prev_ctr = 0;
    uint16_t display_fctr = 0;
    uint32_t caddr = 0;             // current sector address

    if (!count_pages) {
        //look up cached jumptable for fast page access
        ctr = ram_read_uint8_t(SDCACHE2 + page_number-1);
        fctr = ram_read_uint16_t(SDCACHE3 + 2 * (page_number-1));
    }

    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < F_LL_SIZE) {
        
        uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors per cluster
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            read_sector(caddr);            // read sector data
            loc = SDCACHE0;
            for(uint8_t j=0; j<16; j++) { // 16 file tables per sector
                // check first position
                firstPos = ram_read_uint8_t(loc);
                _current_attrib = ram_read_uint8_t(loc + 0x0B);    // attrib byte

                // continue if an unused entry is encountered 0xE5
                if(firstPos == 0xE5) {
                    loc += 32;  // next file entry location
                    continue;
                }

                // early exit if a zero is read
                if(firstPos == 0x00) return;

                display_next_file = (display_fctr < PAGE_SIZE) && (page_number == fctr / PAGE_SIZE + 1); // current page number based on file count

                // check for LFN entry
                if (display_next_file) {
                    if ((_current_attrib & 0x0F) == 0x0F) {
                        if (!lfn_found) {
                            lfn_found = 1;  // indicate LNF found
                            memset(_filename, 0, MAX_LFN_LENGTH+1);
                        }
                        uint8_t seq = firstPos & 0x1F;  // LFN sequence number
                        uint8_t k = 0;
                        if (seq <= 3) {
                            // extract characters from LFN entry
                            for (k = 0; k < 5; k++) _filename[(seq - 1) * 13 + k] = ram_read_uint8_t(loc + 1 + k * 2);
                            for (k = 0; k < 6; k++) _filename[(seq - 1) * 13 + 5 + k] = ram_read_uint8_t(loc + 14 + k * 2);
                            for (k = 0; k < 2; k++) _filename[(seq - 1) * 13 + 11 + k] = ram_read_uint8_t(loc + 28 + k * 2);
                        }
                    }
                }

                // check for SFN entry
                if((_current_attrib & 0x0F) == 0x00) {

                    uint8_t secondPos = ram_read_uint8_t(loc+1);
                    if(firstPos != '.' || secondPos == '.') { // skip dotfiles but keep ".." parent folder

                        fctr++;
                        if (display_next_file) {
                            display_fctr++;

                            // if no LFN found, the SFN filename needs to be formatted
                            if (!lfn_found) {
                                copy_from_ram(loc, _filename, 8);
                                copy_from_ram(loc+8, _filename+9, 3);
                                _filename[12] = '\0'; // terminate the string
                                // if file, inject dot before extension
                                _filename[8] = (_current_attrib & 0x10) ? '\0' : '.';
                                // remove superfluous spaces before extension
                                uint8_t k = 0;
                                for (k = 7; k >= 1 && _filename[k] == ' '; k--);
                                if (k < 7) memcpy(&_filename[k+1], &_filename[8], 5); // 5 = "." + ext + '\0'
                            }

                            // char _debug[9] = {0};
                            // copy_from_ram(loc, _temp, 8);
                            // sprintf(vidmem + 0x50*(display_fctr+DISPLAY_OFFSET) + 4, "%s: ctr %d, caddr %d", _debug, ctr, caddr);

                            if(_current_attrib & 0x10) {
                                // directory entry
                                if (secondPos == '.') strcpy(_filename, "(terug)");
                                sprintf(vidmem + 0x50*(display_fctr+DISPLAY_OFFSET) + 3, "%c%-26.26s  (map)", COL_CYAN, _filename);
                            } else {
                                // file entry          
                                _filesize_current_file = ram_read_uint32_t(loc + 0x1C);
                                sprintf(vidmem + 0x50*(display_fctr+DISPLAY_OFFSET) + 3, "%c%-26.26s %6lu", COL_YELLOW, _filename, _filesize_current_file);
                            }
                        }

                        if (!count_pages && display_fctr == PAGE_SIZE)
                           return; // when full page is displayed, exit

                        // cache ctr and fctr for this page
                        if (count_pages) {
                            if (ctr != prev_ctr) {
                                prev_ctr_start_fctr = fctr - 1;
                                prev_ctr = ctr;
                            }
                            if ((fctr-1) % PAGE_SIZE == 0) {
                                if (fctr > 1) _num_of_pages++;
                                ram_write_uint8_t(SDCACHE2 + _num_of_pages-1, ctr);
                                ram_write_uint16_t(SDCACHE3 + 2 * (_num_of_pages-1), prev_ctr_start_fctr);
                            }
                        }
                    }
                    lfn_found = 0; // reset LFN tracking 
                }
                loc += 32;  // next file entry location
            }
            caddr++;    // next sector
        }
        ctr++;  // next cluster
    }
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
    uint32_t caddr = 0;
    uint16_t nbytes_read = 0;    // count number of bytes read
    uint8_t sector_ctr = 0; // counter sector
    uint8_t first_sector = 1; // whether this is the first sector

    ctr = 0;
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16) {

        // calculate address of sector
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors given a cluster and copy the data to RAM
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {

            read_sector_to(caddr + i, ram_addr); // read sector data (512 bytes) to external ram address
            switch(sector_ctr % 5) {
            case 0:
                // preamble is first 0x100 bytes of sector
                if (first_sector) {
                    // first sector, copy the preamble's transfer address and length
                    ram_write_uint16_t(0x8000, ram_read_uint16_t(SDCACHE0 + 0x0030));
                    ram_write_uint16_t(0x8002, ram_read_uint16_t(SDCACHE0 + 0x0032));
                    first_sector = 0;
                }
                ram_transfer(ram_addr + 0x100, ram_addr, 0x100);
                ram_addr += 256;
                break;
            case 2:
                ram_addr += 256;
                break;
            default: // 1,3,4 are complete blocks
                ram_addr += 512;
                break;
            }

            nbytes_read += 512;
            if(nbytes_read >= _filesize_current_file) {
                return;
            }
            sector_ctr++;
        }
        ctr++;
    }
}

/**
 * @brief Store a file in the internal ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_prg_intram(uint32_t faddr, uint16_t ram_addr) {
    build_linked_list(faddr);

    // count number of clusters
    uint8_t ctr = 0;
    uint8_t cursec = 0;
    uint32_t caddr = 0;
    uint16_t nbytes = 0;    // count number of bytes

    ctr = 0;
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && nbytes < _filesize_current_file) {

        // calculate address of sector
        caddr = calculate_sector_address(_linkedlist[ctr], 0);

        // loop over all sectors given a cluster and copy the data to RAM
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {

            // copy sector over to internal memory
            open_command();
            cmd17(caddr);
            fast_sd_to_intram_full(ram_addr);
            close_command();

            // increment ram pointer
            ram_addr += 0x200;

            nbytes += 512;
            if(nbytes >= _filesize_current_file) {
                break;
            }
            caddr++;
            cursec++;
        }
        ctr++;
    }
}