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

PUBLIC _crc16_intram
PUBLIC _crc16_romchip

;-------------------------------------------------------------------------------
; Generate a 16 bit checksum of internal RAM
;
; input:  bc - number of bytes
;         hl - start of memory address
; output: hl - crc16 checksum
; uses: a, bc, de, hl
;
; source: https://mdfs.net/Info/Comp/Comms/CRC16.htm
;-------------------------------------------------------------------------------
_crc16_intram:
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; number of bytes
    push de                     ; put return address back on stack
    ld de,$0000                 ; set de to $0000
nextbyte:
    push bc                     ; push counter onto stack
    ld a, (hl)                  ; read byte from ram chip
    call calc_byte
    pop bc                      ; get counter back from stack
    dec bc                      ; decrement counter
    ld a,b                      ; check if counter is zero
    or c
    jp nz,nextbyte              ; if not zero, go to next byte
    ex de,hl                    ; swap de and hl such that hl contains crc
    ret                         ; return value is stored in hl

calc_byte:
    xor d                       ; xor byte into CRC top byte
    ld b,8                      ; prepare to rotate 8 bits
rot:
    sla e                       ; rotate crc
    adc a,a
    jp nc,clr                   ; bit 15 was zero
    ld d,a                      ; put crc high byte back into d
    ld a,e                      ; crc = crc ^ $1021 (xmodem polynomic)
    xor $21
    ld e,a
    ld a,d                      ; get crc top byte back into a
    xor $10
clr:
    dec b                       ; decrement bit counter
    jp nz,rot                 ; loop for 8 bits
    ld d,a                      ; put crc top byte back into d
    inc hl                      ; step to next byte
    ret

;-------------------------------------------------------------------------------
; Generate a 16 bit checksum
;
; input:  bc - number of bytes
;         hl - start of memory address
; output: hl - crc16 checksum
; uses: a, bc, de, hl
;
; source: https://mdfs.net/Info/Comp/Comms/CRC16.htm
;-------------------------------------------------------------------------------
_crc16_romchip:
    ld a,0x01
    out (LED_IO),a              ; turn ROM led on
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; number of bytes
    push de                     ; put return address back on stack
    ld de,$0000                 ; set de to $0000
nextbyte_rom:
    push bc                     ; push counter onto stack
    ld a,h                      ; set upper address memory
    out (ADDR_HIGH),a
    ld a,l                      ; set lower address memory
    out (ADDR_LOW),a
    in a, (ROM_IO)              ; read byte from ram chip
    call calc_byte
    pop bc                      ; get counter back from stack
    dec bc                      ; decrement counter
    ld a,b                      ; check if counter is zero
    or c
    jp nz,nextbyte_rom          ; if not zero, go to next byte
    ex de,hl                    ; swap de and hl such that hl contains crc
    ld a,0x00
    out (LED_IO),a              ; turn ROM led off
    ret                         ; return value is stored in hl