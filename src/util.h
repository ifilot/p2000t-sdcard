#ifndef _UINT_UTIL_H
#define _UINT_UTIL_H

#include "terminal.h"

uint16_t read_uint16_t(const uint8_t* data);
uint32_t read_uint32_t(const uint8_t* data);
void wait_for_key(void);
uint8_t wait_for_key_fixed(uint8_t quitkey);
void clear_screen(void);

#endif //_UINT_UTIL_H