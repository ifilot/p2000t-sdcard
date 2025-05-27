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

PUBLIC _hexcode_to_uint16t
PUBLIC _load_only

;-------------------------------------------------------------------------------
; uint16_t hexcode_to_uint16t(uint8_t *addr) __z88dk_callee;
;-------------------------------------------------------------------------------
_hexcode_to_uint16t:
    pop iy                  ; get return address
    pop hl                  ; get pointer to memory address
    call hex_to_uint8_t     ; returns num in a and increments hl by 2
    ld d,a                  ; store upper byte
    call hex_to_uint8_t     ; returns num in a and increments hl by 2
    ld e,a                  ; store lower byte
    ex de,hl                ; put ex into hl
    push iy                 ; put return address back onto the stack
    ret                     ; return result in HL

;-------------------------------------------------------------------------------
; convert two hex chars to 8-bit unsigned integer
;
; input    - hl : memory position
;
; output   - a : 8 bit integer
;
; garbles: - c
;          - hl' = hl + 2
;-------------------------------------------------------------------------------
hex_to_uint8_t:
    ld a,(hl)
    call hex_to_int         ; convert a
    or a                    ; clear carry
    rla                     ; shift left four times
    rla
    rla
    rla
    and 0xF0                ; zero lower bits
    ld c,a                  ; store upper nibble
    inc hl                  ; increment pointer
    ld a,(hl)               ; load next char
    call hex_to_int         ; convert to num and store in a
    or c                    ; place upper nibble into a
    inc hl
    ret                     ; a contains 8-bit unsigned integer

;-------------------------------------------------------------------------------
; convert hexadecimal character stored in 'A' to numeric value
; input:  a - hexadecimal character
; output: a - numeric value
; 
; sets 0x00 for invalid number
;-------------------------------------------------------------------------------
hex_to_int:
    ld a,(hl)               ; load character into a
    cp '0'
    jr c, hex_end
    cp '9'
    jr nc, hex_alpha_check
    sub '0'
    ret
hex_alpha_check:
    cp 'A'
    jr c, hex_invalid
    cp 'F'
    jr nc, hex_invalid
    sub 'A' - 10
hex_end:
    ret
hex_invalid:
    ld de,0
    ret


;-------------------------------------------------------------------------------
; VARIABLES
;-------------------------------------------------------------------------------
CUSTOM_LOADROM:     EQU $6151      ; start address of relocated loadrom code
PRG_SRC_ADDR:       EQU $0000      ; SLOT2 RAM start address of selected program
PRG_SRC_META:       EQU $8000      ; ... and its metadata location
RELOCATION_OFFSET:  EQU loadrom - CUSTOM_LOADROM ; relocation offset for loadrom

_load_only:
    ld hl, loadrom                ; Source address
    ld de, CUSTOM_LOADROM         ; Destination address
    ld bc, loadrom_end - loadrom  ; Number of bytes to copy
    ldir                          ; Copy BC bytes from (HL) to (DE)
    jp CUSTOM_LOADROM

;-------------------------------------------------------------------------------
; Load data from external rom and return to Basic
;-------------------------------------------------------------------------------
loadrom:
    ; clear screen
    ld hl,$5000         ; start of screen memory
    ld a,24             ; 24 lines to clear
    call $0035          ; call Monitor's clear_lines routine

    ld a,1
    out (LED_IO), a     ; set read LED
    out (RAM_BANK), a   ; load programs from second RAM bank
    
    ; load deploy address into de
    ld hl,PRG_SRC_META  
    call read_ram_byte - RELOCATION_OFFSET
    ld e,a
    ld hl,PRG_SRC_META+1
    call read_ram_byte - RELOCATION_OFFSET
    ld d,a
    ; load file size into bc
    ld hl,PRG_SRC_META+2 
    call read_ram_byte - RELOCATION_OFFSET
    ld c,a
    ld hl,PRG_SRC_META+3
    call read_ram_byte - RELOCATION_OFFSET
    ld b,a
    ; load start of SLOT2 ram into hl
    ld hl,PRG_SRC_ADDR  
    ; copy data from SLOT2 RAM to P2000T RAM
    call copy_program - RELOCATION_OFFSET

    ld a,0
    out (LED_IO), a     ; turn read LED off
    xor a               ; set flags z, nc
    jp $1FC6            ; back to Basic

; -------------------------------------------------------------------------------
; read a byte from SLOT2 RAM into a register
;
; hl - address
; -------------------------------------------------------------------------------
read_ram_byte:
    ld a,h
    out (ADDR_HIGH),a   ; store upper bytes in register
    ld a,l
    out (ADDR_LOW),a    ; store lower bytes in register
    in a,(RAM_IO)       ; load byte
    ret

; -------------------------------------------------------------------------------
; Copy data from SLOT2 RAM to P2000T RAM and set BASIC pointers
;
; hl - source in SLOT2 RAM
; de - destination in P2000T RAM
; bc - number of bytes to copy
; -------------------------------------------------------------------------------
copy_program:
    push bc
    push de
cp_loop:
    call read_ram_byte - RELOCATION_OFFSET ; load from SLOT2 ram into a register
    ld (de),a
    inc de
    inc hl
    dec bc
    ld a,b
    or c
    jr nz,cp_loop
set_deploy_addr:
    pop hl              ; read destination addres into $625C
    ld ($625C), hl
set_basic_pointers:
    pop bc
    add hl,bc           ; add program length
    ld ($6405),hl       ; set BASIC pointers to variable space
    ld ($6407),hl
    ld ($6409),hl
    ret

loadrom_end:
    ASSERT (loadrom_end - loadrom) <= ($6200 - CUSTOM_LOADROM), "Error: Relocated loadrom code too large!"
