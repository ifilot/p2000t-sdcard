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

#include "commands.h"
#include "launch_cas.h"
#include "flash_utils.h"

char __lastinput[INPUTLENGTH];

// set list of commands
char* __commands[] = {
    "ls",
    "lscas",
    "cd",
    "run",
    "load",
    "ledtest",
    "flash",
    "help",
};

// set list of function pointers
void (*__operations[])(void) = {
    command_ls,
    command_lscas,
    command_cd,
    command_run,
    command_load,
    command_ledtest,
    command_flash,
    command_help,
};

// *****************************************************************************
// LIST OF COMMANDS
// *****************************************************************************

/**
 * @brief List contents of a folder
 * 
 */
void command_ls(void) {
    read_folder(-1, 0);
}

/**
 * @brief List contents of a folder and parse CAS files
 * 
 */
void command_lscas(void) {
    read_folder(-1, 1);
}

/**
 * @brief change directory to folder indicated by id
 */
void command_cd(void) {
    static const char err[] = "Invalid entry or not a directory";

    int id = atoi(&__lastinput[2]);
    _current_attrib = 0x00;

    uint32_t clus = read_folder(id, 0);
    if(clus != _root_dir_first_cluster) {
        if(_current_attrib & (1 << 4)) {
            if(clus == 0) { // if zero, this is the root directory
                _current_folder_cluster = _root_dir_first_cluster;
            } else {
                _current_folder_cluster = clus;
            }
        } else {
            print_error(err);
        }
    } else {
        print_error(err);
    }
}

void command_flash(void) {
    int fileid = atoi(&__lastinput[5]); // file nr

    // find a file and store its cluster structure into the linked list
    if(read_file_metadata(fileid) != 0) {
        return;
    }

    if ((memcmp(_base_name, "LAUNCHER", 8) == 0 || memcmp(_base_name, "EZLAUNCH", 8) == 0) && memcmp(_ext, "BIN", 3 ) == 0) {
        flash_rom(_linkedlist[0]);
    } else{
        print_error("Not a valid firmware file.");
    }
}

/**
 * @brief Load a (CAS) file into memory and launch it
 * 
 */
void command_loadrun(unsigned type) {
    print_recall("Searching file...");

    int fileid = atoi(&__lastinput[type ? 3 : 4]); // file nr after LOAD / RUN

    // find a file and store its cluster structure into the linked list
    if(read_file_metadata(fileid) != 0) {
        return;
    }

    if(memcmp(_ext, "CAS", 3) == 0) {
        sprintf(termbuffer, "Filename:%c%.22s", COL_CYAN, _filename);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Filesize: %lu bytes", _filesize_current_file);
        terminal_printtermbuffer();

        set_ram_bank(RAM_BANK_CASSETTE);
        store_cas_ram(_linkedlist[0], 0x0000);

        uint16_t deploy_addr = ram_read_uint16_t(0x8000);
        uint16_t file_length = ram_read_uint16_t(0x8002);

        if(memory[0x605C] == 1 && _filesize_current_file > MAX_BYTES_16K) {
            print_error("File too large to load");
            return;
        }

        sprintf(termbuffer, "Deploy addr: %c0x%04X", COL_CYAN, deploy_addr);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Program length: %c0x%04X", COL_CYAN, file_length);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Top RAM: %c0x%04X", COL_CYAN, deploy_addr + file_length);
        terminal_printtermbuffer();

        sprintf(termbuffer, "Press%cany key%cto %s the program.", TEXT_FLASH, TEXT_STEADY, 
            type ? "RUN" : "LOAD");
        terminal_printtermbuffer();
        wait_for_key();

        set_ram_bank(0);
        // now call asm function to copy the CAS program bytes from ext RAM to int RAM
        // and then start it by calling Run (0x28d4) or "warm" Reset (0x1FC6)
        // see "ROM routines BASIC.pdf" section 7.2
        launch_cas(type ? 0x28d4 : 0x1FC6);
    } else if(memcmp(_ext, "PRG", 3) == 0) {
        if(memory[0x605C] < 2) {
            print_error("At least 32kb of memory required.");
            return;
        }

        // verify that the filesize is not too big
        if(_filesize_current_file > 0x3D00) {
            print_error("File too large to load");
            return;
        }

        // copy program
        sprintf(termbuffer, "Deploying program at %c0xA000", COL_CYAN);
        terminal_printtermbuffer();
        store_prg_intram(_linkedlist[0], PROGRAM_LOCATION);

        // verify that the signature is correct
        if(memory[PROGRAM_LOCATION] != 0x50) {
            print_error("Invalid program ID");
            return;
        }

        // verify that the CRC-16 checksum matches
        if(crc16_intram(&memory[0xA010], read_uint16_t(&memory[0xA001])) != 
                        read_uint16_t(&memory[0xA003])) {
            print_error("CRC16 checksum failed");
            return;
        }

        // wait on user key push
        print("Press any key to run");
        wait_for_key();

        // transfer copy of current screen to external RAM
        copy_to_ram(vidmem, VIDMEM_CACHE, 0x1000);

        // launch the program
        //memset(&memory[0xA000], 0x00, 0x200);
        call_program(PROGRAM_LOCATION + 0x10);

        // retrieve copy of current screen
        copy_from_ram(VIDMEM_CACHE, vidmem, 0x1000);

        // clean up memory including stack program stack
        memset(&memory[0xA000], 0x00, 0xDF00 - 0xA000);
    } else {
        print_error("Can only run CAS or PRG files.");
    }
}

void command_load(void) {
    command_loadrun(0);
}

void command_run(void) {
    command_loadrun(1);
}

/**
 * @brief Test burning of read and write LEDs
 * 
 */
void command_ledtest(void) {
    z80_outp(PORT_LED_IO, 0x00);
    z80_delay_ms(500);
    z80_outp(PORT_LED_IO, 0x01);
    z80_delay_ms(500);
    z80_outp(PORT_LED_IO, 0x02);
    z80_delay_ms(500);
    z80_outp(PORT_LED_IO, 0x00);
}

/**
 * @brief Dump system RAM to the screen
 * 
 */
void command_help(void) {
    print("List of commands:");
    for(uint8_t i=0; i<(sizeof(__operations) / sizeof(void*)); i++) {
        sprintf(termbuffer, "  * %s", __commands[i]);
        terminal_printtermbuffer();
    }
    print("For more information, see:");
    print("https://github.com/ifilot/p2000t-sdcard");
    // print("or visit");
    // print("https://philips-p2000t.nl/");
}

// *****************************************************************************
// COMMAND PARSER
// *****************************************************************************

/**
 * @brief Execute the command given by instruction
 * 
 */
void execute_command(void) {
    // create copy of the input and flush input buffer
    memcpy(__lastinput, __input, INPUTLENGTH);
    memset(__input, 0x00, INPUTLENGTH+1);
    __inputpos = 0;
    strrstrip(__lastinput);

    sprintf(termbuffer, "%c>%c%s", COL_CYAN, COL_WHITE, __lastinput);
    terminal_printtermbuffer();

    // if only whitespaces are read, simply return
    if(strlen(__lastinput) == 0) {
        return;
    }

    // loop over all commmands until a match is found;
    // if so, execute the command
    for(uint8_t i=0; i<(sizeof(__operations) / sizeof(void*)); i++) {
        if(strcmp(__lastinput, __commands[i]) == 0) {
            __operations[i]();
            return;
        }
    }

    // try the same thing, but now only for the first n bytes
    for(uint8_t i=0; i<(sizeof(__operations) / sizeof(void*)); i++) {
        if(memcmp(__lastinput, __commands[i], strlen(__commands[i])) == 0) {
            __operations[i]();
            return;
        }
    }

    // if no valid command is found, print an error message
    print_error("Invalid command.");
}

// *****************************************************************************
// AUXILIARY ROUTINES
// *****************************************************************************

/**
 * @brief Read the metadata of a file identified by id
 * 
 * @param file_id iterator corresponding to ith file in folder
 * @return uint8_t whether file can be read, 0 true, 1 false
 */
uint8_t read_file_metadata(int16_t file_id) {
    // read file id and check its value
    if(file_id < 0) {
        print_error("Invalid file id");
        return 1;
    }

    uint32_t cluster = read_folder(file_id, 0);
    if(cluster == _root_dir_first_cluster) {
        print_error("Could not find file");
        return 1;
    }

    if(_current_attrib & (1 << 4)) {
        print_error("Not a file");
        return 1;
    }

    build_linked_list(cluster);

    return 0;
}