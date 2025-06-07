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

#include "sdcard.h"

// shared buffer object to store the data of a single sector on the SD card
uint8_t _resp8[5];
uint8_t _resp58[5];
uint8_t _flag_sdcard_mounted = 0;

/**
 * @brief Output information of the SD-CARD to the user
 * 
 */
uint8_t init_sdcard(void) {
    // mount sd card
#ifndef NON_VERBOSE
    print_recall("Initializing SD card..");
#endif
    
    // settings ~CS and PULL-UP resistor
    sdcs_set();
    sdout_set();

    // reset SD card
    sdpulse();

    // byte for capturing responses
    uint8_t c = 0x00;

    // CMD0: Reset the SD Memory Card
    open_command();
    c = cmd0();
    close_command();

    // CMD8: Sends interface condition    
    open_command();
    cmd8(_resp8);
    close_command();

    // output resp8 to terminal
    // Typical responses:
    // Intenso cards: 01 00 00 00 01 AA
    // sprintf(termbuffer, "CMD8: %02X %02X %02X %02X %02X", _resp8[0], _resp8[1], _resp8[2], _resp8[3], _resp8[4]);
    // terminal_printtermbuffer();

    if(_resp8[0] >= 0x02) {
        return -1;
    }

    // keep on looping until zero result is found
    c = 0xFF;
    uint16_t ctr = 0;
    while(c != 0 & ctr < 1000) {
        open_command();
        cmd55();
        close_command();
        open_command();
        c = acmd41();   // Send host capacity support information
        close_command();
        ctr++;
    }

    if(ctr == 1000) {
#ifndef NON_VERBOSE
        print_error("SD card time-out");
#endif
        return -1;
    } else {
        // sprintf(termbuffer, "ACMD41 attempts: %i", ctr);
        // terminal_printtermbuffer();
    }

    // CMD53: Read OCR register
    open_command();
    cmd58(_resp58);
    close_command();

    // output resp58 to terminal
    // sprintf(termbuffer, "CMD58: %02X %02X %02X %02X %02X", _resp58[0], _resp58[1], _resp58[2], _resp58[3], _resp58[4]);
    // terminal_printtermbuffer();

    // inform user that the SD card is initialized and that we are ready to read
    // the first block from the SD card and print it to the screen
#ifndef NON_VERBOSE
    print("SD Card initialized");
#endif

    return 0;
}

uint8_t read_sector(uint32_t sec_addr) { 
    return read_sector_to(sec_addr, SDCACHE0); 
}