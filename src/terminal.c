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

void print_info(char* str, uint8_t backup_line) {
    sprintf(termbuffer, str);
    terminal_printtermbuffer();
    if(backup_line == 1) {
        terminal_backup_line();
    }
}

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

void terminal_hexdump(uint16_t addr, uint8_t *mem) {
    sprintf(termbuffer, "%c%04X", COL_YELLOW, addr);
    for(uint8_t i=0; i<8; i++) {
        sprintf(&termbuffer[5+i*3], "%c%02X", COL_WHITE, mem[i]);
    }

    termbuffer[5+8*3] = COL_CYAN;

    for(uint8_t i=0; i<8; i++) {
        if(mem[i] >= 32 && mem[i] <= 127) {
            termbuffer[6+8*3+i] = mem[i];
        } else {
            termbuffer[6+8*3+i] = '.';
        }
    }
    terminal_printtermbuffer();
}