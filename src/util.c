#include "util.h"

// uint16_t read_uint16_t(const uint8_t* data) {
//     return (*(data+1)) << 8 | (*data);
// }

// uint32_t read_uint32_t(const uint8_t* data) {
//     return (uint32_t)(*(data+3)) << 24 |
//            (uint32_t)(*(data+2)) << 16 | 
//            (uint32_t)(*(data+1)) << 8 |
//            (uint32_t)(*data);
// }

/**
 * @brief Wait for key-press
 *
 */
void wait_for_key(void) {
    keymem[0x0C] = 0;
    while(keymem[0x0C] == 0) {} // wait until a key is pressed
}

/**
 * @brief Wait but check for a specific key press
 *
 */
uint8_t wait_for_key_fixed(uint8_t quitkey) {
    wait_for_key();
    if(keymem[0x00] == quitkey) {
        return 1;
    } else {
        return 0;
    }
}

void clear_screen(void) {
    memset(vidmem, 0x00, 0x1000);
}