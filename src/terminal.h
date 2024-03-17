#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "commands.h"

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
void print_info(char* str, uint8_t backup_line);

void terminal_cursor_blink(void);
void terminal_hexdump(uint16_t addr, uint8_t *mem);

#endif // _TERMINAL_H