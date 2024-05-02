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

#ifndef _COPY_H
#define _COPY_H

#include "ram.h"
#include "sst39sf.h"

/**
 * @brief Copy bytes from external RAM to external ROM
 * 
 * @param ram_src starting position in RAM
 * @param nbytes  number of bytes to copy
 * @param verbose whether to show verbose output
 */
void copy_ram_rom(uint16_t ram_src, uint16_t nbytes, uint8_t verbose);

/**
 * @brief Create a CRC-16 checksum of the first 0x4000 bytes
 *        of the internal ROM.
 * 
 * This routine is only used for the firmware flasher.
 */
void calculatecrc16(void);

#endif // _COPY_H