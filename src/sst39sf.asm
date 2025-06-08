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

INCLUDE "ports.inc"

PUBLIC _copy_to_rom
PUBLIC _fast_sd_to_rom_full

;-------------------------------------------------------------------------------
; Copy bytes to external ROM chip
;
; void copy_to_rom(uint8_t *src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;
;
; input:  hl - source address
;         de - destination address
;         bc - number of bytes
; uses: all
;-------------------------------------------------------------------------------
_copy_to_rom:
    ld a,1
    out (LED_IO),a              ; turn ROM led on
    pop iy                      ; return address
    pop hl                      ; src
    pop de                      ; dest
    pop bc                      ; number of bytes
    push iy                     ; put return address back on stack
next:
    ; send 0xAA to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$AA
    out (ROM_IO),a
    
    ; send 0x55 to 0x2AAAA
    ld a,$2A
    out (ADDR_HIGH),a
    ld a,$AA
    out (ADDR_LOW),a
    ld a,$55
    out (ROM_IO),a

    ; send 0xA0 to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$A0
    out (ROM_IO),a

    ; send byte from (hl) to ROM address in (de)
    ld a,d
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    ld a,(hl)
    out (ROM_IO),a

    ; increment addresses and decrement counters
    inc de
    inc hl
    dec bc
    ld a,c
    or b
    jp nz, next
    ld a,0
    out (LED_IO),a              ; turn ROM led off
    ret

;-------------------------------------------------------------------------------
; Copy bytes to external ROM chip
;
; void fast_sd_to_rom_full(uint16_t rom_addr) __z88dk_callee;
;-------------------------------------------------------------------------------
_fast_sd_to_rom_full:
    ld a,1
    out (LED_IO),a              ; turn ROM led on
    pop iy                      ; return address
    pop de                      ; dest
    push iy                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,2
copyouter:
    ld b,0
copynext:
    ; send 0xAA to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$AA
    out (ROM_IO),a
    
    ; send 0x55 to 0x2AAAA
    ld a,$2A
    out (ADDR_HIGH),a
    ld a,$AA
    out (ADDR_LOW),a
    ld a,$55
    out (ROM_IO),a

    ; send 0xA0 to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$A0
    out (ROM_IO),a

    ; send byte from to ROM address in (de)
    ld a,d
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    out (ROM_IO),a              ; store in ROM

    ; increment addresses and decrement counters
    inc de
    djnz copynext
    dec c
    jp nz, copyouter
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ld a,0
    out (LED_IO),a              ; turn ROM led off
    ret