#ifndef _UINT_UTIL_H
#define _UINT_UTIL_H

#include "terminal.h"

/**
 * @brief Replace all bytes in a string
 * 
 * @param str original string
 * @param org byte to replace
 * @param rep replacement byte
 * @param nrbytes number of bytes to check
 */
void replace_bytes(uint8_t *str, uint8_t org, uint8_t rep, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Read a 16 bit value from memory
 * 
 * @param data memory location
 * @return uint16_t 16-bit value
 */
uint16_t read_uint16_t(const uint8_t* data) __z88dk_callee;

/**
 * @brief Read a 32 bit value from memory
 * 
 * @param data memory location
 * @return uint32_t 32-bit value
 */
uint32_t read_uint32_t(const uint8_t* data) __z88dk_callee;

/**
 * @brief Wait for key-press
 *
 */
void wait_for_key(void);

/**
 * @brief Wait but check for a specific key press
 *
 */
uint8_t wait_for_key_fixed(uint8_t quitkey);

/**
 * @brief Clear the screen
 * 
 */
void clear_screen(void);

#endif //_UINT_UTIL_H