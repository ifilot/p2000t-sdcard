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

#ifndef _FAT32_H
#define _FAT32_H

#define F_LL_SIZE               16
#define MAX_LFN_LENGTH          26 // 2 * 13 (LFN entries come in 13 byte chunks)
#define PAGE_SIZE               18 // max number of files displayed on a page
#define DISPLAY_OFFSET           2 // line-offset in the video memory for displaying files

#include "sdcard.h"
#include "util.h"
#include "ram.h"

// global variables for the FAT
extern uint16_t _bytes_per_sector;
extern uint8_t _sectors_per_cluster;
extern uint16_t _reserved_sectors;
extern uint8_t _number_of_fats;
extern uint32_t _sectors_per_fat;
extern uint32_t _root_dir_first_cluster;
extern uint32_t _fat_begin_lba;
extern uint32_t _cluster_begin_lba;
extern uint32_t _lba_addr_root_dir;
extern uint32_t _linkedlist[F_LL_SIZE];
extern uint32_t _current_folder_cluster;

// global variables for currently active file or folder
extern uint32_t _filesize_current_file;
extern uint8_t _filename[]; // filename buffer
extern char _base_name[9]; // DOS 8.3 base name (8 chars, uppercased)
extern char _ext[4]; // DOS 8.3 extension (3 chars, uppercased)
extern uint8_t _current_attrib;
extern uint8_t _num_of_pages; // number of pages in the current folder

/**
 * @brief Read the Master Boot Record
 * 
 * @param verbose whether to return verbose output to terminal
 * @return uint32_t start sector-address of the first sector
 */
uint32_t read_mbr(void);

/**
 * @brief Read metadata of the partition
 * 
 * @param lba0 address of the partition
 */
void read_partition(uint32_t lba0);

/**
 * @brief Read the contents of the folder and display a page of files and folders.
 * 
 * @param page_number page number to display
 * @return uint32_t first cluster of the file or directory
 */
void read_folder(uint8_t page_number, uint8_t count_pages);

/**
 * @brief Find a file identified by sequence number in the current folder
 * 
 * @param file_id   file sequence number
 * @return uint32_t cluster address of the file or 0 if not found
 */
uint32_t find_file(uint16_t file_id);

/**
 * @brief Build a linked list of sector addresses starting from a root address
 * 
 * @param cluster0 first cluster in the linked list
 */
void build_linked_list(uint32_t nextcluster);

/**
 * @brief Calculate the sector address from cluster and sector
 * 
 * @param cluster which cluster
 * @param sector which sector on the cluster (0-Nclusters)
 * @return uint32_t sector address (512 byte address)
 */
uint32_t calculate_sector_address(uint32_t cluster, uint8_t sector);

/**
 * @brief Grab cluster address from file entry
 * 
 * @return uint32_t 
 */
uint32_t grab_cluster_address_from_fileblock(uint16_t loc);

/**
 * @brief Store a CAS file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_cas_ram(uint32_t faddr, uint16_t ram_addr);

/**
 * @brief Store a PRG file in internal ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_prg_intram(uint32_t faddr, uint16_t ram_addr);

#endif // _FAT32_H