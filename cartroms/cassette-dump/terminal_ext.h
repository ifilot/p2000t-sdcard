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

#ifndef _TERMINAL_EXT_H
#define _TERMINAL_EXT_H

#include <stdio.h>
#include <string.h>
#include "ram.h"
#include "terminal.h"

#define DUMP_INTRAM 0
#define DUMP_EXTRAM 1
#define DUMP_EXTROM 2

/**
 * @brief Perform hexdump from part of external RAM
 * 
 * @param addr external RAM address
 */
void terminal_hexdump(uint16_t addr, uint8_t nrlines, uint8_t mode);

uint8_t bytegrab(uint16_t addr, uint8_t mode);

#endif