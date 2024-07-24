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

#include "terminal.h"

uint8_t _terminal_curline = 0;
uint8_t _terminal_maxlines = 0;
uint8_t _terminal_startline = 0;
uint8_t _terminal_endline = 0;
uint16_t _prevcounter = 0;
char termbuffer[LINELENGTH];

char __input[INPUTLENGTH+1];
uint8_t __inputpos = 0;

void terminal_init(uint8_t start, uint8_t stop) {
    _terminal_startline = start;
    _terminal_curline = _terminal_startline;
    _terminal_maxlines = stop - start + 1;
    _terminal_endline = stop;
    memset(termbuffer, 0x00, LINELENGTH);
}

void terminal_printtermbuffer(void) {
    // scroll everything up when we are at the last line
    if(_terminal_curline > _terminal_endline) {
        terminal_scrollup();
        _terminal_curline--;
    }

    // copy buffer to screen
    memcpy(&vidmem[_terminal_curline * 0x50], termbuffer, LINELENGTH);
    memset(termbuffer, 0x00, LINELENGTH);

    // go to next line
    _terminal_curline++;
}

void terminal_redoline(void) {
    if(_terminal_curline > _terminal_endline) {
        terminal_scrollup();
        _terminal_curline--;
    }

    memcpy(&vidmem[_terminal_curline * 0x50], termbuffer, LINELENGTH);
    memset(termbuffer, 0x00, LINELENGTH);
}

void terminal_scrollup(void) {
    for(uint8_t i=_terminal_startline; i<_terminal_endline; i++) {
        memcpy(&vidmem[0x50*i], &vidmem[0x50*(i+1)], LINELENGTH);
    }
    memset(&vidmem[0x50*_terminal_endline], 0x00, LINELENGTH);
}

void terminal_backup_line(void) {
    _terminal_curline--;
}

void print_error(char* str) {
    sprintf(termbuffer, "%cERROR%c%s", COL_RED, COL_WHITE, str);
    terminal_printtermbuffer();
}

/**
 * @brief Print a line to regular terminal, but overwrite the line when a new
 *        line is going to be printed.
 * 
 * @param str 
 */
void print_recall(char* str) {
    print(str);
    terminal_backup_line();
}

/**
 * @brief Print a line to the terminal
 * 
 * @param str 
 */
void print(char* str) {
    strcpy(termbuffer, str);
    terminal_printtermbuffer();
}

/**
 * @brief Produce a blinking cursor
 * 
 */
void terminal_cursor_blink(void) {
    uint16_t counter = read_uint16_t(&memory[0x6010]);

    if((counter - _prevcounter) > (BLINK_INTERVAL / TIMER_INTERVAL)) {
        _prevcounter = counter;
        uint16_t cursorpos = _terminal_curline * 0x50 + __inputpos + 3;
        if(vidmem[cursorpos] != 127) {
            vidmem[cursorpos] = 127;
        } else {
            vidmem[cursorpos] = 0;
        }
    }
}