#define ADDR0 2
#define ADDR1 3
#define RD    4
#define WR    5

/**
 * Initialize the SD card
 */
void init_sdcard() {
  pinMode(ADDR0, OUTPUT);
  pinMode(ADDR1, OUTPUT);
  pinMode(RD, OUTPUT);
  pinMode(WR, OUTPUT);

  digitalWrite(ADDR0, LOW);
  digitalWrite(ADDR1, LOW);
  digitalWrite(RD, HIGH); // inactive
  digitalWrite(WR, HIGH); // inactive

  // use port L as output
  DDRL = 0xFF;

  // write 0x00 to L
  PORTL = 0x00;

  Serial.begin(9600);

  sdout_set();
}

/**
 * Write a byte to the shift register
 */
void write_byte(uint8_t c) {
  DDRL = 0xFF;
  PORTE = (1 << 3) | (0x00 << 4); // set address
  PORTL = c;                      // set byte
  PORTE = 0x00;                   // pull write low
  PORTE = (1 << 3) | (0x00 << 4); // pull write high

  // set address
  PORTE = (0x01 << 4) | (1 << 3); // set address
  PORTE = (0x01 << 4);            // pull write low
  PORTE = (0x01 << 4) | (1 << 3); // pull write high

  __builtin_avr_delay_cycles(12);
}

/**
 * COMMANDS
 */

/**
 * CMD0: Reset the SD Memory Card
 */
void cmd0(uint8_t* resp) {
  static uint8_t CMD[] = {0|0x40,0,0,0,0,0x94|0x01};
  send_command(CMD);
  receive_R1(resp);
}

/**
 * CMD8: Sends interface condition
 */
void cmd8(uint8_t* resp) {
  static uint8_t CMD[] = {8|0x40,0,0,0x01,0xaa,0x86|0x01};
  send_command(CMD);
  receive_R7(resp);
}

/**
 * CMD17: Read block
 */
void cmd17(uint8_t* resp, uint32_t addr) {
  uint8_t CMD[] = {17|0x40,0,0,0,0,0x00|0x01};

  // reverse byte order as SD card commands are big endian
  CMD[1] = (uint8_t)((addr >> 24) & 0xFF);
  CMD[2] = (uint8_t)((addr >> 16) & 0xFF);
  CMD[3] = (uint8_t)((addr >>  8) & 0xFF);
  CMD[4] = (uint8_t)((addr >>  0) & 0xFF);
  send_command(CMD);
  receive_R1(resp);
  uint8_t c = 0;
  while(c != 0xFE) {  // keep on grabbing bytes until "FE" is read
    c = read_byte();  
  }
}

/**
 * CMD55: Next command is application specific command
 */
void cmd55(uint8_t* resp) {
  static uint8_t CMD[] = {55|0x40,0,0,0,0,0x00|0x01};
  send_command(CMD);
  receive_R1(resp);
}

/**
 * CMD58: Read OCR register
 */
void cmd58(uint8_t* resp) {
  static uint8_t CMD[] = {58|0x40,0,0,0,0,0x00|0x01};
  send_command(CMD);
  receive_R1(resp);
}

/**
 * ACMD41: Send host capacity support information
 */
void acmd41(uint8_t* resp) {
  static uint8_t CMD[] = {41|0x40,0x40,0,0,0,0x00|0x01};
  //open_command();
  send_command(CMD);
  resp[0] = read_byte();
  //close_command();
}

/**
 * AUXILARY COMMAND FUNCTIONS
 */

/**
 * Send a command to the SD card, the cmdbytes array
 * contains 6 bytes.
 */
void send_command(uint8_t* cmdbytes) {
  for(uint8_t i=0; i<6; i++) {
    write_byte(cmdbytes[i]);
  }

  write_byte(0xFF);
}

/**
 * Open the command interface
 */
void open_command() {
  write_byte(0xFF);
  cs_set();
  write_byte(0xFF);
}

/**
 * Close the command interface
 */
void close_command() {
  write_byte(0xFF);
  cs_reset();
  write_byte(0xFF); 
}

/*
 * RESPONSE FUNCTIONS
 */

/**
 * Receive a response R1
 * 
 * Uses a response buffer object to write data to
 */
void receive_R1(uint8_t* resp) {
  write_byte(0xFF);
  resp[0] = read_byte();
}

/**
 * Receive a response R7
 * 
 * Uses a response buffer object to write data to
 */
void receive_R7(uint8_t* resp) {
  write_byte(0xFF);
  for(uint8_t i=0; i<5; i++) {
    resp[i] = read_byte();
  }
}

/*
 * Response output function
 */
void print_response(const char* str, const uint8_t* resp, uint8_t numbytes) {
  Serial.print(str);
  Serial.print(": ");
  for(uint8_t i=0; i<numbytes; i++) {
    if(resp[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(resp[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void print_datablock(const uint8_t* buf) {
  for(int i=0; i<16; i++) {
    print_response("CMD17", &block[i*32], 32);
  }
  print_response("CHECKSUM", &block[512], 2);
}

/**
 * Read a byte from the shift register after sending
 * 8 high bytes
 */
uint8_t read_byte() {
  write_byte(0xFF);

  DDRL = 0x00;
  PORTE = (0x00 << 4) | (1 << 3); // set address
  PORTG &= ~(1 << 5);             // pull read low
  uint8_t c = PINL;
  PORTG |=  (1 << 5);             // pull read high
  DDRL = 0xFF;

  return c;
}

/**
 * Read a 512 byte block including the 2 bytes checksum
 */
void read_block(uint8_t* buf) {
  for(unsigned int i=0; i<514; i++) {
    buf[i] = read_byte();
  }
}

/**
 * Reset SDOUT
 */
void sdout_reset() {
  PORTE = (1 << 3) | (0x02 << 4); // set address
  PORTG &= ~(1 << 5);             // pull read low
  PORTG |=  (1 << 5);             // pull read high
}

/**
 * Set SDOUT
 */
void sdout_set() {
  PORTE = (1 << 3) | (0x03 << 4); // set address
  PORTG &= ~(1 << 5);             // pull read low
  PORTG |=  (1 << 5);             // pull read high
}

/**
 * Reset CS
 */
void cs_reset() {
  PORTE = (1 << 3) | (0x02 << 4); // set address
  PORTE = (0x02 << 4);            // pull write low
  PORTE = (0x02 << 4) | (1 << 3); // pull write high
}

/**
 * Set CS
 */
void cs_set() {
  PORTE = (1 << 3) | (0x03 << 4); // set address
  PORTE = (0x03 << 4);            // pull write low
  PORTE = (0x03 << 4) | (1 << 3); // pull write high
}
