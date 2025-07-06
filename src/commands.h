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

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <string.h>
#include <stdlib.h>
#include <z80.h>

#include "constants.h"
#include "sst39sf.h"
#include "sdcard.h"
#include "memory.h"
#include "terminal.h"
#include "fat32.h"
#include "ram.h"
#include "ports.h"
#include "util.h"
#include "crc16.h"

#define __clock_freq 2500000

extern char __lastinput[INPUTLENGTH];

void commands_ls(void);

/**
 * @brief List contents of a folder and parse CAS files
 * 
 */
void command_lscas(void);

/**
 * @brief change directory to folder indicated by id
 */
void command_cd(void);

/**
 * @brief Flash a file to the ROM chip
 * 
 */
void command_flash(void);

/**
 * @brief Load a (CAS) file into memory and launch it
 * 
 */
void command_run(void);

/**
 * @brief Test burning of read and write LEDs
 * 
 */
void command_ledtest(void);

/**
 * @brief Show brief help message on screen
 * 
 */
void command_help(void);

/**
 * @brief Execute the command given by instruction
 * 
 */
void execute_command(void);

// *****************************************************************************
// AUXILIARY ROUTINES
// *****************************************************************************

/**
 * @brief Read the metadata of a file identified by id
 * 
 * @param file_id iterator corresponding to ith file in folder
 * @return uint8_t whether file can be read, 0 true, 1 false
 */
uint8_t read_file_metadata(int16_t file_id);

/**
 * @brief Convert hexcode to unsigned 16 bit integer
 * 
 * @param addr 
 * @return uint16_t 
 */
uint16_t hexcode_to_uint16t(uint8_t *addr) __z88dk_callee;

#endif // _COMMANDS_H