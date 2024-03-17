#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <z80.h>

#include "commands.h"
#include "constants.h"
#include "ascii.h"
#include "config.h"

// set printf io
#pragma printf "%i %X %lX %c %s %lu %u"

// definitions
void init(void);

void main(void) {
    // initialize environment
    init();

    // put in infinite loop and wait for user commands
    // only terminate the loop when a program should be executed
    while(__bootcas == 0) {
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
    clear_screen();
    terminal_init(3, 20);
    vidmem[0x50] = TEXT_DOUBLE;
    vidmem[0x50+1] = COL_CYAN;
    sprintf(&vidmem[0x50+2], "SDCARD READER");
    sprintf(&vidmem[0x50*22], "Version: %s", __VERSION__);
    sprintf(&vidmem[0x50*23], "Compiled at: %s / %s", __DATE__, __TIME__);

    // initialize command line
    memset(__input, 0x00, INPUTLENGTH+1);
    memset(__lastinput, 0x00, INPUTLENGTH);

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
    print_info("System ready.", 0);

    // insert cursor
    sprintf(termbuffer, "%c>%c", COL_CYAN, COL_WHITE);
    terminal_redoline();

    // load program using first ram bank
    set_ram_bank(0);
}