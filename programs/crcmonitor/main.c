#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "util.h"
#include "crc16.h"

void init(void);

int main(void) {
    init();

    print_info("This program calculates the CRC-16", 0);
    print_info("checksum of the P2000T monitor routines.", 0);
    print_info("", 0); // empty line

    uint16_t checksum = crc16(0x0000, 0x1000);
    sprintf(termbuffer, "CRC-16 checksum: %c0x%04X", COL_CYAN, checksum);
    terminal_printtermbuffer();

    // wait for key press
    print_info("Press any key to return to menu...", 0);
    wait_for_key();

    return 0;
}

void init(void) {
    memset(vidmem, 0x00, 0x1000);
    terminal_init(3, 20);
    sprintf(&vidmem[0x50], "%c%cMONITOR CRC16 CHECKSUM", TEXT_DOUBLE, COL_CYAN);
}