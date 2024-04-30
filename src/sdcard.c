#include "sdcard.h"

// shared buffer object to store the data of a single sector on the SD card
uint8_t _sectorblock[514];
uint8_t _resp8[5];
uint8_t _resp58[5];
uint8_t _flag_sdcard_mounted = 0;

/**
 * Write value on the databus into the PISO register
 */
void serial_write(uint8_t val) {
    z80_outp(0x60, val);
}

/**
 * Send byte in OUT register to SD card using 8 clock pulses
 */
void clkstart(void) {
    z80_outp(0x61, 0x00);
}

/**
 * Read value from the SIPO register
 */
uint8_t serial_read(void) {
    return z80_inp(0x60);
}

/**
 * @brief      Send a byte to the SD card
 *
 * @param[in]  val   The value
 */
void send_byte(uint8_t val) {
    serial_write(val);
    clkstart();
}

/**
 * @brief      Receive a byte from the SD card
 *
 * @return     Received byte
 */
uint8_t receive_byte(void) {
    send_byte(0xFF);
    return serial_read();
}

/**
 * @brief      Send a command (6 bytes) to the SD card
 *
 * @param      Pointer to command to send
 */
void send_command(uint8_t *vals) {
    for(uint8_t i=0; i<6; i++) {
        send_byte(vals[i]);
    }
}

/******************************************************************************
 * COMMAND OPERATIONS
 ******************************************************************************

/**
 * CMD17: Read block
 */
void cmd17(uint32_t addr) {
  uint8_t CMD[] = {17|0x40,0,0,0,0x00,0x00|0x01};

  // reverse byte order as SD card commands are big endian
  CMD[1] = (uint8_t)((addr >> 24) & 0xFF);
  CMD[2] = (uint8_t)((addr >> 16) & 0xFF);
  CMD[3] = (uint8_t)((addr >>  8) & 0xFF);
  CMD[4] = (uint8_t)((addr >>  0) & 0xFF);
  send_command(CMD);
  receive_R1();
  uint8_t c = 0;
  while(c != 0xFE) {  // keep on grabbing bytes until "FE" is read
    c = receive_byte();  
  }
}

/******************************************************************************
 * BLOCK OPERATIONS
 ******************************************************************************/

/**
 * @brief Read a single 512-byte sector
 * 
 * @param addr sector address
 */
void read_sector(uint32_t addr) {
    open_command();
    cmd17(addr);
    read_block(_sectorblock);
    close_command();
}