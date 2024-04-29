#include "sdcard.h"

// shared buffer object to store the data of a single sector on the SD card
uint8_t _sectorblock[514];
uint8_t _resp[7];
uint8_t _flag_sdcard_mounted = 0;

void init_sdcard(void) {
    // set chip select to low, activating the SD-card
    sdcs_set();

    // pull MISO to low via a 10k resistor
    sdout_set();

    // first send 96 pulses (12 * 8 bits) to the SD card 
    for(uint8_t i=0; i<12; i++) {
        send_byte(0xFF);
    }

    // send CMD0: Reset the SD Memory Card
    open_command();
    cmd0();
    close_command();

    #ifdef SDCARD_VERBOSE
        sprintf(termbuffer, "CMD0: %02X", _resp[0]);
        terminal_printtermbuffer();
    #endif

    // send CMD8: Sends interface condition
    open_command();
    cmd8();
    close_command();

    #ifdef SDCARD_VERBOSE
        sprintf(termbuffer, "CMD8: %02X %02X %02X %02X %02X", _resp[0], _resp[1], _resp[2], _resp[3], _resp[4]);
        terminal_printtermbuffer();
    #endif

    // keep polling until we have received host capacity information
    _resp[0] = 5;
    uint16_t ctr = 0;
    while(_resp[0] != 0) {
        ctr++;
        open_command();
        cmd55();
        close_command();
    
        open_command();
        acmd41();
        close_command();

        #ifdef SDCARD_VERBOSE
            sprintf(termbuffer, "ACMD41: %02X", _resp[0]);
            terminal_redoline();
        #endif
    }

    #ifdef SDCARD_VERBOSE
      // provide number of responds and response value
        sprintf(termbuffer, "ACMD41 accepted (%i attempts)", ctr);
        terminal_printtermbuffer();
    #endif
    

    // CMD58: read OCR register - R3 response (5 bytes)
    open_command();
    cmd58();
    close_command();

    #ifdef SDCARD_VERBOSE
        sprintf(termbuffer, "CMD58: %02X %02X %02X %02X %02X", _resp[0], _resp[1], _resp[2], _resp[3], _resp[4]);
        terminal_printtermbuffer();
    #endif
}

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
 * Open the command interface
 */
void open_command(void) {
    send_byte(0xFF);
    sdcs_reset();
    send_byte(0xFF);
}

/**
 * Close the command interface
 */
void close_command(void) {
    send_byte(0xFF);
    sdcs_set();
    send_byte(0xFF); 
}

/**
 * CMD0: Reset the SD Memory Card
 */
void cmd0(void) {
    static uint8_t CMD[] = {0|0x40,0,0,0,0,0x94|0x01};
    send_command(CMD);
    receive_R1();
}

/**
 * CMD8: Sends interface condition
 */
void cmd8(void) {
  static uint8_t CMD[] = {8|0x40,0,0,0x01,0xaa,0x86|0x01};
  send_command(CMD);
  receive_R7();
}

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

/**
 * CMD55: Next command is application specific command
 */
void cmd55(void) {
  static uint8_t CMD[] = {55|0x40,0,0,0,0,0x00|0x01};
  send_command(CMD);
  receive_R1();
}

/**
 * CMD58: Read OCR register
 */
void cmd58(void) {
  static uint8_t CMD[] = {58|0x40,0,0,0,0,0x00|0x01};
  send_command(CMD);
  receive_R3();
}

/**
 * ACMD41: Send host capacity support information
 */
void acmd41(void) {
  static uint8_t CMD[] = {41|0x40,0x40,0x00,0x00,0x00,0x00|0x01};
  send_command(CMD);
  receive_R1();
}

/******************************************************************************
 * RECEIVE OPERATIONS
 ******************************************************************************/

/**
 * Receive a response R1
 * 
 * Uses a response buffer object to write data to
 */
void receive_R1(void) {
    send_byte(0xFF);
    _resp[0] = receive_byte();
}

/**
 * Receive a response R3
 * 
 * Uses a response buffer object to write data to
 */
void receive_R3(void) {
  send_byte(0xFF);
  for(uint8_t i=0; i<5; i++) {
    _resp[i] = receive_byte();
  }
}

/**
 * Receive a response R7
 * 
 * Uses a response buffer object to write data to
 */
void receive_R7(void) {
  send_byte(0xFF);
  for(uint8_t i=0; i<5; i++) {
    _resp[i] = receive_byte();
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

/******************************************************************************
 * I/O CONTROL
 ******************************************************************************/

 /**
 * SDOUT is pulled low, pulling MISO low via a 10k resistor
 */
void sdout_set(void) {
    z80_inp(0x62);
}

/**
 * SDOUT is pulled high, pulling MISO high via a 10k resistor
 */
void sdout_reset(void) {
    z80_inp(0x63);
}

/**
 * Set the SD CS signal to low (activating the SD card)
 */
void sdcs_set(void) {
    z80_outp(0x62, 0x00);
}

/**
 * Set the SD CS signal to high (deactivating the SD card)
 */
void sdcs_reset(void) {
    z80_outp(0x63, 0x00);
}