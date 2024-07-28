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
void terminal_hexdump(uint16_t addr, uint8_t nrlines, uint8_t mode) {

    for(uint16_t j=0; j<nrlines; j++) {  // loop over lines

        // show address
        sprintf(termbuffer, "%c%04X", COL_YELLOW, addr);

        // show bytes
        for(uint8_t i=0; i<8; i++) {    // loop over bytes
            uint8_t val = bytegrab(addr, mode);
            sprintf(&termbuffer[5+i*3], "%c%02X", COL_WHITE, val);

            // show ASCII value
            if(val >= 32 && val <= 127) {
                termbuffer[6+8*3+i] = val;
            } else {
                termbuffer[6+8*3+i] = '.';
            }

            addr++;
        }

        termbuffer[5+8*3] = COL_CYAN;
        terminal_printtermbuffer();
    }
}

uint8_t bytegrab(uint16_t addr, uint8_t mode) {
    switch(mode) {
        case DUMP_INTRAM:
            return memory[addr];
        break;
        case DUMP_EXTRAM:
            return ram_read_uint8_t(addr);
        break;
        default:
            return 0x00;
        break;
    }
}