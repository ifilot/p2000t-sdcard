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

PUBLIC _launch_cas

INCLUDE "ports.inc"
;-------------------------------------------------------------------------------
; VARIABLES
;-------------------------------------------------------------------------------
RELOCATED_LAUNCHER: EQU $6151      ; start address of relocated launch_cas_code
PRG_SRC_ADDR:       EQU $0000      ; SLOT2 RAM start address of selected program
PRG_SRC_META:       EQU $8000      ; ... and its metadata location
RELOCATION_OFFSET:  EQU launch_cas_code - RELOCATED_LAUNCHER ; relocation offset for launch_cas_code

_launch_cas:
    ld hl, launch_cas_code         ; Source address
    ld de, RELOCATED_LAUNCHER      ; Destination address
    ld bc, launch_cas_code_end - launch_cas_code ; Number of bytes to copy
    ldir                           ; Copy BC bytes from (HL) to (DE)
    jp RELOCATED_LAUNCHER

;-------------------------------------------------------------------------------
; Load cas program data from external ram and call boot address
;-------------------------------------------------------------------------------
launch_cas_code:
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
    pop hl              ; pop z88dk_caller return address
    pop hl              ; boot address
    jp (hl)             ; call boot address

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

launch_cas_code_end:
    ASSERT (launch_cas_code_end - launch_cas_code) <= ($6200 - RELOCATED_LAUNCHER), "Error: Relocated launch_cas_code code too large!"
