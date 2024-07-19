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

#ifndef _UTIL_H
#define _UTIL_H

#include "terminal.h"

/**
 * @brief Replace all bytes in a string
 * 
 * @param str original string
 * @param org byte to replace
 * @param rep replacement byte
 * @param nrbytes number of bytes to check
 */
void replace_bytes(uint8_t *str, uint8_t org, uint8_t rep, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Read a 16 bit value from memory
 * 
 * @param data memory location
 * @return uint16_t 16-bit value
 */
uint16_t read_uint16_t(const uint8_t* data) __z88dk_callee;

/**
 * @brief Read a 32 bit value from memory
 * 
 * @param data memory location
 * @return uint32_t 32-bit value
 */
uint32_t read_uint32_t(const uint8_t* data) __z88dk_callee;

/**
 * @brief Wait for key-press
 *
 */
void wait_for_key(void);

/**
 * @brief Wait but check for a specific key press
 *
 */
uint8_t wait_for_key_fixed(uint8_t quitkey);

/**
 * @brief Clear the screen
 * 
 */
void clear_screen(void);

/**
 * @brief Get the stack location object
 * 
 * @return uint16_t 
 */
uint16_t get_stack_location(void) __z88dk_callee;

/**
 * @brief Call program at location
 * 
 * @param location 
 */
void call_program(uint16_t ramptr) __z88dk_callee;

/**
 * @brief Convert hexcode to unsigned 16 bit integer
 * 
 * @param addr 
 * @return uint16_t 
 */
uint16_t hexcode_to_uint16t(uint8_t *addr) __z88dk_callee;

#endif //_UTIL_H