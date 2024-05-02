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

#include "terminal_ext.h"

/**
 * @brief Perform hexdump from part of external RAM
 * 
 * @param addr external RAM address
 */
void terminal_hexdump_ram(uint16_t addr) {
    sprintf(termbuffer, "%c%04X", COL_YELLOW, addr);
    for(uint8_t i=0; i<8; i++) {
        uint8_t val = ram_read_byte(addr+i);
        sprintf(&termbuffer[5+i*3], "%c%02X", COL_WHITE, val);
    }

    termbuffer[5+8*3] = COL_CYAN;

    for(uint8_t i=0; i<8; i++) {
        uint8_t val = ram_read_byte(addr+i);
        if(val >= 32 && val <= 127) {
            termbuffer[6+8*3+i] = val;
        } else {
            termbuffer[6+8*3+i] = '.';
        }
    }
    terminal_printtermbuffer();
}

/**
 * @brief Perform hexdump from part of internal RAM
 * 
 * @param buf buffer location
 */
void printblock(const uint8_t* buf) {
    // display the first 128 bytes of the 512 block on the screen
    for(uint8_t i=0; i<64; i++) {

        // show hexchars
        termbuffer[0] = COL_YELLOW;
        sprintf(&termbuffer[1], "0x%04X", i*8);

        termbuffer[7] = COL_WHITE;
        for(uint8_t j=0; j<8; j++) {
            sprintf(&termbuffer[j*3 + 8], "%02X", buf[i*8+j]);
        }

        // show ascii
        termbuffer[31] = COL_CYAN;
        for(uint8_t j=0; j<8; j++) {
            const uint8_t c = buf[i*8+j];
            if(c >= 32 && c <= 126) {
                termbuffer[32 + j] = c;
            } else {
                termbuffer[32 + j] = '.';
            }
        }

        terminal_printtermbuffer();
    }
}