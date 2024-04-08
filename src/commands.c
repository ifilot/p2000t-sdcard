#include "commands.h"

char __lastinput[INPUTLENGTH];
uint8_t __bootcas = 0;

// set list of commands
char* __commands[] = {
    "ls",
    "cd",
    "fileinfo",
    "run",
    "hexdump",
    "ledtest",
};

// set list of function pointers
void (*__operations[])(void) = {
    command_ls,
    command_cd,
    command_fileinfo,
    command_run,
    command_hexdump,
    command_ledtest,
};

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

void command_ls(void) {
    if(check_mounted() == 1) { return; }

    read_folder(_current_folder_cluster, -1);
}

/**
 * @brief change directory to folder indicated by id
 */
void command_cd(void) {
    if(check_mounted() == 1) { return; }

    int id = atoi(&__lastinput[2]);

    uint32_t clus = read_folder(_current_folder_cluster, id);
    if(clus != _root_dir_first_cluster) {
        if(_current_attrib & (1 << 4)) {
            if(clus == 0) { // if zero, this is the root directory
                _current_folder_cluster = _root_dir_first_cluster;
            } else {
                _current_folder_cluster = clus;
            }
        }
    } else {
        print_error("Invalid entry or not a directory");
    }
}

void command_fileinfo(void) {
    int fileid = atoi(&__lastinput[8]);

    if(read_file_metadata(fileid) != 0) {
        return;
    }

    sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
    terminal_printtermbuffer();
    sprintf(termbuffer, "Filesize: %lu bytes", _filesize_current_file);
    terminal_printtermbuffer();
    print_info("Clusters:", 0);
    for(uint8_t i=0; i<LINKEDLIST_SIZE; i++) {
        uint32_t cluster_id = _linkedlist[i];

        if(cluster_id == 0 || cluster_id == 0xFFFFFFFF) {
            break;
        }

        sprintf(termbuffer, "  %02i: %c%08lX", i+1, COL_CYAN, cluster_id);
        terminal_printtermbuffer();
    }
}

void command_run(void) {
    print_info("Searching file...", 1);

    int fileid = atoi(&__lastinput[3]);

    // find a file and store its cluster structure into the linked list
    if(read_file_metadata(fileid) != 0) {
        return;
    }

    if(strcmp(_ext, "CAS") != 0) {
        print_error("Cannot run a non-CAS file.");
        return;
    }

    sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
    terminal_printtermbuffer();
    sprintf(termbuffer, "Filesize: %lu bytes", _filesize_current_file);
    terminal_printtermbuffer();
    
    set_ram_bank(1);
    store_cas_ram(_linkedlist[0], 0x0000, 1);

    uint16_t deploy_addr = ram_read_uint16_t(0x8000);
    uint16_t file_length = ram_read_uint16_t(0x8002);

    sprintf(termbuffer, "Deploy addr: %c0x%04X", COL_CYAN, deploy_addr);
    terminal_printtermbuffer();
    sprintf(termbuffer, "Program length: %c0x%04X", COL_CYAN, file_length);
    terminal_printtermbuffer();
    sprintf(termbuffer, "Top RAM: %c0x%04X", COL_CYAN, deploy_addr + file_length);
    terminal_printtermbuffer();

    print_info("Press c to calculate checksum or any", 0);
    print_info("or any other key to launch program.", 0);
    if(wait_for_key_fixed(28) == 1) {
        // calculate CRC16 checksum
        print_info("Calculating CRC16 checksum...", 1);
        uint16_t crc16 = crc16_ramchip(0x0000, file_length);
        sprintf(termbuffer, "CRC16 checksum: %c0x%04X", COL_CYAN, crc16);
        terminal_printtermbuffer();

        print_info("Press any key to start program", 0);
        wait_for_key();
    }

    set_ram_bank(0);
    __bootcas = 1;
}

void command_hexdump(void) {
    int file_id = atoi(&__lastinput[7]);

    if(read_file_metadata(file_id) != 0) {
        return;
    }

    // read the first sector of the file
    read_sector(get_sector_addr(_linkedlist[0], 0));

    sprintf(termbuffer, "Filename: %s.%s", _basename, _ext);
    terminal_printtermbuffer();

    // print sectorblock to screen
    for(uint8_t i=0; i<16; i++) {
        terminal_hexdump(i*8, &_sectorblock[i*8]);
    }
}

void command_ledtest(void) {
    z80_outp(LED_IO, 0x00);
    z80_delay_ms(500);
    z80_outp(LED_IO, 0x01);
    z80_delay_ms(500);
    z80_outp(LED_IO, 0x02);
    z80_delay_ms(500);
    z80_outp(LED_IO, 0x00);
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

    if(ram_read_uint32_t(SECTOR_CACHE_ADDR) == _current_folder_cluster &&
       file_id < ram_read_uint32_t(SECTOR_CACHE_SIZE)) {

        uint32_t cluster = ram_read_uint32_t(SECTOR_CACHE + (file_id-1) *4);
        uint8_t entry_id = ram_read_byte(ENTRY_CACHE + file_id - 1);

        read_sector(cluster);
        uint32_t fc = store_file_metadata(entry_id);
        build_linked_list(fc);

        return 0;
    }

    uint32_t cluster = read_folder(_current_folder_cluster, file_id);
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