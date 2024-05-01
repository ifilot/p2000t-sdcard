#ifndef _TERMINAL_EXT_H
#define _TERMINAL_EXT_H

#include <stdio.h>
#include <string.h>
#include "ram.h"
#include "terminal.h"

/**
 * @brief Perform hexdump from part of external RAM
 * 
 * @param addr external RAM address
 */
void terminal_hexdump_ram(uint16_t addr);

/**
 * @brief Perform hexdump from part of internal RAM
 * 
 * @param buf buffer location
 */
void printblock(const uint8_t* buf);

#endif