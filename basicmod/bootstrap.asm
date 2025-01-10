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

;-------------------------------------------------------------------------------
; bootstrap.asm
;
; Upon the first keyboard parse routine of the basic
; rom, executes the code located at $4EC7. This will
; modify a few pointers and loads in the code from
; the I/O port to ADDR EXCODE and launches it from there.
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; HARDWARE CONSIDERATIONS
;
; The SD-card cartridge has a 128kb RAM chip which is accessible as two separate
; 64kb banks. The upper RAM bank is used for loading cassette data.
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; VARIABLES
;-------------------------------------------------------------------------------
EXCODE:         EQU $7000       ; address to put and launch external code from
LED_IO:         EQU $44         ; LED I/O
ROM_IO:         EQU $4C         ; external ROM
RAM_IO:         EQU $4D         ; external RAM
IO_AL:          EQU $48         ; address low
IO_AH:          EQU $49         ; address high
ROM_BANK:       EQU $4A         ; address ROM bank
RAM_BANK:       EQU $4B         ; address RAM bank
NUMBYTES:       EQU $2D00       ; hardcoded length of launcher program
PROGADDR:       EQU $0000       ; location on ROM where program resides
DEPLOYADDR:     EQU $6150       ; storage location of deploy addr
BOOTSTR_HOOK:   EQU $60D3       ; Basic's hook address for our bootstrap

	; The bootstrap code is injected into the end of the standard BASIC 
	; cartridge starting at $4EC7
    org $4EC7

;-------------------------------------------------------------------------------
;
; Load code from I/O port and launch the code
;
; bc - number of bytes
; de - start location on ROM
; hl - destination in RAM
;
;-------------------------------------------------------------------------------
loadcode:
    ld a,0x01
    out (LED_IO), a     ; turn read LED on
    dec a               ; set a = 0
    ld (BOOTSTR_HOOK),a ; remove bootstrap hook
    ld (BOOTSTR_HOOK+1),a
    ld (BOOTSTR_HOOK+2),a
    ld hl,msgbl
    call printmsg
    di
    ld a,0
    out (ROM_BANK),a    ; set to bank 0
    out (RAM_BANK),a    ; set to bank 0
    ld bc,NUMBYTES      ; load number of bytes from internal ram
    ld hl,EXCODE        ; location where to write
    ld de,PROGADDR      ; location where to read
lcnextbyte:
    call read_rom
    ld (hl),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jr nz,lcnextbyte
    out (LED_IO), a     ; turn read led off (a = 0 here)
    call EXCODE         ; call custom firmware code (will return here)
    jp loadrom

msgbl:
    DB $06,$0D,"Booting launcher",$FF

;-------------------------------------------------------------------------------
; Load data from external rom
;-------------------------------------------------------------------------------
loadrom:
    ld a,1
    out (LED_IO), a     ; set read LED
    out (RAM_BANK), a   ; load programs from second RAM bank
    ld hl,msglp
    call printmsg
    ld de,$8000+1
    call read_ram       ; load high byte deploy addr
    ld (DEPLOYADDR+1),a
    ld de,$8000
    call read_ram       ; load low byte deploy addr
    ld (DEPLOYADDR),a
    ld de,$8000+3
    call read_ram       ; load high byte file size
    ld b,a
    ld de,$8000+2
    call read_ram       ; load low byte file size
    ld c,a
    call copydata       ; bc contains number of bytes
    ld a,0
    out (LED_IO), a     ; turn read LED off
    xor a               ; set flags z, nc
    jp $28d4            ; launch basic program
    ;jp $1fc6            ; return to Basic

msglp:
    DB $06,$0D,"Launching program",$FF

;-------------------------------------------------------------------------------
; Copy data from external ram to internal memory
;
; bc - number of bytes
;-------------------------------------------------------------------------------
copydata:
    di
    push bc                 ; store number of bytes
    ld de,$0000             ; start of external ram address
    ld hl,(DEPLOYADDR)      ; start of ram address
cdnextbyte:
    call read_ram           ; load from external ram into a register
    ld (hl),a
    inc de
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,cdnextbyte
    pop bc

    ld hl,(DEPLOYADDR)      ; set deploy address
    ld ($625C), hl

    add hl,bc               ; add program length

    ld ($6405),hl           ; set basic pointers to variable space
    ld ($6407),hl
    ld ($6409),hl
    ei
    ret

;-------------------------------------------------------------------------------
; clear the screen
;-------------------------------------------------------------------------------
clrscrn:
    ld a, $0C               ; char code for clear screen    
    call $104A              ; Basic's print char routine
    ret

;-------------------------------------------------------------------------------
; Print message to screen
; hl - pointer to string
;-------------------------------------------------------------------------------
printmsg:
    call clrscrn
    ld bc,$5000 + $50*10    ; print on line 10
pmprint:
    ld a,(hl)
    cp 255
    ret z
    ld (bc),a
    inc hl
    inc bc
    jp pmprint

;-------------------------------------------------------------------------------
; Reads a byte from the I/O port and put it in reg A.
; The address stored at IOREADADDR is used as the load
; address for the I/O port. Upon loading a byte, this
; address is incremented and stored back into the storage
; location.
;
; input: (IOREADADDR) I/O port address to read code from
;  uses: a,de
;
;-------------------------------------------------------------------------------
read_rom:
    ld a,d
    out (IO_AH),a         ; store upper bytes in register
    ld a,e
    out (IO_AL),a         ; store lower bytes in register
    in a,(ROM_IO)         ; load byte
    ret

;-------------------------------------------------------------------------------
; read a byte to external ram
;
; de - address
;-------------------------------------------------------------------------------
read_ram:
    ld a,d
    out (IO_AH),a         ; store upper bytes in register
    ld a,e
    out (IO_AL),a         ; store lower bytes in register
    in a,(RAM_IO)         ; load byte
    ret