#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <z80.h>

#include "constants.h"
#include "util.h"
#include "copy.h"
#include "config.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

#define LAUNCHERNAME "LAUNCHER"
#define LAUNCHEREXT "BIN"

// definitions
void init(void);

uint8_t store_file_rom(uint32_t faddr, uint16_t rom_addr, uint8_t verbose);

int main(void) {

    // initialize environment
    init();

    print_info("Press any key to search SD card for", 0);
    print_info("flashable file.", 0);

    wait_for_key();

    // mount sd card
    print_info("Initializing SD card..", 1);
    init_sdcard();

    // inform user that the SD card is initialized and that we are ready to read
    // the first block from the SD card and print it to the screen
    print_info("SD Card initialized", 0);

    print_info("Mounting partition 1..", 1);
    uint32_t lba0 = read_mbr();
    read_partition(lba0);

    // sd card successfully mounted
    print_info("Partition 1 mounted", 0);
    print_info("", 0);

    // read the root directory
    uint32_t faddr = find_file(_root_dir_first_cluster, LAUNCHERNAME, LAUNCHEREXT);
    if(faddr != 0) {
        set_ram_bank(1);
        set_rom_bank(0);

        // reporting if file is found
        sprintf(termbuffer, "%s.%s found: %c%lu Bytes", LAUNCHERNAME, LAUNCHEREXT, COL_GREEN, _filesize_current_file);
        terminal_printtermbuffer();

        uint16_t rom_id = sst39sf_get_device_id();
        if(rom_id == 0xB5BF) {
            // copying from RAM to ROM
            print_info("Connection to ROM chip established.", 0);
            print_info("Wiping 0x0000-0x3FFF.", 0);
            for(uint8_t i=0; i<4; i++) {
                sst39sf_wipe_sector(0x1000 * i);
            }

            // copying from SD-CARD to RAM
            sprintf(termbuffer, "Copying %s.%s, please wait...", LAUNCHERNAME, LAUNCHEREXT);
            terminal_printtermbuffer();
            uint8_t sectors_stored = store_file_rom(faddr, 0x0000, 1);
            print_info("",0); // empty line

            print_info("Calculating CRC16, please wait...", 0);
            uint16_t checksum = crc16_romchip(0x0000, _filesize_current_file);

            sprintf(termbuffer, "Checksum: %c0x%04X", COL_CYAN, checksum);
            terminal_printtermbuffer();

            // all done
            print_info("",0); // empty line
            sprintf(termbuffer, "%cFLASHING COMPLETED!", COL_GREEN);
            terminal_printtermbuffer();
        } else {
            sprintf(termbuffer, "Invalid device id: %04X", rom_id);
            terminal_printtermbuffer();
        }

        set_ram_bank(0);
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

        const uint32_t caddr = get_sector_addr(_linkedlist[ctr], 0);

        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            read_sector(caddr + i); // read sector data
            copy_to_rom((uint16_t)&_sectorblock[0], rom_addr, 0x200);
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
    clear_screen();
    terminal_init(3, 20);
    vidmem[0x50] = TEXT_DOUBLE;
    vidmem[0x50+1] = COL_CYAN;
    sprintf(&vidmem[0x50+2], "SDCARD FLASHER");
    sprintf(&vidmem[0x50*22], "Version: %s", __VERSION__);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    print_info("System booted.", 0);

    // load program using first ram bank
    set_rom_bank(0);
    set_ram_bank(0);
}