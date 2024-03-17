#include <stdio.h>
#include <string.h>

/**
 * Create reference to video memory
 */
__at (0x5000) char VIDMEM[];
char* vidmem = VIDMEM;

int main(void) {
    memset(&vidmem[0x0000], 0x00, 0x1000);
    sprintf(&vidmem[0x0000], "Hello world!");

    for(;;){} // infinite loop
}