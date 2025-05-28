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

#include "commands.h"
#include "constants.h"
#include "ascii.h"
#include "config.h"
#include "ports.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

// definitions
void init(void);

void main(void) {
    // initialize environment
    init();

    // check if there is a file called "AUTOBOOT.CAS", if so, immediately
    // launch this CAS file
    uint32_t fcl = find_file(_root_dir_first_cluster, "AUTOBOOT", "CAS");
    if(fcl != 0) {
        print("Loading AUTOBOOT.CAS...");
        build_linked_list(fcl);
        set_ram_bank(RAM_BANK_CASSETTE);
        store_cas_ram(_linkedlist[0], 0x0000);
        set_ram_bank(0);
        return;
    }

    // put in infinite loop and wait for user commands
    for(;;) {
        if(keymem[0x0C] > 0) {
            for(uint8_t i=0; i<keymem[0x0C]; i++) {
                if(keymem[i] == 52) { // return key
                    memset(keymem, 0x00, 0x0D); // flush key buffer
                    execute_command();
                    break;
                }

                if(keymem[i] == 44) { // backspace key
                    if(__inputpos > 0) {
                        __inputpos--;
                        __input[__inputpos] = 0x00;
                    }
                    break;
                }

                if(__inputpos < INPUTLENGTH) {
                    __input[__inputpos] = __ascii[keymem[i]];
                    __inputpos++;
                }
            }

            // flush key buffer
            memset(keymem, 0x00, 0x0D);

            // place command on command line
            __input[__inputpos] = 0x00;
            sprintf(termbuffer, "%c>%c%s", COL_CYAN, COL_WHITE, __input);
            terminal_redoline();
        }

        // add a blinking cursor
        terminal_cursor_blink();
    }
}

void init(void) {
    // disable SD-card
    sdcs_set();

    // set the CACHE bank
    set_ram_bank(RAM_BANK_CACHE);

    clear_screen();
    terminal_init(3, 20);

    // set number of kb of memory
    // memory[0x605C] == 1 --> 16 KiB (0x6000 - 0x9FFF)
    //                   2 --> 32 KiB (0x6000 - 0xDFFF)
    //                   3 --> 40 KiB (0x6000 - 0xFFFF)
    const uint8_t nrkb = memory[0x605C] <= 2 ? memory[0x605C] * 16 : 40;

    sprintf(&vidmem[0x50], "%c%cSDCARD READER", TEXT_DOUBLE, COL_CYAN);
    sprintf(&vidmem[0x50*22], "Version: %s. Memory model: %i kb.", __VERSION__, nrkb);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    // initialize command line
    memset(__input, 0x00, INPUTLENGTH+1);
    memset(__lastinput, 0x00, INPUTLENGTH);

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);

    // mount sd card
    if(init_sdcard() != 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    }

    print_recall("Mounting partition 1..");
    uint32_t lba0 = read_mbr();
    if(lba0 == 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    } else {
        read_partition(lba0);
        print("Partition 1 mounted");
        print("System ready.");

        // insert cursor
        sprintf(termbuffer, "%c>%c", COL_CYAN, COL_WHITE);
        terminal_redoline();
    }
}