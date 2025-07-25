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

#ifndef _CRC16_H
#define _CRC16_H

#include <stdint.h>

/**
 * @brief Calculate CRC16 checksum for N bytes starting at internal ram address
 * 
 * @param addr internal RAM address
 * @param nrbytes number of bytes to parse
 * @return uint16_t 
 */
uint16_t crc16_intram(uint8_t *addr, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Calculate CRC16 checksum on ROM
 *
 * @param addr start address
 * @param nrbytes number of bytes to evaluate
 * @return uint16_t CRC-16 checksum
 */
uint16_t crc16_romchip(uint16_t addr, uint16_t nrbytes) __z88dk_callee;

#endif // _CRC16_H