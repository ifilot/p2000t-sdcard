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

char __lastinput[INPUTLENGTH];
uint8_t __bootcas = 0;

// set list of commands
char* __commands[] = {
    "ls",
    "lscas",
    "cd",
    "fileinfo",
    "run",
    "hexdump",
    "ledtest",
    "stack",
    "romdump",
    "ramdump",
    "dump",
    "help",
};

// set list of function pointers
void (*__operations[])(void) = {
    command_ls,
    command_lscas,
    command_cd,
    command_fileinfo,
    command_run,
    command_hexdump,
    command_ledtest,
    command_stack,
    command_romdump,
    command_ramdump,
    command_dump,
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
    if(check_mounted() == 1) { return; }

    read_folder(-1, 0);
}

/**
 * @brief List contents of a folder and parse CAS files
 * 
 */
void command_lscas(void) {
    if(check_mounted() == 1) { return; }

    read_folder(-1, 1);
}

/**
 * @brief change directory to folder indicated by id
 */
void command_cd(void) {
    static const char err[] = "Invalid entry or not a directory";
    if(check_mounted() == 1) { return; }

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

/**
 * @brief Obtain info of a given file
 * 
 */
void command_fileinfo(void) {
    int fileid = atoi(&__lastinput[8]);

    if(read_file_metadata(fileid) != 0) {
        return;
    }

    sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
    terminal_printtermbuffer();
    sprintf(termbuffer, "Filesize: %lu bytes", _filesize_current_file);
    terminal_printtermbuffer();
    print("Clusters:");
    for(uint8_t i=0; i<F_LL_SIZE; i++) {
        uint32_t cluster_id = _linkedlist[i];

        if(cluster_id == 0 || cluster_id == 0xFFFFFFFF) {
            break;
        }

        sprintf(termbuffer, "  %02i: %c%08lX", i+1, COL_CYAN, cluster_id);
        terminal_printtermbuffer();
    }
}

/**
 * @brief Load a (CAS) file into memory and launch it
 * 
 */
void command_run(void) {
    print_recall("Searching file...");

    int fileid = atoi(&__lastinput[3]);

    // find a file and store its cluster structure into the linked list
    if(read_file_metadata(fileid) != 0) {
        return;
    }

    if(memcmp(_ext, "CAS", 3) == 0) {
        sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Filesize: %lu bytes", _filesize_current_file);
        terminal_printtermbuffer();
        
        set_ram_bank(RAM_BANK_CASSETTE);
        store_cas_ram(_linkedlist[0], 0x0000);

        uint16_t deploy_addr = ram_read_uint16_t(0x8000);
        uint16_t file_length = ram_read_uint16_t(0x8002);

        sprintf(termbuffer, "Deploy addr: %c0x%04X", COL_CYAN, deploy_addr);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Program length: %c0x%04X", COL_CYAN, file_length);
        terminal_printtermbuffer();
        sprintf(termbuffer, "Top RAM: %c0x%04X", COL_CYAN, deploy_addr + file_length);
        terminal_printtermbuffer();

        print("Press c to calculate checksum or any");
        print("other key to launch program.");
        if(wait_for_key_fixed(28) == 1) {
            // calculate CRC16 checksum
            print_recall("Calculating CRC16 checksum...");
            uint16_t crc16 = crc16_ramchip(0x0000, file_length);
            sprintf(termbuffer, "CRC16 checksum: %c0x%04X", COL_CYAN, crc16);
            terminal_printtermbuffer();

            print("Press any key to start program");
            wait_for_key();
        }

        set_ram_bank(0);
        __bootcas = 1;
    } else if(memcmp(_ext, "PRG", 3) == 0) {
        if(memory[0x605C] < 2) {
            print_error("Insufficient memory.");
            print("At least 32kb of memory required.");
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
        print_error("Cannot only run CAS or PRG files.");
    }
}

void command_hexdump(void) {
    int file_id = atoi(&__lastinput[7]);

    if(read_file_metadata(file_id) != 0) {
        return;
    }

    // read the first sector of the file
    read_sector(calculate_sector_address(_linkedlist[0], 0));

    sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
    terminal_printtermbuffer();

    // print to screen
    terminal_hexdump(SDCACHE0, DUMP_EXTRAM);
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
 * @brief Indicate where the stack is
 * 
 */
void command_stack(void) {
    const uint16_t stackloc = get_stack_location();
    sprintf(termbuffer, "Stack location: %04X", stackloc);
    terminal_printtermbuffer();
}

/**
 * @brief Dump cartridge ROM contents to the screen
 * 
 */
void command_romdump(void) {
    uint8_t bank = 0;
    if(__lastinput[7] == '1') {
        bank = 1;
    }
    uint16_t addr = hexcode_to_uint16t(&__lastinput[8]);

    set_rom_bank(bank);
    terminal_hexdump(addr, DUMP_EXTROM);
    set_rom_bank(ROM_BANK_DEFAULT);
}

/**
 * @brief Dump cartridge ROM contents to the screen
 * 
 */
void command_ramdump(void) {
    uint8_t bank = 0;
    if(__lastinput[7] == '1') {
        bank = 1;
    }
    uint16_t addr = hexcode_to_uint16t(&__lastinput[8]);

    set_ram_bank(bank);
    terminal_hexdump(addr, DUMP_EXTRAM);
    set_ram_bank(RAM_BANK_CACHE);
}

/**
 * @brief Dump system RAM to the screen
 * 
 */
void command_dump(void) {
    uint16_t addr = hexcode_to_uint16t(&__lastinput[4]);
    terminal_hexdump(addr, DUMP_INTRAM);
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
    if(check_mounted() == 1) { return 1; }

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
        print_error("This is not a file");
        return 1;
    }

    build_linked_list(cluster);

    return 0;
}

/**
 * @brief Check whether the SD card is mounted
 * 
 * @return uint8_t 0 if mounted, 1 if not
 */
uint8_t check_mounted(void) {
    if(_flag_sdcard_mounted != 1) {
        print_error("Please mount the SD card first.");
        return 1;
    }

    return 0;
}