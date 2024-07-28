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
__at (0x0000) char MEMORY[];
char* memory = MEMORY;

__at (0x5000) char VIDMEM[];
char* vidmem = VIDMEM;

__at (0x6000) char KEYMEM[];
char* keymem = KEYMEM;

__at (0xA000) char HIGHMEM[];
char* highmem = HIGHMEM;

__at (0xE000) char BANKMEM[];
char* bankmem = BANKMEM;