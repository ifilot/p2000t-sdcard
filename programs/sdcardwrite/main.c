#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "util.h"
#include "crc16.h"
#include "sdcard.h"
#include "fat32.h"

void init(void);

int main(void) {
    init();

    print_info("This program aims to write data to a", 0);
    print_info("specific file on the SD card.", 0);
    print_info("", 0); // empty line

    // mount sd card
    print_info("Initializing SD card..", 1);
    if(init_sdcard(_resp8, _resp58) != 0) {
        print_error("Cannot connect to SD-CARD.");
        for(;;){}
    }

    // inform user that the SD card is initialized and that we are ready to read
    // the first block from the SD card and print it to the screen
    print_info("SD Card initialized", 0);

    print_info("Mounting partition 1..", 1);
    uint32_t lba0 = read_mbr();
    read_partition(lba0);

    // sd card successfully mounted
    print_info("Partition 1 mounted", 0);
    print_info("System ready.", 0);

    // wait for key press
    print_info("Press any key to return to menu...", 0);
    wait_for_key();

    return 0;
}

void init(void) {
    memset(vidmem, 0x00, 0x1000);
    terminal_init(3, 20);
    sprintf(&vidmem[0x50], "%c%cSDCARD WRITE TEST", TEXT_DOUBLE, COL_CYAN);
}