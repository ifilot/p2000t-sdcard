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
#include "ascii.h"
#include "config.h"
#include "ports.h"
#include "fat32.h"
#include "ram.h"
#include "sdcard.h"
#include "terminal_ext.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

// definitions
void init(void);
void show_sdcard_data(void);

void main(void) {
    // initialize environment
    init();

    // output SD card information to the user
    show_sdcard_data();

    // read contents root folder
    read_folder(-1, 0);

    // find dumps folder
    uint32_t folder_addr = find_in_folder("DUMPS      ", F_FIND_FOLDER_NAME);

    // check if folder is found
    if(folder_addr != 0) {
        // output folder contents
        set_current_folder(folder_addr);
        read_folder(-1, 0);

        // proceed to create new file
        uint8_t res = create_new_file("COPYTES2TXT");
        sprintf(termbuffer, "File creation result: %02X", res);
        terminal_printtermbuffer();

        set_current_folder(folder_addr);
        uint32_t file_addr = find_in_folder("TEST    TXT", F_FIND_FILE_NAME);
        read_sector(calculate_sector_address(file_addr, 0));
        ram_transfer(SDCACHE0, SDCACHE1, 512);

        file_addr = find_in_folder("COPYTES2TXT", F_FIND_FILE_NAME);
        // set_file_pointer(folder_addr, file_addr);

        // for(uint8_t i=0; i<130; i++) {
        //     write_to_file(SDCACHE1, 512);
        // }

        build_linked_list(file_addr);
        for(uint8_t i=0; i<F_LL_SIZE; i++) {
            sprintf(termbuffer, "%02X %08lX", i, _linkedlist[i]);
            terminal_printtermbuffer();
        }

        print("End of program");

    } else {
        print_error("No folder DUMPS found in root dir.");
    }

    for(;;){}
}

void init(void) {
    // disable SD-card
    sdcs_set();

    // set the CACHE bank
    set_ram_bank(RAM_BANK_CACHE);

    clear_screen();
    terminal_init(3, 20);

    const uint8_t nrkb = memory[0x605C] <= 2 ? memory[0x605C] * 16 : 40;

    sprintf(&vidmem[0x50], "%c%cSDCARD WRITETEST", TEXT_DOUBLE, COL_CYAN);
    sprintf(&vidmem[0x50*22], "Version: %s. Memory model: %i kb.", __VERSION__, nrkb);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);
}

/**
 * @brief Output information of the SD-CARD to the user
 * 
 */
void show_sdcard_data(void) {
    // mount sd card
    print("Initializing SD card..");
    
    // settings ~CS and PULL-UP resistor
    sdcs_set();
    sdout_set();

    // reset SD card
    sdpulse();

    // byte for capturing responses
    uint8_t c = 0x00;

    // CMD0: Reset the SD Memory Card
    open_command();
    c = cmd0();
    close_command();
    sprintf(termbuffer, "Response CMD0: %02X", c);
    terminal_printtermbuffer();

    // CMD8: Sends interface condition    
    open_command();
    cmd8(_resp8);
    close_command();

    // output resp8 to terminal
    // Typical responses:
    // Intenso cards: 01 00 00 00 01 AA
    sprintf(termbuffer, "CMD8: %02X %02X %02X %02X %02X", _resp8[0], _resp8[1], _resp8[2], _resp8[3], _resp8[4]);
    terminal_printtermbuffer();

    // keep on looping until zero result is found
    c = 0xFF;
    while(c != 0) {
        open_command();
        cmd55();
        close_command();
        open_command();
        c = acmd41();   // Send host capacity support information
        close_command();
    }

    // CMD53: Read OCR register
    open_command();
    cmd58(_resp58);
    close_command();

    // output resp58 to terminal
    sprintf(termbuffer, "CMD58: %02X %02X %02X %02X %02X", _resp58[0], _resp58[1], _resp58[2], _resp58[3], _resp58[4]);
    terminal_printtermbuffer();

    // inform user that the SD card is initialized and that we are ready to read
    // the first block from the SD card and print it to the screen
    print("SD Card initialized");

    uint32_t lba0 = read_mbr();
    read_partition(lba0);

    // sd card successfully mounted
    print("Partition 1 mounted");
}