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

#include "util.h"

/**
 * @brief Wait for key-press
 *
 */
void wait_for_key(void) {
    keymem[0x0C] = 0;
    while(keymem[0x0C] == 0) {} // wait until a key is pressed
}

/**
 * @brief Wait but check for a specific key press
 *
 */
uint8_t wait_for_key_fixed(uint8_t quitkey) {
    wait_for_key();
    if(keymem[0x00] == quitkey) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Clear the screen
 * 
 */
void clear_screen(void) {
    memset(vidmem, 0x00, 0x1000);
}