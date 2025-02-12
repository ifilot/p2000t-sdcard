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

#ifndef _FAT32_H
#define _FAT32_H

#define F_LL_SIZE               16
#define MAX_LFN_LENGTH          26 // 2 * 13

#include "sdcard.h"
#include "util.h"
#include "ram.h"
#include "util.h"

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
extern char _ext[4];                        // file extension (uppercase)
extern uint8_t _filename[MAX_LFN_LENGTH+1];  // filename buffer
extern uint8_t _current_attrib;

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
 * @brief Read the contents of the root folder and search for a file identified 
 *        by file id. When a negative file_id is supplied, the directory is
 *        simply scanned and the list of files are outputted to the screen.
 * 
 * @param file_id ith file in the folder
 * @param casrun whether we are performing a run with CAS file metadata scan
 * @return uint32_t first cluster of the file
 */
uint32_t read_folder(int16_t file_id, uint8_t casrun);

/**
 * @brief Find a file identified by BASENAME and EXT in the folder correspond
 *        to the cluster address
 * 
 * @param cluster   cluster address
 * @param basename  first 8 bytes of the file
 * @param ext       3 byte extension of the file
 * @return uint32_t cluster address of the file or 0 if not found
 */
uint32_t find_file(uint32_t cluster, const char* basename, const char* ext);

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
 * @brief Store entry metadata in special global variables
 * 
 * @param entry_id entry id with respect to current sector data
 * @return uint32_t pointer to first cluster
 */
uint32_t store_file_metadata(uint8_t entry_id);

/**
 * @brief Store a CAS file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_cas_ram(uint32_t faddr, uint16_t ram_addr);

/**
 * @brief Store a PRG file in external ram
 * 
 * @param faddr    cluster address of the file
 * @param ram_addr first position in ram to store the file
 */
void store_prg_ram(uint32_t faddr, uint16_t ram_addr);

#endif // _FAT32_H