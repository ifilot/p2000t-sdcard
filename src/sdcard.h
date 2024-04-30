#ifndef _SDCARD_H
#define _SDCARD_H

#include <z80.h>
#include "terminal.h"
#include "memory.h"

/**
 * Perform low-level operations on the SD-card. Note that all functions
 * operate with two globally shared uint8 arrays:
 * 
 * - _resp: response object for the commands
 * - _sectorblock: for reading 512 byte blocks and their 2-byte checksums
 *
 * It is assumed that the contents of _sectorblock is only valid and
 * useful directly after setting it
 */

// shared buffer object to store the data of a single sector on the SD card
extern uint8_t _sectorblock[514];
extern uint8_t _resp[7];
extern uint8_t _flag_sdcard_mounted;

/**
 * @brief Initialize the SD card in such a way that sectors can be read
 *        from the card
 */
void init_sdcard(void);

/**
 * @brief Send a byte to the SD card
 * 
 * @param val 
 */
void send_byte(uint8_t val);

/******************************************************************************
 * RECEIVE OPERATIONS
 ******************************************************************************/

/**
 * Receive a response R1
 * 
 * Uses a response buffer object to write data to
 */
uint8_t receive_R1(void) __z88dk_callee;

/******************************************************************************
 * COMMAND OPERATIONS
 ******************************************************************************/

/**
 * Open the command interface
 */
void open_command(void) __z88dk_callee;

/**
 * Close the command interface
 */
void close_command(void) __z88dk_callee;

/**
 * CMD0: Reset the SD Memory Card
 */
void cmd0(void) __z88dk_callee;

/**
 * CMD8: Sends interface condition
 */
void cmd8(uint8_t *resp) __z88dk_callee;

/**
 * CMD17: Read block
 */
void cmd17(uint32_t addr);

/**
 * CMD55: Next command is application specific command
 */
void cmd55(void) __z88dk_callee;

/**
 * CMD58: Read OCR register
 */
void cmd58(uint8_t *resp) __z88dk_callee;

/**
 * ACMD41: Send host capacity support information
 */
uint8_t acmd41(void) __z88dk_callee;

/******************************************************************************
 * BLOCK OPERATIONS
 ******************************************************************************/

/**
 * @brief Read a 512 byte block including 2 bytes checksum from SD card
 * 
 * @param ramptr internal memory address to write to
 */
void read_block(uint16_t* ramptr) __z88dk_callee;

/**
 * @brief Copy the first 0x100 bytes immediately from SD to RAM while discarding
 *        all other data.
 * 
 * @param ram_addr external memory address
 */
void fast_sd_to_ram_first_0x100(uint16_t ram_addr) __z88dk_callee;

/**
 * @brief Copy the last 0x100 bytes immediately from SD to RAM while discarding
 *        all other data.
 * 
 * @param ram_addr external memory address
 */
void fast_sd_to_ram_last_0x100(uint16_t ram_addr) __z88dk_callee;

/**
 * @brief Copy all 0x200 bytes immediately from SD to external RAM.
 * 
 * @param ram_addr external memory address
 */
void fast_sd_to_ram_full(uint16_t ram_addr) __z88dk_callee;

/**
 * @brief Read a single 512-byte sector
 * 
 * @param addr cluster address
 * @param addr sector address
 */
void read_sector(uint32_t addr);

/******************************************************************************
 * I/O CONTROL
 ******************************************************************************/

/**
 * Set the SD CS signal to low (activating the SD card)
 */
void sdcs_reset(void);

/**
 * Set the SD CS signal to high (deactivating the SD card)
 */
void sdcs_set(void);

 /**
 * SDOUT is pulled low, pulling MISO low via a 10k resistor
 */
void sdout_set(void);

/**
 * SDOUT is pulled high, pulling MISO high via a 10k resistor
 */
 void sdout_reset(void);

#endif // _SDCARD_H