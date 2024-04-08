#ifndef _SST39SF_H
#define _SST39SF_H

#include <z80.h>

#define ROM_ADDR_LOW  0x68
#define ROM_ADDR_HIGH 0x69
#define ROM_BANK      0x6A
#define ROM_IO        0x6C

/**
 * @brief Send a byte to the ROM chip
 * 
 * @param addr address on rom chip
 * @param byte byte to send
 */
void sst39sf_send_byte(uint16_t addr, uint8_t byte);

/**
 * @brief Write a byte on the rom chip
 * 
 * @param addr address to write byte to
 * @param byte byte to write
 */
void sst39sf_write_byte(uint16_t addr, uint8_t byte);

/**
 * @brief Read byte from rom chip
 * 
 * @param addr address to read from
 * @return uint8_t 
 */
uint8_t sst39sf_read_byte(uint16_t addr);

/**
 * @brief Get the device ID
 * 
 * @return uint16_t device id
 */
uint16_t sst39sf_get_device_id(void);

/**
 * @brief Wipe sector (0x1000 bytes) on the ROM chip
 * 
 * @param addr sector to wipe
 */
void sst39sf_wipe_sector(uint16_t addr);

/**
 * @brief Set the rom bank
 * 
 * @param rom_bank rom bank index (0 or 1)
 */
void set_rom_bank(uint8_t rom_bank);

/**
 * @brief Copy data from internal memory to external RAM
 * 
 * See: sst39sf.asm
 *
 * @param src      address on external ROM
 * @param dest     internal address
 * @param nrbytes  number of bytes to copy
 */
void copy_to_rom(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;

/**
 * @brief Calculate CRC16 checksum on ROM
 * 
 * See: sst39sf.asm
 *
 * @param addr start address
 * @param nrbytes number of bytes to evaluate
 * @return uint16_t CRC-16 checksum
 */
uint16_t crc16_romchip(uint16_t addr, uint16_t nrbytes) __z88dk_callee;

#endif