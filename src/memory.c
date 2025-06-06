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

#include "memory.h"

// set video memory
__at (0x0000) uint8_t MEMORY[];
uint8_t* memory = MEMORY;

__at (0x5000) uint8_t VIDMEM[];
uint8_t* vidmem = VIDMEM;

__at (0x6000) uint8_t KEYMEM[];
uint8_t* keymem = KEYMEM;

__at (0xA000) uint8_t HIGHMEM[];
uint8_t* highmem = HIGHMEM;

__at (0xE000) uint8_t BANKMEM[];
uint8_t* bankmem = BANKMEM;