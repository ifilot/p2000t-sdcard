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

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "util.h"

#define LINELENGTH 40
#define BLINK_INTERVAL 500 // ms
#define TIMER_INTERVAL 20

// these (global) variables are used to track the terminal
extern uint8_t _terminal_curline;
extern uint8_t _terminal_maxlines;
extern uint8_t _terminal_startline;
extern uint8_t _terminal_endline;
extern uint16_t _prevcounter;

extern char __input[INPUTLENGTH+1];
extern uint8_t __inputpos;

extern char termbuffer[LINELENGTH];

void terminal_init(uint8_t, uint8_t);
void terminal_printtermbuffer(void);
void terminal_redoline(void);
void terminal_scrollup(void);
void terminal_backup_line(void);

void print_error(char* str);

/**
 * @brief Print a line to the terminal
 * 
 * @param str 
 */
void print(char* str);

/**
 * @brief Print a line to regular terminal, but overwrite the line when a new
 *        line is going to be printed.
 * 
 * @param str 
 */
void print_recall(char* str);

/**
 * @brief Produce a blinking cursor
 * 
 */
void terminal_cursor_blink(void);

#endif // _TERMINAL_H