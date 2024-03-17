/**
 * @file ram.c
 * @brief Contains routines to interface with SLOT2 ram chip
 */

#include "ram.h"

void set_address(uint16_t addr) {
    z80_outp(PORT_ADDR_LOW, addr & 0xFF);
    z80_outp(PORT_ADDR_HIGH, (addr >> 8) & 0xFF);
}

void ram_write_byte(uint16_t addr, uint8_t val) {
    set_address(addr);
    z80_outp(PORT_RAM, val);
}

uint8_t ram_read_byte(uint16_t addr) {
    set_address(addr);
    return z80_inp(PORT_RAM);
}

void ram_write_uint16_t(uint16_t addr, uint16_t val) {
    ram_write_byte(addr, val & 0xFF);
    ram_write_byte(addr+1, (val >> 8) & 0xFF);
}

void ram_write_uint32_t(uint16_t addr, uint32_t val) {
    ram_write_byte(addr, val & 0xFF);
    ram_write_byte(addr+1, (val >> 8) & 0xFF);
    ram_write_byte(addr+2, (val >> 16) & 0xFF);
    ram_write_byte(addr+3, (val >> 24) & 0xFF);
}

uint32_t ram_read_uint32_t(uint16_t addr) {
    uint32_t res = 0x00;

    res |= ram_read_byte(addr);
    res |= (uint32_t)ram_read_byte(addr+1) << 8;
    res |= (uint32_t)ram_read_byte(addr+2) << 16;
    res |= (uint32_t)ram_read_byte(addr+3) << 24;

    return res;
}

uint16_t ram_read_uint16_t(uint16_t addr) {
    uint16_t res = 0x00;

    res |= ram_read_byte(addr);
    res |= (uint16_t)ram_read_byte(addr+1) << 8;

    return res;
}

void read_from_ram(uint8_t *dest, uint16_t src, uint16_t n) {
    for(uint16_t i=0; i<n; i++) {
        *dest = ram_read_byte(src++);
        dest++;
    }
}

void set_ram_bank(uint8_t val) {
    z80_outp(PORT_RAM_BANK, val);
}