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

#ifndef _RAM_H
#define _RAM_H

#include <z80.h>
#include <stdint.h>

#include "memory.h"

#define SDCACHE0 0x0000
#define SDCACHE1 0x0200
#define SDCACHE2 0x0400
#define SDCACHE3 0x0600
#define SDCACHE4 0x0800
#define SDCACHE5 0x0A00
#define SDCACHE6 0x0C00
#define SDCACHE7 0x0E00

#define VIDMEM_CACHE 0x1000      // video memory address

/*
 * The internal memory on the SD-card cartridge has a capacity of 128kb divided
 * over 2x64kb banks. The lower bank is used for caching SD-card data while the
 * upper bank is used for loading in programs.
 */
#define RAM_BANK_CACHE          0
#define RAM_BANK_CASSETTE       1

//------------------------------------------------------------------------------
// SETTER FUNCTIONS
//------------------------------------------------------------------------------

/**
 * @brief Set the external RAM pointer
 * 
 * @param addr 
 */
void set_ram_address(uint16_t addr) __z88dk_fastcall;

/**
 * @brief Set the ram bank
 * 
 * @param bank id
 */
void set_ram_bank(uint8_t val) __z88dk_fastcall;

//------------------------------------------------------------------------------
// READ FUNCTIONS
//------------------------------------------------------------------------------

/**
 * @brief Retrieve single byte from external RAM
 * 
 * @param addr external memory address
 * @return uint8_t byte at address
 */
uint8_t ram_read_uint8_t(uint16_t addr) __z88dk_fastcall;

/**
 * @brief Retrieve 16 bit value from external RAM
 * 
 * @param addr  external memory address
 * @return uint16_t value to receive
 */
uint16_t ram_read_uint16_t(uint16_t addr) __z88dk_fastcall;

/**
 * @brief Retrieve 32 bit value from external RAM
 * 
 * @param addr  external memory address
 * @return uint32_t value to receive
 */
uint32_t ram_read_uint32_t(uint16_t addr) __z88dk_fastcall;

//------------------------------------------------------------------------------
// WRITE FUNCTIONS
//------------------------------------------------------------------------------

/**
 * @brief Write 8-bit value to external RAM
 * 
 * @param addr external memory address
 * @param val 8-bit value to write
 */
void ram_write_uint8_t(uint16_t addr, uint8_t val) __z88dk_callee;

/**
 * @brief Write 16-bit value to external RAM
 * 
 * @param addr  external memory address
 * @param val 16-bit value to write
 */
void ram_write_uint16_t(uint16_t addr, uint16_t val) __z88dk_callee;

//------------------------------------------------------------------------------
// COPY FUNCTIONS
//------------------------------------------------------------------------------

/**
 * @brief Copy data from internal memory to external RAM
 * 
 * See: ram.asm
 *
 * @param src      internal address
 * @param dest     external address
 * @param nrbytes  number of bytes to copy
 */
void copy_to_ram(uint8_t *src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Copy data from external memory to internal RAM
 * 
 * See: ram.asm
 *
 * @param src      address on external RAM
 * @param dest     internal address
 * @param nrbytes  number of bytes to copy
 */
void copy_from_ram(uint16_t src, uint8_t *dest, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Copy data from external RAM to external RAM
 * 
 * See: ram.asm
 *
 * @param src      source address on external RAM
 * @param dest     destination address on external RAM
 * @param nrbytes  number of bytes to copy
 */
void ram_transfer(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;

#endif // _RAM_H