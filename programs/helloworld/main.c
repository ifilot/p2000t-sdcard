#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "util.h"

int main(void) {
    // clear screen
    memset(vidmem, 0x00, 0x1000);

    // write hello world
    memcpy(vidmem, "Hello world!", 11);

    memcpy(&vidmem[0x50], "Press any key to exit program...", 32);

    // wait for key press
    wait_for_key();

    return 0;
}