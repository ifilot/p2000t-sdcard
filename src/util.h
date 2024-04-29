#ifndef _UINT_UTIL_H
#define _UINT_UTIL_H

#include "terminal.h"

void replace_bytes(uint8_t* str, uint8_t org, uint8_t rep, uint16_t nrbytes) __z88dk_callee;

uint16_t read_uint16_t(const uint8_t* data);

uint32_t read_uint32_t(const uint8_t* data);

void wait_for_key(void);

uint8_t wait_for_key_fixed(uint8_t quitkey);

void clear_screen(void);

#endif //_UINT_UTIL_H