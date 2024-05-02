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

#include "sdcard.h"

// shared buffer object to store the data of a single sector on the SD card
uint8_t _resp8[5];
uint8_t _resp58[5];
uint8_t _flag_sdcard_mounted = 0;

/******************************************************************************
 * BLOCK OPERATIONS
 ******************************************************************************/

/**
 * @brief Read a single 512-byte sector
 * 
 * @param addr sector address
 */
void read_sector(uint32_t addr) {
    open_command();
    cmd17(addr);
    read_block();
    close_command();
}