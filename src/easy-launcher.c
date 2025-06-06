/**************************************************************************
 *                                                                        *
 *   Author(s): Dion Olsthoorn <@dionoid>                                 *
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <z80.h>

#include "constants.h"
#include "config.h"
#include "ports.h"
#include "memory.h"
#include "ram.h"
#include "fat32-easy.h"
#include "launch_cas.h"
#include "sst39sf.h"

#define EZ_LAUNCHER_VERSION "0.1"

// set printf io
#pragma printf "%d %c %s %lu"

// helper function prototypes
void show_status(const char* str);
void highlight_refresh(void);
void update_screen(uint8_t);
void clearscreen(void);
void update_pagination(void);
void store_file_rom(uint32_t faddr, uint16_t rom_addr);
uint8_t flash_rom(uint32_t faddr);
// key handling functions
void handle_key_H(void);
void handle_key_down(void);
void handle_key_up(void);
void handle_key_right(void);
void handle_key_select(uint8_t key0);

uint16_t highlight_id = 1; // file id of the currently highlighted/selected file
uint8_t page_num = 1;      // current page number

// chars containing the help bar
static const uint8_t help_bar[] = {
    0x13, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00,
    0x13, 0x00, 0x28, 0x3D, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x28, 0x6B, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x4E, 0x54, 0x45, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x35, 0x30, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x6E, 0x24, 0x00,
    0x13, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00,
    0x06, 0x50, 0x61, 0x67, 0x69, 0x6E, 0x61, 0x00, 0x53, 0x63, 0x72, 0x6F, 0x6C, 0x6C, 0x00, 0x00, 0x53, 0x74, 0x61, 0x72, 0x74, 0x00, 0x41, 0x70, 0x70, 0x00, 0x00, 0x53, 0x63, 0x72, 0x6F, 0x6C, 0x6C, 0x00, 0x50, 0x61, 0x67, 0x69, 0x6E, 0x61,
    0x06, 0x54, 0x65, 0x72, 0x75, 0x67, 0x00, 0x00, 0x4F, 0x6D, 0x68, 0x6F, 0x6F, 0x67, 0x00, 0x00, 0x4F, 0x70, 0x65, 0x6E, 0x00, 0x00, 0x4D, 0x61, 0x70, 0x00, 0x00, 0x4F, 0x6D, 0x6C, 0x61, 0x61, 0x67, 0x00, 0x00, 0x00, 0x48, 0x65, 0x65, 0x6E
};

void init(void) {
    // deactivate SD-card
    sdcs_set();

    // set the CACHE bank
    set_ram_bank(RAM_BANK_CACHE);

    // turn LEDs off
    z80_outp(PORT_LED_IO, 0x00);

    // activate and mount sd card
    uint32_t lba0;
    if(init_sdcard() != 0 || (lba0 = read_mbr()) == 0) {
        show_status("\001Kan geen FAT32 SD kaart vinden.");
        for(;;){}
    }
    read_partition(lba0);
}

void main(void) {
    // initialize SD card
    init();
    update_screen(1);

    // put in infinite loop and wait for program selection
    for(;;) {
        // wait for key-press
        if(keymem[0x0C] > 0) {
            uint8_t key0 = keymem[0];
            keymem[0x0C] = 0;
            if (key0 == 9) { // H key
                handle_key_H();
            }
            // key down
            if(key0 == 21)  {
                handle_key_down();
            }
            // key up
            if(key0 == 2)  {
                handle_key_up();
            }
            // key right
            if(key0 == 23)  {
                handle_key_right();
            }
            // space or enter key
            if(key0 == 17 || key0 == 52 || key0 == 32)  { // space or enter or CODE
                handle_key_select(key0);
            }
        }
    }
}

// *****************************************************************************
// HELPER ROUTINES
// *****************************************************************************

/**
 * @brief Refresh the highlight on the screen
 * 
 * This function updates the screen to highlight the currently selected file
 * and remove the highlight from all other files.
 */
void highlight_refresh(void) {
    uint8_t is_folder = 0;
    for (uint8_t i = 1; i <= PAGE_SIZE; i++) {
        is_folder = vidmem[0x50*(i+DISPLAY_OFFSET) + 36] == ')';
        memcpy(vidmem + 0x50*(i+DISPLAY_OFFSET) + 2, (i == highlight_id) ? "\x07\x3E" : (is_folder ? " \x06" : " \x03"), 2);
    }
}

/**
 * @brief Update the screen with the current folder contents
 * 
 * @param count_pages whether to count pages or not
 */
void update_screen(uint8_t count_pages) {
    // refresh the screen
    clearscreen();
    if (count_pages) build_linked_list(_current_folder_cluster);
    read_folder(page_num, count_pages);
    update_pagination();
    highlight_refresh();
}

/**
 * @brief Clear the screen and write the header
 * 
 * This function clears the video memory and writes the header, footer and empty body
 */
void clearscreen(void) {
    // clear screen
    memset(vidmem, 0x00, 0x780);
    strcpy(vidmem, "\006 P2000T SD-CARD\002v"EZ_LAUNCHER_VERSION);
    for (uint8_t i = 2; i < 22; i++) {
        strcpy(vidmem + 0x50*i, "\004\x1D");
    }
    strcpy(vidmem + 0x50*23 + 20, "\002 Toets H voor Hulp");
}

/**
 * @brief Update the pagination at the top-left of the screen
 * 
 * This function updates the pagination text at the top-left of the screen
 */
void update_pagination(void) {
    char pagina_str[32];
    sprintf(pagina_str, "\003Pagina %d van %d", page_num, _num_of_pages);
    strcpy(vidmem + 39 - strlen(pagina_str), pagina_str);
}

/**
 * @brief Show a status message at the bottom of the screen
 * 
 * This function clears the last line of the screen and writes the given string to it.
 * 
 * @param str The string to display
 */
void show_status(const char* str) {
    memset(vidmem + 0x50 * 23, 0x00, 0x50);
    strcpy(vidmem + 0x50 * 23, str);
}

/**
 * @brief Flash the external ROM with a new firmware
 * 
 * This function checks the device ID of the SST39SF ROM chip and if it is one of the known types,
 * it wipes it and copies the firmware from the SD card to the ROM.
 * 
 * @param faddr cluster address of the firmware file
 * 
 * @return 1 on success, 0 on failure
 */
uint8_t flash_rom(uint32_t faddr) {
    set_rom_bank(ROM_BANK_DEFAULT);
    set_ram_bank(RAM_BANK_CACHE);
    uint16_t rom_id = sst39sf_get_device_id();

    if(rom_id == 0xB5BF || rom_id == 0xB6BF || rom_id == 0xB7BF) {
        // Wiping 0x0000-0x3FFF
        for(uint8_t i=0; i<4; i++) {
            sst39sf_wipe_sector(0x1000 * i);
        }
        // copying from SD-CARD to ROM
        store_file_rom(faddr, 0x0000);
        return 1;
    } else {
        show_status("\001Onbekend SST39SF apparaatnummer.");
        return 0;
    }
}

/**
 * @brief Store a file in the external ROM
 * 
 * @param faddr    cluster address of the file
 * @param rom_addr first position in ROM to store the file
 * 
 * @return number of sectors stored
 */
void store_file_rom(uint32_t faddr, uint16_t rom_addr) {
    build_linked_list(faddr);
    // count number of sectors
    uint8_t total_sectors = (_filesize_current_file + 511) / 512;

    uint8_t ctr = 0;    // counter for clusters
    uint8_t scctr = 0;  // counter for sectors
    while(_linkedlist[ctr] != 0xFFFFFFFF && ctr < 16) {
        const uint32_t caddr = calculate_sector_address(_linkedlist[ctr], 0);
        for(uint8_t i=0; i<_sectors_per_cluster; i++) {
            // directly transfer data to ROM chip
            open_command();
            cmd17(caddr + i);
            fast_sd_to_rom_full(rom_addr);
            close_command();
            // increment memory pointer
            rom_addr += 0x200;
            if(++scctr == total_sectors) return;
        }
        ctr++;
    }
}

/**
 * @brief Handle the H key press
 * 
 * This function clears the screen and displays the help bar with tips.
 */
void handle_key_H(void) {
    clearscreen();
    memset(vidmem + 0x50 * 7, 0x00, 11 * 0x50);
    for (uint8_t i = 0; i < 5; i++) {
        memcpy(vidmem + 0x50 * (8 + i), help_bar + i * 40, 40);
    }
    strcpy(vidmem + 0x50*14, "\003Tips:");
    strcpy(vidmem + 0x50*15, "\003*\007Spatiebalk werkt ook i.p.v. Enter");
    strcpy(vidmem + 0x50*16, "\003*\007CODE toets: LOAD en terug naar Basic");

    while(keymem[0x0C] == 0) {} // wait until a key is pressed
    keymem[0x0C] = 0;

    update_screen(0);
}

/**
 * @brief Handle the key down press
 * 
 * This function moves the highlight down to the next file or page.
 */
void handle_key_down(void) {
    if (vidmem[0x50*(highlight_id+DISPLAY_OFFSET+1) + 4] != 0x00) {
        highlight_id++;
    }
    else {
        page_num++;
        if (page_num > _num_of_pages) {
            page_num = 1; // wrap around
        }
        highlight_id = 1; // highlight first item in newly loaded folder
        if (_num_of_pages > 1) update_screen(0);
    }
    highlight_refresh();
}

/**
 * @brief Handle the key up press
 * 
 * This function moves the highlight up to the previous file or page.
 */
void handle_key_up(void) {
    if (highlight_id > 1) {
        highlight_id--;
    }
    else {
        page_num--;
        if (page_num == 0) {
            page_num = _num_of_pages; // wrap around
        }
        if (_num_of_pages > 1) {
            clearscreen();
            read_folder(page_num, 0);
            update_pagination();
        }
        for (int8_t i = PAGE_SIZE; i >= 0; i--) {
            if (vidmem[0x50*(i+DISPLAY_OFFSET) + 4] != 0x00) {
                highlight_id = i;
                break;
            }
        }
    }
    highlight_refresh();
}

/**
 * @brief Handle the key right press
 * 
 * This function moves to the next page of files.
 */
void handle_key_right(void) {
    if (_num_of_pages == 1) return;
    if (page_num < _num_of_pages) {
        page_num++;
    } else {
        page_num = 1; // wrap around
    }
    highlight_id = 1; // highlight first item in newly loaded folder
    update_screen(0);
}

void color_selected_file_red(void) {
    // color the file red
    vidmem[0x50*(highlight_id + DISPLAY_OFFSET) + 2] = 0x01; // color file red
    highlight_refresh();
}

/**
 * @brief Handle the key select press (space or enter)
 * 
 * This function handles the selection of a file or folder.
 * If a folder is selected, it navigates into that folder.
 * If a file is selected, it loads and runs the file if supported.
 * 
 * @param key0 The key pressed (space or enter)
 */
void handle_key_select(uint8_t key0) {
    uint32_t cluster = find_file(highlight_id + PAGE_SIZE * (page_num-1));
    if(cluster != _root_dir_first_cluster) {
        if(_current_attrib & 0x10) {
            if(cluster == 0) { // if zero, this is the root directory
                _current_folder_cluster = _root_dir_first_cluster;
            } else {
                _current_folder_cluster = cluster;
            }
            page_num = 1;
            highlight_id = 1; // highlight first item in newly loaded folder
            update_screen(1);
        }
        else {
            if ((memcmp(_base_name, "LAUNCHER", 8) == 0 || memcmp(_base_name, "EZLAUNCH", 8) == 0) && memcmp(_ext, "BIN", 3 ) == 0) {
                show_status("\003Firmware vernieuwen...");
                if (flash_rom(cluster))
                    call_addr(0x1010); //cold reset after firmware flashing
                return;
            }

            if (memcmp(_ext, "CAS", 3) != 0 && memcmp(_ext, "PRG", 3) != 0) {
                // unsupported file type
                color_selected_file_red();
                return;
            }

            if (memcmp(_ext, "PRG", 3) == 0 && memory[0x605C] < 2) {
                // no extension RAM found to load PRG into
                color_selected_file_red();
                return;
            }
            
            build_linked_list(cluster); // update _linkedlist for store_cas_ram and store_prg_intram

            if (memcmp(_ext, "CAS", 3) == 0) {
                // set RAM bank to CASSETTE
                set_ram_bank(RAM_BANK_CASSETTE);
                show_status("\003Programma laden...");
                store_cas_ram(_linkedlist[0], 0x0000);
                set_ram_bank(RAM_BANK_CACHE);
                // if CODE was pressed, load and return to Basic, otherwise load and run
                launch_cas(key0 == 32 ? 0x1FC6 : 0x28d4);
            }

            // load PRG file into internal RAM
            store_prg_intram(_linkedlist[0], PROGRAM_LOCATION);

            // verify that the signature is correct
            if(memory[PROGRAM_LOCATION] != 0x50) {
                color_selected_file_red();
                goto restore_state;
            }

            copy_to_ram(vidmem, VIDMEM_CACHE, 0x1000); // save the current video memory state
            call_addr(PROGRAM_LOCATION + 0x10); // launch the PRG program
            copy_from_ram(VIDMEM_CACHE, vidmem, 0x1000); //r estore the video memory state
            keymem[0x0C] = 0; // clear the key buffer
            
restore_state:
            build_linked_list(_current_folder_cluster); // rebuild the linked list for the current folder
        }
    }
}