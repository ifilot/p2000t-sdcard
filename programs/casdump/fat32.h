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

#define LINKEDLIST_SIZE     16

#include "sdcard.h"
#include "util.h"
#include "ram.h"
#include "util.h"
#include "terminal_ext.h"

#define F_FIND_FOLDER_NAME      0x00
#define F_FIND_FILE_NAME        0x01
#define F_FIND_FILE_ADDR        0x02

#define F_SUCCESS               0x00
#define F_ERROR                 0x01
#define F_ERROR_DIR_FULL        0x02
#define F_ERROR_CARD_FULL       0x03
#define F_ERROR_FILE_EXISTS     0x04

#define F_LL_SIZE               16

// global variables for the FAT
extern uint16_t _bytes_per_sector;
extern uint8_t _sectors_per_cluster;
extern uint16_t _reserved_sectors;
extern uint8_t _number_of_fats;
extern uint32_t _sectors_per_fat;
extern uint32_t _root_dir_first_cluster;
extern uint32_t _fat_begin_lba;
extern uint32_t _shadow_fat_begin_lba;
extern uint32_t _sector_begin_lba;
extern uint32_t _cluster_begin_lba;
extern uint32_t _lba_addr_root_dir;
extern uint32_t _linkedlist[F_LL_SIZE];
extern uint32_t _current_folder_cluster;
extern uint32_t _fptr_cluster;

// global variables for currently active file or folder
extern uint32_t _filesize_current_file;
extern char _basename[9];
extern char _ext[4];
extern uint8_t _current_attrib;


/**
 * @brief Read the Master Boot Record
 * 
 * @return uint32_t sector-address of the first partition
 */
uint32_t read_mbr(void);

/**
 * @brief Read metadata of the partition
 * 
 * @param lba0 address of the partition (retrieved from read_mbr)
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
 * @brief Find a subfolder or a file inside current folder
 * 
 * @param search    11-byte search pattern
 * @param which     FIND_FOLDER or FIND_FILE
 * @return uint32_t cluster address
 */
uint32_t find_in_folder(const char* search, uint8_t which);

/**
 * @brief Build a linked list of cluster addresses starting from a root address
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
 * @brief Store entry metadata in special global variables
 * 
 * @param entry_id entry id with respect to current sector data
 * @return uint32_t pointer to first cluster
 */
uint32_t store_file_metadata(uint8_t entry_id);

/**
 * @brief Create a new file in the current folder
 * 
 * @param filename 11 byte file name
 */
uint8_t create_new_file(const char* filename);

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
uint8_t create_file_entry(const char* filename);

/**
 * @brief Find the first available free sector from the FAT
 * 
 * @return uint32_t cluster address
 */
uint32_t allocate_free_cluster(void);

/**
 * @brief Grab cluster address from file entry
 * 
 * @return uint32_t 
 */
uint32_t grab_cluster_address_from_fileblock(uint16_t loc);

/**
 * @brief Set the file pointer by specifying folder address and file address
 * 
 * @param folder_addr 
 * @param file_addr 
 */
void set_file_pointer(uint32_t folder_addr, uint32_t file_addr);

/**
 * @brief Set the current active folder
 * 
 * @param folder_addr 
 */
inline void set_current_folder(uint32_t folder_addr) {
    _current_folder_cluster = folder_addr;
}

/**
 * @brief Write data to file pointer from external RAM
 * 
 * @param extramptr external RAM address
 * @param nrbytes   number of bytes to write
 */
void write_to_file(uint16_t extramptr, uint16_t nrbytes);

/**
 * @brief Allocate for file pointer additional clusters
 */
void allocate_clusters(uint8_t nr_of_clusters);

/**
 * @brief Update cluster pointer
 * 
 * @param src // source cluster
 * @param des // destination cluster
 */
void update_pointer_next_cluster(uint32_t src, uint32_t des);

/**
 * @brief Convert any invalid characters. FAT32 8.3 filenames only support
 *        uppercase characters and certain special characters. This function
 *        transforms any lowercase to uppercase characters and transforms any
 *        invalid special characters to 'X'.
 * 
 * @param filename filename to convert (only convert first 8 characters)
 */
void parse_fat32_filename(char* filename);

/**
 * @brief Rename a FAT32 filename by grabbing the last digit, checking
 *        if it is a digit. If so, increment it by one, if not, replace
 *        it by a zero.
 * 
 * @param filename filename to convert
 */
void rename_fat32_filename(char* filename);

#endif // _FAT32_H