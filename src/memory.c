#include "memory.h"

// set video memory
__at (0x0000) char MEMORY[];
char* memory = MEMORY;

__at (0x5000) char VIDMEM[];
char* vidmem = VIDMEM;

__at (0x6000) char KEYMEM[];
char* keymem = KEYMEM;

__at (0xA000) char HIGHMEM[];
char* highmem = HIGHMEM;

__at (0xE000) char BANKMEM[];
char* bankmem = BANKMEM;