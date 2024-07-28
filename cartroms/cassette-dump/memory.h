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

#ifndef _MEMORY_H
#define _MEMORY_H

#include <z80.h>
#include "constants.h"

#define LOWMEM          0x6200 // starting point of lower memory
#define HIGHMEM_START   0xA000 // start address of upper memory
#define HIGHMEM_STOP    0xDFFF // end address of upper memory
#define BANKMEM_START   0xE000 // starting point of bankable memory
#define BANKMEM_STOP    0xFFFF // starting point of bankable memory
#define BANK_BYTES      0x2000 // number of bytes per bank
#define STACK           0x9F00 // lower position of the stack
#define NUMBANKS        6      // assuming 64kb memory expansion
#define MEMBANK         0x94   // Z80 I/O address for memory banking

#define PROGRAM_LOCATION 0xA000  // where to store custom programs

extern char* memory;
extern char* vidmem;
extern char* keymem;
extern char* highmem;
extern char* bankmem;

#endif // _MEMORY_H
