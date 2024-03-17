#ifndef _COPY_H
#define _COPY_H

#include "ram.h"
#include "sst39sf.h"

/**
 * @brief Copy bytes from external RAM to external ROM
 * 
 * @param ram_src starting position in RAM
 * @param nbytes  number of bytes to copy
 * @param verbose whether to show verbose output
 */
void copy_ram_rom(uint16_t ram_src, uint16_t nbytes, uint8_t verbose);

/**
 * @brief Create a CRC-16 checksum of the first 0x4000 bytes
 *        of the internal ROM.
 * 
 * This routine is only used for the firmware flasher.
 */
void calculatecrc16(void);

#endif // _COPY_H