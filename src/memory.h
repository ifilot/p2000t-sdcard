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

#define BASIC_PRG_START 0x6547 // start of BASIC program memory
#define CUST_PRG_START  0x6549 // start of custom PRG program memory
#define MAX_BYTES_16K   14966  // maximum bytes free on a 16K P2000T
#define BASIC_RESET     0x1FC6 // call address to reset to BASIC prompt
#define BASIC_RUN       0x28D4 // call address to RUN a BASIC program

extern char* memory;
extern char* vidmem;
extern char* keymem;
extern char* highmem;
extern char* bankmem;

#endif // _MEMORY_H
