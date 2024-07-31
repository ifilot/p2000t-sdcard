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
#include "copy.h"
#include "config.h"
#include "sdcard.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

#define LAUNCHERNAME "LAUNCHER"
#define LAUNCHEREXT "BIN"

// forward definitions
void init(void);
uint8_t store_file_rom(uint32_t faddr, uint16_t rom_addr, uint8_t verbose);

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

        uint16_t rom_id = sst39sf_get_device_id();

        char *devicestring[11];
        memset(devicestring, 0x00, 11);
        switch(rom_id) {
            case 0xB5BF:
                memcpy(devicestring, "SST39SF010", 10);
            break;
            case 0xB6BF:
                memcpy(devicestring, "SST39SF020", 10);
            break;
            case 0xB7BF:
                memcpy(devicestring, "SST39SF040", 10);
            break;
        }

        if(rom_id == 0xB5BF || rom_id == 0xB6BF || rom_id == 0xB7BF) {
            // copying from RAM to ROM
            print("Connection to ROM chip established.");
            sprintf(termbuffer, "Device signature%c%04X%c: %s", COL_CYAN, rom_id, COL_WHITE, devicestring);
            terminal_printtermbuffer();
            print("Wiping 0x0000-0x3FFF.");
            for(uint8_t i=0; i<4; i++) {
                sst39sf_wipe_sector(0x1000 * i);
            }

            // copying from SD-CARD to RAM
            sprintf(termbuffer, "Copying %s.%s, please wait...", LAUNCHERNAME, LAUNCHEREXT);
            terminal_printtermbuffer();
            uint8_t sectors_stored = store_file_rom(faddr, 0x0000, 1);
            print(""); // empty line

            print("Calculating CRC16, please wait...");
            uint16_t checksum = crc16_romchip(0x0000, _filesize_current_file);

            if(checksum == 0x0000) {
                print("Checksum successfully validated.");

                print(""); // empty line
                sprintf(termbuffer, "%cFLASHING COMPLETED!", COL_GREEN);
                terminal_printtermbuffer();
            } else {
                print_error("Invalid checksum encountered.");
                print("Please try again.");
            }
        } else {
            sprintf(termbuffer, "Invalid device id: %04X", rom_id);
            terminal_printtermbuffer();
        }
    } else {
        sprintf(termbuffer, "No %s.%s found in root dir", LAUNCHERNAME, LAUNCHEREXT);
        terminal_printtermbuffer();
    }

    for(;;) {}
}

/**
 * @brief Store a file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param rom_addr first position in ram to store the file
 * @param verbose  whether to show progress
 * 
 * @return number of sectors stored
 */
uint8_t store_file_rom(uint32_t faddr, uint16_t rom_addr, uint8_t verbose) {
    build_linked_list(faddr);

    // count number of clusters
    uint8_t total_sectors = _filesize_current_file / 512;
    if(_filesize_current_file % 512 != 0) {
        total_sectors++;
    }

    uint8_t ctr = 0;    // counter for clusters
    uint8_t scctr = 0;  // counter for sectors
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16 && scctr < total_sectors) {

        const uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);

        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            // directly transfer data to ROM chip
            open_command();
            cmd17(caddr + i);
            fast_sd_to_rom_full(rom_addr);
            close_command();

            // increment memory pointer
            rom_addr += 0x200;

            if(verbose == 1) {
                sprintf(termbuffer, "Parsing %i / %i sectors", 
                    scctr, total_sectors);
                terminal_redoline();
            }

            scctr++;
            if(scctr == total_sectors) {
                break;
            }
        }

        ctr++;
    }

    if(verbose == 1) {
        sprintf(termbuffer, "Done parsing %i / %i sectors", 
                    total_sectors, total_sectors);
        terminal_printtermbuffer();
    }

    return scctr;
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