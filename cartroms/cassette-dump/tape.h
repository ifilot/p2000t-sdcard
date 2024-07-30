#ifndef _TAPE_H
#define _TAPE_H

// variables for the cassette header
#define CASSTAT    0x6017
#define TRANSFER   0x6030
#define LENGTH     0x6032
#define FILESIZE   0x6034
#define DESC1      0x6036
#define DESC2      0x6047
#define EXT        0x603E
#define FILETYPE   0x6041
#define BLOCKCTR   0x604F
#define MEMSIZE    0x605C
#define TAPE       0x0018
//#define BUFFER     0x6100

/**
 * @brief Rewind the tape drive
 * 
 */
void tape_rewind(void);

/**
 * @brief Read a single block from the tape
 * 
 * @param location storage location
 */
void tape_read_block(uint8_t *location) __z88dk_fastcall;

/**
 * @brief Skip the tape one block forward
 * 
 */
void tape_skip_forward(void);

#endif // _TAPE_H