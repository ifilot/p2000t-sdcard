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
#include "util.h"
#include "config.h"
#include "sdcard.h"
#include "flash_utils.h"
#include "rom.h"
#include "crc16.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

#define LAUNCHERNAME "LAUNCHER"
#define LAUNCHEREXT "BIN"

// forward definitions
void init(void);

int main(void) {
    // initialize environment
    init();

    print("Press any key to search SD card for");
    print("flashable file.");

    wait_for_key();

    // mount sd card
    print_recall("Initializing SD card..");
    if(init_sdcard() != 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    }

    // inform user that the SD card is initialized and that we are ready to read
    // the first block from the SD card and print it to the screen
    print("SD Card initialized");

    print_recall("Mounting partition 1..");
    uint32_t lba0 = read_mbr();
    if(lba0 == 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    } else {
        read_partition(lba0);
        print("Partition 1 mounted");
    }

    // read the root directory
    uint32_t faddr = find_file(_root_dir_first_cluster, LAUNCHERNAME, LAUNCHEREXT);
    if(faddr != 0) {
        // reporting if file is found
        sprintf(termbuffer, "%s.%s found: %c%lu Bytes", LAUNCHERNAME, LAUNCHEREXT, COL_GREEN, _filesize_current_file);
        terminal_printtermbuffer();
        flash_rom(faddr);
    } else {
        sprintf(termbuffer, "No %s.%s found in root dir", LAUNCHERNAME, LAUNCHEREXT);
        terminal_printtermbuffer();
    }

    for(;;) {}
}

void init(void) {
    sdcs_set(); // disable SD card
    clear_screen();
    terminal_init(3, 20);
    vidmem[0x50] = TEXT_DOUBLE;
    vidmem[0x50+1] = COL_CYAN;
    sprintf(&vidmem[0x50+2], "SDCARD FLASHER");
    sprintf(&vidmem[0x50*22], "Version: %s", __VERSION__);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);
    print("System booted.");

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);

    // load program using first ram bank
    set_rom_bank(ROM_BANK_DEFAULT);
    set_ram_bank(RAM_BANK_CACHE);
}