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

#ifndef LAUNCH_H
#define LAUNCH_H

#include <z80.h>

/**
 * @brief Copy the cas program from the external RAM to the internal RAM and the call boot_addr
 * 
 */
void launch_cas(uint16_t boot_addr) __z88dk_callee;

/**
 * @brief Call the address in the internal RAM
 * 
 * @param addr address to call
 */
void call_addr(uint16_t addr) __z88dk_fastcall;

#endif // LAUNCH_H