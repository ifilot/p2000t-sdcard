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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <z80.h>

#include "constants.h"
#include "config.h"
#include "ports.h"
#include "memory.h"
#include "terminal.h"
#include "ram.h"
#include "util.h"
#include "sdcard.h"

// set printf io
#pragma printf "%c %X %s"

// definitions
void init(void);

void main(void) {
    // initialize environment
    init();

    // mount sd card
    print_info("Initializing SD card..", 1);
    if(init_sdcard(_resp8, _resp58) != 0) {
        print_error("Error detecting SD card");
        for(;;){}
    } else {
        print_info("Initialization successfull", 0);
    }

    sprintf(termbuffer, "resp8: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
            _resp8[0], _resp8[1], _resp8[2], _resp8[3], _resp8[4]);
    terminal_printtermbuffer();

    sprintf(termbuffer, "resp58: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
            _resp58[0], _resp58[1], _resp58[2], _resp58[3], _resp58[4]);
    terminal_printtermbuffer();

    for(;;){}
}

void init(void) {
    // set the CACHE bank
    set_ram_bank(RAM_BANK_CACHE);

    clear_screen();
    terminal_init(3, 20);

    // set number of kb of memory
    // memory[0x605C] == 1 --> 16 KiB (0x6000 - 0x9FFF)
    //                   2 --> 32 KiB (0x6000 - 0xDFFF)
    //                   3 --> 40 KiB (0x6000 - 0xFFFF)
    const uint8_t nrkb = memory[0x605C] <= 2 ? memory[0x605C] * 16 : 40;   

    sprintf(&vidmem[0x50], "%c%cSDCARD TEST", TEXT_DOUBLE, COL_CYAN);
    sprintf(&vidmem[0x50*22], "Version: %s. Memory model: %i kb.", __VERSION__, nrkb);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);
}