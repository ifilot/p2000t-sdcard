/**************************************************************************
 *                                                                        *
 *   Author: Ivo Filot <ivo@ivofilot.nl>                                  *
 *                                                                        *
 *   P2000T-SDCARD is free software:                                      *
 *   you can redistribute it and/or modify it under the terms of the      *
 *   GNU General Public License as published by the Free Software         *
 *   Foundation, either version 3 of the License, or (at your option)     *
 *   any later version.                                                   *
 *                                                                        *
 *   P2000T-SDCARD is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#include "sst39sf.h"

/**
 * @brief Send a byte to the ROM chip
 * 
 * @param addr address on rom chip
 * @param byte byte to send
 */
void sst39sf_send_byte(uint16_t addr, uint8_t byte) {
    z80_outp(PORT_ADDR_LOW, addr  & 0xFF);
    z80_outp(PORT_ADDR_HIGH, (addr >> 8) & 0xFF);
    z80_outp(PORT_ROM_IO, byte);
}

/**
 * @brief Write a byte on the rom chip
 * 
 * @param addr address to write byte to
 * @param byte byte to write
 */
void sst39sf_write_byte(uint16_t addr, uint8_t byte) {
    sst39sf_send_byte(0x5555, 0xAA);
    sst39sf_send_byte(0x2AAA, 0x55);
    sst39sf_send_byte(0x5555, 0xA0);
    sst39sf_send_byte(addr, byte);
}

/**
 * @brief Read byte from rom chip
 * 
 * @param addr address to read from
 * @return uint8_t 
 */
uint8_t sst39sf_read_byte(uint16_t addr) {
    z80_outp(PORT_ADDR_LOW, addr  & 0xFF);
    z80_outp(PORT_ADDR_HIGH, (addr >> 8) & 0xFF);
    return z80_inp(PORT_ROM_IO);
}

/**
 * @brief Get the device ID
 * 
 * @return uint16_t device id
 */
uint16_t sst39sf_get_device_id(void) {
    // sequence for retrieving device id
    sst39sf_send_byte(0x5555, 0xAA);
    sst39sf_send_byte(0x2AAA, 0x55);
    sst39sf_send_byte(0x5555, 0x90);
    uint8_t id0 = sst39sf_read_byte(0x0000);
    uint8_t id1 = sst39sf_read_byte(0x0001);
    sst39sf_send_byte(0x5555, 0xAA);
    sst39sf_send_byte(0x2AAA, 0x55);
    sst39sf_send_byte(0x5555, 0xF0);

    return ((id1 << 8) | id0);
}

/**
 * @brief Wipe sector (0x1000 bytes) on the ROM chip
 * 
 * @param addr sector to wipe
 */
void sst39sf_wipe_sector(uint16_t addr) {
    sst39sf_send_byte(0x5555, 0xAA);
    sst39sf_send_byte(0x2AAA, 0x55);
    sst39sf_send_byte(0x5555, 0x80);
    sst39sf_send_byte(0x5555, 0xAA);
    sst39sf_send_byte(0x2AAA, 0x55);
    sst39sf_send_byte(addr, 0x30);

    uint16_t attempts = 0;
    while((sst39sf_read_byte(0x0000) & 0x80) != 0x80 && attempts < 1000) {
        attempts++;
    }
}