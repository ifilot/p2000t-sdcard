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

#include "copy.h"
#include "terminal.h"

/**
 * @brief Copy 0x4000 bytes from external RAM to external ROM
 * 
 * @param ram_src starting position in RAM
 * @param nbytes  number of bytes to copy
 * @param verbose whether to show verbose output
 */
void copy_ram_rom(uint16_t ram_src, uint16_t nbytes, uint8_t verbose) {

    // wipe 16kb on rom
    for(uint8_t i=0; i<4; i++) {
        sst39sf_wipe_sector(0x1000 * i);
    }

    uint8_t nsectors = nbytes / 256;
    uint8_t remaining = nbytes % 256;
    uint16_t rom_addr = 0;

    for(uint8_t i=0; i<nsectors; i++) {
        uint8_t j = 0;

        do {
            sst39sf_write_byte(rom_addr++, ram_read_uint8_t(ram_src++));
        } while (j++ != 255);

        if(verbose == 1) {
            sprintf(termbuffer, "Copying sector %i / %i to ROM", 
                    i+1, nsectors+1);
            terminal_redoline();
        }
    }

    for(uint8_t j=0; j<remaining; j++) {
        sst39sf_write_byte(rom_addr++, ram_read_uint8_t(ram_src++));
    }

    if(verbose == 1) {
        sprintf(termbuffer, "Done copying sector %i / %i to ROM", 
                    nsectors+1, nsectors+1);
        terminal_printtermbuffer();
    }
}