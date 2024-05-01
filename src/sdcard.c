#include "sdcard.h"

// shared buffer object to store the data of a single sector on the SD card
uint8_t _resp8[5];
uint8_t _resp58[5];
uint8_t _flag_sdcard_mounted = 0;

/******************************************************************************
 * BLOCK OPERATIONS
 ******************************************************************************/

/**
 * @brief Read a single 512-byte sector
 * 
 * @param addr sector address
 */
void read_sector(uint32_t addr) {
    open_command();
    cmd17(addr);
    read_block();
    close_command();
}