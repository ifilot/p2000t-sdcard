#ifndef _RAM_H
#define _RAM_H

#include <z80.h>
#include <stdint.h>

#include "memory.h"

#define PORT_ADDR_LOW   0x68
#define PORT_ADDR_HIGH  0x69
#define PORT_RAM_BANK   0x6B
#define PORT_RAM        0x6D

#define SECTOR_CACHE_ADDR       0x1000      // cluster address
#define SECTOR_CACHE_SIZE       0x1004      // number of files in cache
#define SECTOR_CACHE            0x1008      // start of sector cache
#define ENTRY_CACHE             0x2000      // file entries

/*
 * The internal memory on the SD-card cartridge has a capacity of 128kb divided
 * over 2x64kb banks. The lower bank is used for caching SD-card data while the
 * upper bank is used for loading in programs.
 */
#define RAM_BANK_CACHE          0
#define RAM_BANK_CASSETTE       1

/**
 * @brief Write a single byte to external RAM
 * 
 * @param addr external memory address
 * @param val nbyte to write
 */
void ram_write_byte(uint16_t addr, uint8_t val) __z88dk_callee;

/**
 * @brief Write fixed byte size to memory, can be used for clearing memory
 * 
 * @param addr memory start address
 * @param val value to write
 * @param num_bytes number of bytes to write
 */
void ram_set(uint16_t addr, uint8_t val, uint16_t num_bytes) __z88dk_callee;

/**
 * @brief Retrieve single byte from external RAM
 * 
 * @param addr external memory address
 * @return uint8_t byte at address
 */
uint8_t ram_read_byte(uint16_t addr) __z88dk_callee;

/**
 * @brief Write 16-bit value to external RAM
 * 
 * @param addr  external memory address
 * @param val 16-bit value to write
 */
void ram_write_uint16_t(uint16_t addr, uint16_t val) __z88dk_callee;

/**
 * @brief Write 32-bit value to external RAM
 * 
 * @param addr  external memory address
 * @param val 16-bit value to write
 */
void ram_write_uint32_t(uint16_t addr, uint32_t val) __z88dk_callee;

/**
 * @brief Retrieve 16 bit value from external RAM
 * 
 * @param addr  external memory address
 * @return uint16_t value to receive
 */
uint16_t ram_read_uint16_t(uint16_t addr) __z88dk_callee;

/**
 * @brief Retrieve 32 bit value from external RAM
 * 
 * @param addr  external memory address
 * @return uint32_t value to receive
 */
uint32_t ram_read_uint32_t(uint16_t addr) __z88dk_callee;

void set_ram_bank(uint8_t val) __z88dk_callee;

/**
 * @brief Copy data from internal memory to external RAM
 * 
 * See: ram.asm
 *
 * @param src      address on external RAM
 * @param dest     internal address
 * @param nrbytes  number of bytes to copy
 */
void copy_to_ram(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Copy data from external memory to internal RAM
 * 
 * See: ram.asm
 *
 * @param src      address on external RAM
 * @param dest     internal address
 * @param nrbytes  number of bytes to copy
 */
void copy_from_ram(uint16_t src, uint8_t *dest, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Calculate CRC16 checksum for N bytes starting at external ram address
 * 
 * @param addr external RAM address
 * @param nrbytes number of bytes to parse
 * @return uint16_t 
 */
uint16_t crc16_ramchip(uint16_t addr, uint16_t nrbytes) __z88dk_callee;

// this function is currently not being used
// void read_from_ram(uint8_t *dest, uint16_t src, uint16_t n);

#endif // _RAM_H