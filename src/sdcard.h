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
extern uint8_t _resp8[5];
extern uint8_t _resp58[5];
extern uint8_t _flag_sdcard_mounted;

/**
 * @brief Initialize the SD card in such a way that sectors can be read
 *        from the card
 * 
 * Returns 0 on success and 1 on error
 */
uint8_t init_sdcard(void);

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
 * Send pulses to SD card to trigger a reset state
 */
void sdpulse(void) __z88dk_callee;

/**
 * CMD0: Reset the SD Memory Card
 */
uint8_t cmd0(void) __z88dk_callee;

/**
 * CMD8: Sends interface condition
 */
void cmd8(uint8_t *resp) __z88dk_fastcall;

/**
 * CMD17: Read block
 */
uint8_t cmd17(uint32_t addr) __z88dk_fastcall;

/**
 * CMD24: Write block
 */
uint8_t cmd24(uint32_t addr) __z88dk_fastcall;

/**
 * CMD55: Next command is application specific command
 */
void cmd55(void) __z88dk_callee;

/**
 * CMD58: Read OCR register
 */
void cmd58(uint8_t *resp) __z88dk_fastcall;

/**
 * ACMD41: Send host capacity support information
 */
uint8_t acmd41(void) __z88dk_callee;

/******************************************************************************
 * BLOCK OPERATIONS
 ******************************************************************************/

/**
 * @brief Copy all 0x200 bytes immediately from SD to internal RAM.
 * 
 * @param ram_addr external memory address
 */
void fast_sd_to_intram_full(uint16_t ram_addr) __z88dk_callee;

/**
 * @brief Read a single 512-byte sector
 * 
 * @param sec_addr sector address
 */
uint8_t read_sector(uint32_t sec_addr);

/**
 * @brief Read a single 512-byte sector
 * 
 * @param sec_addr sector address
 * @param ram_addr external RAM address to write the sector data to
 */
uint8_t read_sector_to(uint32_t sec_addr, uint16_t ram_addr) __z88dk_callee;

/******************************************************************************
 * I/O CONTROL
 ******************************************************************************/

/**
 * Set the SD CS signal to low (activating the SD card)
 */
void sdcs_reset(void) __z88dk_callee;

/**
 * Set the SD CS signal to high (deactivating the SD card)
 */
void sdcs_set(void) __z88dk_callee;

 /**
 * SDOUT is pulled low, pulling MISO low via a 10k resistor
 */
void sdout_set(void) __z88dk_callee;

/**
 * SDOUT is pulled high, pulling MISO high via a 10k resistor
 */
 void sdout_reset(void) __z88dk_callee;

#endif // _SDCARD_H