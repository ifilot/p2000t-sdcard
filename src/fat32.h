#ifndef _FAT32_H
#define _FAT32_H

#define LINKEDLIST_SIZE     16

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
extern uint32_t _linkedlist[LINKEDLIST_SIZE];
extern uint32_t _current_folder_cluster;

// global variables for currently active file or folder
extern uint32_t _filesize_current_file;
extern char _basename[9];
extern char _ext[4];
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
 * @param cluster cluster address of the folder
 * @param file_id ith file in the folder
 * @return uint32_t first cluster of the file
 */
uint32_t read_folder(uint32_t cluster, int16_t file_id);

void read_folder_cas(uint32_t cluster);

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
uint32_t get_sector_addr(uint32_t cluster, uint8_t sector);

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
 * @param verbose  whether to show progress
 */
void store_cas_ram(uint32_t faddr, uint16_t ram_addr, uint8_t verbose);

#endif // _FAT32_H