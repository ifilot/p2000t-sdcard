
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <z80.h>

#include "constants.h"
#include "config.h"
#include "sdcard.h"
#include "terminal.h"
#include "sst39sf.h"
#include "flash_utils.h"

uint8_t flash_rom(uint32_t faddr) {
    uint16_t rom_id = sst39sf_get_device_id();

#ifdef FLASH_VERBOSE 
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
#endif

    if(rom_id == 0xB5BF || rom_id == 0xB6BF || rom_id == 0xB7BF) {
        // copying from RAM to ROM
#ifdef FLASH_VERBOSE 
        print("Connection to ROM chip established.");
        sprintf(termbuffer, "Device signature%c%04X%c: %s", COL_CYAN, rom_id, COL_WHITE, devicestring);
        terminal_printtermbuffer();
        print("Wiping 0x0000-0x3FFF.");
#endif
        for(uint8_t i=0; i<4; i++) {
            sst39sf_wipe_sector(0x1000 * i);
        }

        // copying from SD-CARD to RAM
        print("Flashing ROM, please wait...");
        uint8_t sectors_stored = store_file_rom(faddr, 0x0000);
#ifdef FLASH_VERBOSE 
        print(""); // empty line
#endif
        print("Calculating CRC16, please wait...");
        uint16_t checksum = crc16_romchip(0x0000, _filesize_current_file);

        if(checksum == 0x0000) {
            print("Checksum successfully validated.");
#ifdef FLASH_VERBOSE 
            print(""); // empty line
#endif
            sprintf(termbuffer, "%cFLASHING COMPLETED!", COL_GREEN);
            terminal_printtermbuffer();
        } else {
            print_error("Invalid checksum encountered.");
        }
    } else {
        sprintf(termbuffer, "Invalid device id: %04X", rom_id);
        terminal_printtermbuffer();
        return 0;
    }
    return 1;
}


/**
 * @brief Store a file in the external ram
 * 
 * @param faddr    cluster address of the file
 * @param rom_addr first position in ram to store the file
 * 
 * @return number of sectors stored
 */
uint8_t store_file_rom(uint32_t faddr, uint16_t rom_addr) {
    build_linked_list(faddr);

    // count number of clusters
    uint8_t total_sectors = (_filesize_current_file + 511) / 512;

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

#ifdef FLASH_VERBOSE
            sprintf(termbuffer, "Parsing %i / %i sectors", 
                scctr, total_sectors);
            terminal_redoline();
#endif

            scctr++;
            if(scctr == total_sectors) {
                break;
            }
        }

        ctr++;
    }

#ifdef FLASH_VERBOSE
    sprintf(termbuffer, "Done parsing %i / %i sectors", 
                total_sectors, total_sectors);
    terminal_printtermbuffer();
#endif

    return scctr;
}