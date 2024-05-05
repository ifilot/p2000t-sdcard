;-------------------------------------------------------------------------------
;                                                                       
;   Author: Ivo Filot <ivo@ivofilot.nl>                                 
;                                                                       
;   P2000T-SDCARD is free software:                                     
;   you can redistribute it and/or modify it under the terms of the     
;   GNU General Public License as published by the Free Software        
;   Foundation, either version 3 of the License, or (at your option)    
;   any later version.                                                  
;                                                                       
;   P2000T-SDCARD is distributed in the hope that it will be useful,    
;   but WITHOUT ANY WARRANTY; without even the implied warranty         
;   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.             
;   See the GNU General Public License for more details.                
;                                                                       
;   You should have received a copy of the GNU General Public License   
;   along with this program.  If not, see http://www.gnu.org/licenses/. 
;                                                                       
;-------------------------------------------------------------------------------

SECTION code_user

ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
RAM_BANK        EQU  $6B
ROM_IO          EQU  $6C

PUBLIC _rom_read_byte

;-------------------------------------------------------------------------------
; Read a byte from external RAM
;
; uint8_t ram_read_byte(uint16_t addr);
;
; input:  hl pointer to ram address
;
; return: value stored at memory address
;-------------------------------------------------------------------------------
_rom_read_byte:
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    in a,(ROM_IO)
    ld l,a                      ; put return value in l-register
    ret