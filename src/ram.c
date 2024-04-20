/**
 * @file ram.c
 * @brief Contains routines to interface with SLOT2 ram chip
 */

#include "ram.h"

void read_from_ram(uint8_t *dest, uint16_t src, uint16_t n) {
    for(uint16_t i=0; i<n; i++) {
        *dest = ram_read_byte(src++);
        dest++;
    }
}