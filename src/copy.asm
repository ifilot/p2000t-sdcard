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
ROM_IO          EQU  $6C
ROMBANK         EQU  $6A
CHECKSUM_BUFFER EQU  $8000
CHECKSUM_STORE  EQU  $90

PUBLIC _calculatecrc16

;-------------------------------------------------------------------------------
; Calculate the CRC16 checksum
;
; For every page on the ROM (4kb), the data is copied to the checksum buffer
; and a CRC16 checksum is being generated. These checksums are stored starting
; at page $9000 and further. In total, four checksums will be written to
; $9000 - $9007. Note that the checksums are stored in little endian format,
; i.e., with the least significant byte (LSB) first.
;-------------------------------------------------------------------------------
_calculatecrc16:
    ld hl,$0000                     ; start external address
    ld b,4
    ld c,$00
nextpage:
    push bc                         ; store page counter
    ld de,CHECKSUM_BUFFER           ; destination address in ram
    ld bc,$1000
calculatecrc16_nextbyte:
    call sst39sfrecv
    ld (de),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jr nz,calculatecrc16_nextbyte
    push hl                         ; store rom counter
    ld bc,$1000
    ld de,$0000
    ld hl,CHECKSUM_BUFFER
    call crc16                      ; checksum resides in DE
    pop hl                          ; restore rom counter
    pop bc                          ; restore page counter
    push hl                         ; store rom counter
    ld h,CHECKSUM_STORE             ; set upper byte memory
    ld a,c                          ; load counter
    ld l,a                          ; set lower address
    ld (hl),e                       ; store lower byte checksum in memory
    inc hl                          ; next address
    ld (hl),d                       ; store upper byte checksum in memory
    inc c                           ; increment storage address by 2
    inc c
    pop hl                          ; restore rom counter
    djnz nextpage                   ; next page
    ret

;-------------------------------------------------------------------------------
; Generate a 16 bit checksum
;
; input:  bc - number of bytes
;         de - starting checksum (typically $0000)
;         hl - start of memory address
; output: de - crc16 checksum
; uses: a, bc, de, hl
;
; source: https://mdfs.net/Info/Comp/Comms/CRC16.htm
;-------------------------------------------------------------------------------
crc16:
bytelp:
    push bc                     ; push counter onto stack
    ld a,(hl)                   ; fetch byte
    xor d                       ; xor byte into CRC top byte
    ld b,8                      ; prepare to rotate 8 bits
rotlp:
    sla e                       ; rotate crc
    adc a,a
    jp nc,clear                 ; bit 15 was zero
    ld d,a                      ; put crc high byte back into d
    ld a,e                      ; crc = crc ^ $1021 (xmodem polynomic)
    xor $21
    ld e,a
    ld a,d                      ; get crc top byte back into a
    xor $10
clear:
    dec b                       ; decrement bit counter
    jp nz,rotlp                 ; loop for 8 bits
    ld d,a                      ; put crc top byte back into d
    inc hl                      ; step to next byte
    pop bc                      ; get counter back from stack
    dec bc                      ; decrement counter
    ld a,b                      ; check if counter is zero
    or c
    jp nz,bytelp               ; if not zero, go to next byte
    ret

;-------------------------------------------------------------------------------
; Receive a byte from internal SST39SF0x0 chip
;         hl - chip address
; output: a - byte at address
;         hl - chip address
;-------------------------------------------------------------------------------
sst39sfrecv:
    ld a,l
    out (ADDR_LOW),a
    ld a,h
    out (ADDR_HIGH),a
    in a,(ROM_IO)
    ret