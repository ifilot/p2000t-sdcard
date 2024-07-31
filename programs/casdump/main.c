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

#define MODE_AUTOMATIC 0
#define MODE_MANUAL    1

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

// definitions
void init(void);
void copy_current_tapeblock(void);

// pre-allocate a big read buffer
static uint8_t cassette_buffer[0x400];
static char filename[11];
uint8_t wait = 1;
uint8_t mode = MODE_AUTOMATIC;

int main(void) {
    // initialize environment
    init();

    // print buffer location
    sprintf(termbuffer, "Cassette buffer: 0x%04X", get_memory_location(cassette_buffer));
    terminal_printtermbuffer();

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
    char description[16];
    char ext[3];

    for(;;) {
        // whether to proceed to next cassette
        print("Select operation:");
        print("   (M)anual   (A)utomatic   (Q)uit");
        print_recall("   Enter command...");
        wait = 1;
        while(wait == 1) {
            wait_for_key();
            switch(keymem[0x00]) {
                case 34:    // A
                    mode = MODE_AUTOMATIC;
                    wait = 0;
                    print("Automatic mode set");
                break;
                case 30:    // M
                    mode = MODE_MANUAL;
                    wait = 0;
                    print("Manual mode set");
                break;
                case 3:     // Q
                    return 0;
                break;
                default:
                    print("Invalid option");
                    print_recall("Enter command...");
                    wait = 1;
                break;
            }
        }

        // rewind the tape
        print_recall("Rewinding tape...");
        tape_rewind();
        memory[CASSTAT] = 0;

        while(memory[CASSTAT] != 'M') {
            // read the first block from the tape; the data from the tape is now
            // copied to internal memory
            print_recall("Reading next program...");
            tape_read_block(cassette_buffer);
            // terminal_hexdump(get_memory_location(cassette_buffer), 4, DUMP_INTRAM);

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
            sprintf(termbuffer, "Found: %c%.16s %.3s%c%i%c%i", COL_YELLOW, description, 
                    ext,COL_CYAN,totalblocks,COL_MAGENTA,length);
            terminal_printtermbuffer();

            uint8_t store_continue = 1;
            uint8_t continue_copying = YES;
            if(mode == MODE_MANUAL) {
                print_recall("Copy program to sd-card? (Y/N) Abort (A)");
                wait = 1;
                while(wait == 1) {
                    wait_for_key();
                    switch(keymem[0x00]) {
                        case 33:    // Y
                            wait = 0;
                            continue_copying = YES;
                            store_continue = 1;
                        break;
                        case 25:    // N
                            wait = 0;
                            store_continue = 0;
                            print("Skipping program.");
                        break;
                        case 34:    // A
                            wait = 0;
                            continue_copying = NO;
                            print("Aborting copying.");
                        break;
                        default:
                            print("Invalid option");
                            print_recall("Copy program to sd-card? (Y/N) Abort (A)");
                            wait = 1;
                        break;
                    }
                }
            }

            if(continue_copying == NO) {
                break;
            }

            if(store_continue == 1) {
                // grab total blocks and start copying first block
                uint8_t blockcounter = 0;

                // create new file
                memcpy(filename, description, 8);
                memcpy(&filename[8], "CAS", 3);
                parse_fat32_filename(filename);
                uint8_t res = create_new_file(filename);
                switch(res) {
                    case F_ERROR_FILE_EXISTS:
                        while(res == F_ERROR_FILE_EXISTS) {
                            print("File exists, auto-renaming...");
                            rename_fat32_filename(filename);
                            sprintf(termbuffer, "Trying:%c%.8s.%.3s", COL_YELLOW, filename, &filename[8]);
                            terminal_printtermbuffer();
                            res = create_new_file(filename);
                        }
                        // fall through
                    case F_SUCCESS:
                        uint32_t file_addr = find_in_folder(filename, F_FIND_FILE_NAME);
                        if(file_addr != 0) {
                            set_file_pointer(folder_addr, file_addr);
                            copy_current_tapeblock();
                        } else {
                            print_error("Could not create file pointer");
                            print_error("Fatal exception, terminating.");
                            return -1;
                        }
                    break;
                    case F_ERROR_CARD_FULL:
                        print_error("SD-card is full.");
                        print("Please replace SD-card and restart");
                        print_error("Terminating.");
                        return -1;
                    break;
                    default:
                        print_error("Fatal error encountered, terminating.");
                        return -1;
                    break;
                }

                // consume all blocks
                while(memory[BLOCKCTR] > 1) {
                    blockcounter++;
                    sprintf(termbuffer, "Remaining blocks: %i...", memory[BLOCKCTR]-1);
                    terminal_redoline();
                    tape_read_block(cassette_buffer);
                    if(memory[CASSTAT] != 0) {
                        sprintf(termbuffer, "Stop reading tape, exit code: %c", memory[CASSTAT]);
                        terminal_printtermbuffer();
                        break;
                    }
                    copy_current_tapeblock();
                }
                sprintf(termbuffer, "%c%.16s%.3s%c>%c%.8s.%.3s", COL_GREEN, description, ext, COL_WHITE, COL_YELLOW, filename, &filename[8]);
                terminal_printtermbuffer();
            } else {
                // skip all blocks
                while(memory[BLOCKCTR] > 1) {
                    sprintf(termbuffer, "Skipping blocks: %i...", memory[BLOCKCTR]-1);
                    terminal_redoline();
                    tape_read_block(cassette_buffer);
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

    // mount sd card for SLOT1 cartridge, else assume that the
    // card is already mounted
    #ifdef SLOT1
    print_recall("Initializing SD card..");
    if(init_sdcard() != 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    }
    #endif

    // mount first partition
    uint32_t lba0 = read_mbr();
    if(lba0 == 0) {
        print_error("Invalid signature");
        sprintf(termbuffer, "Signature read: %04X", ram_read_uint16_t(SDCACHE0 + 510));
        terminal_printtermbuffer();
        #ifndef SLOT1
        print("Press any key to return to launcher");
        wait_for_key();
        #else
        for(;;){}
        #endif
    } else {
        read_partition(lba0);
        print("Partition 1 mounted");
    }
}

/**
 * @brief Copy the current tape block in memory to the SD-card
 * 
 */
void copy_current_tapeblock(void) {
    ram_set(SDCACHE1, 0x00, 0x100);                         // wipe first 0x100 bytes
    copy_to_ram(&memory[0x6030], SDCACHE1 + 0x30, 0x20);    // set metadata
    copy_to_ram(cassette_buffer, SDCACHE1 + 0x100, 0x400);  // set data
    write_to_file(SDCACHE1, 0x500);
}