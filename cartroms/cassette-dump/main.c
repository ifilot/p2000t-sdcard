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
#include "tape.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

// definitions
void init(void);
void copy_current_tapeblock(void);
void parse_filename(char* filename);

void main(void) {
    // initialize environment
    init();

    // read contents root folder
    read_folder(-1, 0);

    // find dumps folder
    uint32_t folder_addr = find_in_folder("DUMPS      ", F_FIND_FOLDER_NAME);

    // check if folder is found
    if(folder_addr != 0) {
        print("Accessing DUMPS folder.");
        set_current_folder(folder_addr);
    } else {
        print_error("No folder DUMPS found in root dir.");
        for(;;){}
    }

    // create placeholders to store tape data
    char description[17];
    description[16] = '\0';
    char ext[4];
    ext[3] = '\0';

    for(;;) {
        // whether to proceed to next cassette
        print_recall("Start reading tape? (Y/N)");
        if(wait_for_key_fixed(33) == 0) {
            break;
        }

        // rewind the tape
        print_recall("Rewinding tape...");
        tape_rewind();
        memory[CASSTAT] = 0;

        while(memory[CASSTAT] != 'M') {
            // read the first block from the tape; the data from the tape is now
            // copied to internal memory
            print_recall("Reading next program...");
            tape_read_block();

            if(memory[CASSTAT] != 0) {
                sprintf(termbuffer, "%cStop reading tape, exit code: %c", COL_RED, memory[CASSTAT]);
                terminal_printtermbuffer();
                break;
            }

            // copy data from description to screen
            memcpy(description, &memory[DESC1], 8);
            memcpy(&description[8], &memory[DESC2], 8);
            memcpy(ext, &memory[EXT], 3);
            const uint8_t totalblocks = memory[BLOCKCTR];
            uint16_t length = (uint16_t)memory[LENGTH] | ((uint16_t)memory[LENGTH+1] << 8);

            // at this point, the data resides in internal memory and the user is
            // asked whether they want to store the program from tape on the ROM
            // chip or whether they want to continue searching for the next program
            // on the tape
            sprintf(termbuffer, "Found: %c%s %s%c%i%c%i", COL_YELLOW, description, 
                    ext,COL_CYAN,totalblocks,COL_MAGENTA,length);
            terminal_printtermbuffer();
            print_recall("Copy program to ROM? (Y/N)");

            // check if user presses YES key
            uint8_t store_continue = wait_for_key_fixed(33);

            if(store_continue == 1) {
                // grab total blocks and start copying first block
                uint8_t blockcounter = 0;

                // create new file
                char filename[12] = "________CAS";  // has terminating char '\0'
                memcpy(filename, description, 8);
                parse_filename(filename);
                if(create_new_file(filename) == F_SUCCESS) {
                    uint32_t file_addr = find_in_folder(filename, F_FIND_FILE_NAME);
                    if(file_addr != 0) {
                        set_file_pointer(folder_addr, file_addr);
                    } else {
                        print_error("Could not create file pointer");
                    }
                } else {
                    print_error("Unable to create file.");
                    for(;;){}
                }
                copy_current_tapeblock();

                // consume all blocks
                while(memory[BLOCKCTR] > 1) {
                    blockcounter++;
                    sprintf(termbuffer, "Remaining blocks: %i...", memory[BLOCKCTR]-1);
                    terminal_redoline();
                    tape_read_block();
                    if(memory[CASSTAT] != 0) {
                        sprintf(termbuffer, "Stop reading tape, exit code: %c", memory[CASSTAT]);
                        terminal_printtermbuffer();
                        
                        print_error("Reading the tape failed.");
                    }
                    copy_current_tapeblock();
                }
                sprintf(termbuffer, "%cCopied: %s to SD-CARD", COL_GREEN, description);
                terminal_printtermbuffer();
            } else {
                // skip all blocks
                while(memory[BLOCKCTR] > 1) {
                    sprintf(termbuffer, "Skipping blocks: %i...", memory[BLOCKCTR]-1);
                    terminal_redoline();
                    tape_read_block();
                }
                sprintf(termbuffer, "%cSkipping: %s", COL_RED, description);
                terminal_printtermbuffer();
            }
        }
        print("All done reading this tape.");
        print("Swap tape or swap side to continue");
        print("copying from tape to SD-card.");
        print("");
    }

    print("End of program.");
}

void init(void) {
    // disable SD-card
    sdcs_set();

    // set the CACHE bank
    set_ram_bank(RAM_BANK_CACHE);

    clear_screen();
    terminal_init(3, 20);

    const uint8_t nrkb = memory[0x605C] <= 2 ? memory[0x605C] * 16 : 40;

    sprintf(&vidmem[0x50], "%c%cCASSETTE DUMP", TEXT_DOUBLE, COL_CYAN);
    sprintf(&vidmem[0x50*22], "Version: %s. Memory model: %i kb.", __VERSION__, nrkb);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);

    // mount sd card
    print_recall("Initializing SD card..");
    if(init_sdcard() != 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    }

    // mount first partition
    uint32_t lba0 = read_mbr();
    read_partition(lba0);

    // sd card successfully mounted
    print("Partition 1 mounted");
}

/**
 * @brief Copy the current tape block in memory to the SD-card
 * 
 */
void copy_current_tapeblock(void) {
    ram_set(SDCACHE1, 0x00, 0x100);                         // wipe first 0x100 bytes
    copy_to_ram(&memory[0x6030], SDCACHE1 + 0x30, 0x20);    // set metadata
    copy_to_ram(&memory[BUFFER], SDCACHE1 + 0x100, 0x400);  // set data
    write_to_file(SDCACHE1, 0x500);
}

/**
 * @brief Convert any invalid characters. FAT32 8.3 filenames only support
 *        uppercase characters and certain special characters. This function
 *        transforms any lowercase to uppercase characters and transforms any
 *        invalid special characters to 'X'.
 * 
 * @param filename filename to convert (only convert first 8 characters)
 */
void parse_filename(char* filename) {
    for(uint8_t j=0; j<8; j++) {
        if(filename[j] >= 'a' && filename[j] <= 'z') {
            filename[j] -= 0x20;
        }
        if(filename[j] >= 'A' && filename[j] <= 'Z') {
            continue;
        }
        if(filename[j] >= '0' && filename[j] <= '9') {
            continue;
        }
        if(filename[j] >= '#' && filename[j] <= ')') {
            continue;
        }
        if(filename[j] >= '^' && filename[j] <= '`') {
            continue;
        }
        if(filename[j] == '!' || filename[j] == '-' || filename[j] == '@') {
            continue;
        }
        if(filename[j] == '{' || filename[j] == '}' || filename[j] == '~' || filename[j] == ' ') {
            continue;
        }
        filename[j] = 'X';
    }
}