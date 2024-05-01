#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <string.h>
#include <stdlib.h>
#include <z80.h>

#include "constants.h"
#include "sst39sf.h"
#include "sdcard.h"
#include "memory.h"
#include "terminal.h"
#include "terminal_ext.h"
#include "fat32.h"
#include "ram.h"
#include "leds.h"

#define __clock_freq 2500000

extern char __lastinput[INPUTLENGTH];
extern uint8_t __bootcas;

void commands_ls(void);

/**
 * @brief List contents of a folder and parse CAS files
 * 
 */
void command_lscas(void);

/**
 * @brief change directory to folder indicated by id
 */
void command_cd(void);

/**
 * @brief Obtain info of a given file
 * 
 */
void command_fileinfo(void);

/**
 * @brief Load a (CAS) file into memory and launch it
 * 
 */
void command_run(void);

/**
 * @brief Test burning of read and write LEDs
 * 
 */
void command_ledtest(void);

/**
 * @brief Execute the command given by instruction
 * 
 */
void execute_command(void);

// *****************************************************************************
// AUXILIARY ROUTINES
// *****************************************************************************

/**
 * @brief Read the metadata of a file identified by id
 * 
 * @param file_id iterator corresponding to ith file in folder
 * @return uint8_t whether file can be read, 0 true, 1 false
 */
uint8_t read_file_metadata(int16_t file_id);

/**
 * @brief Check whether the SD card is mounted
 * 
 * @return uint8_t 0 if mounted, 1 if not
 */
uint8_t check_mounted(void);

#endif // _COMMANDS_H