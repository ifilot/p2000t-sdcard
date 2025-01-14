;-------------------------------------------------------------------------------
;                                                                       
;   Authors: Ivo Filot <ivo@ivofilot.nl>     
;            Dion Olsthoorn <@dionoid>                          
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
; Upon the first keyboard interrupt routine of the BASIC rom, executes the 
; code located at $4EC7. This will load in the code bytes from the I/O port to 
; address LNCHR_DST_ADDR and launches it from there.
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
LAUNCHER_SRC:   EQU $0000       ; Launcher's source address on SLOT2 ROM
LAUNCHER_ADDR:  EQU $6549       ; Launcher's target address in P2000T RAM
                                ; note: this is BASIC's prog start addres + 2
LAUNCHER_SIZE:  EQU $3A37       ; hardcoded max length of Launcher app
                                ; note: Launcher loaded from $6549 to $9F7F
                                ; leaving 128 ($80) bytes for stack
PRG_SRC_ADDR:   EQU $0000       ; SLOT2 RAM start address of selected program
PRG_SRC_META:   EQU $8000       ; ... and its metadata
LED_IO:         EQU $44         ; LED I/O
ROM_IO:         EQU $4C         ; SLOT2 ROM
RAM_IO:         EQU $4D         ; SLOT2 RAM
IO_AL:          EQU $48         ; address low
IO_AH:          EQU $49         ; address high
ROM_BANK:       EQU $4A         ; address ROM bank
RAM_BANK:       EQU $4B         ; address RAM bank
BOOTSTR_HOOK:   EQU $60D3       ; BASIC's hook address for our bootstrap

;-------------------------------------------------------------------------------
; MARCO DEFINITIONS
;-------------------------------------------------------------------------------
led_on: macro
    ld a,1
    out (LED_IO), a
    endm

led_off: macro
    xor a
    out (LED_IO), a
    endm

print: macro string
    ld de,string
    call printmsg
    call $0AF2          ; Monitor routine which waits 500ms
    endm

relocate_stack: macro
    ld sp,$A000         ; set stack to top of RAM +1 (for minimal 16K RAM)
    ld a,($605C)        ; $605C contains RAM size (1 = 16K, 2 = 32K, 3 = 48K)
    cp 1
    jr z, .finish
    ld sp,$E000
.finish: endm

;-------------------------------------------------------------------------------
; This bootstrap code is injected into the end of the standard BASIC ROM
; starting at $4EC7
;-------------------------------------------------------------------------------
    org $4EC7
    
start:
    relocate_stack
    call remove_hook
    led_on
    print msg_booting
    call copy_launcher
    led_off
    call LAUNCHER_ADDR  ; start launcher app (returns after program selection)
    led_on
    print msg_loading
    call load_program
    led_off
    call clrscrn
    call run_program
    jp start            ; restart bootstrap (in case program returns)

;-------------------------------------------------------------------------------
; SUBROUTINES
;-------------------------------------------------------------------------------
remove_hook:
    xor a               ; set a = 0
    ld (BOOTSTR_HOOK),a ; clear bootstrap hook
    ld (BOOTSTR_HOOK+1),a
    ld (BOOTSTR_HOOK+2),a
    ret

;-------------------------------------------------------------------------------
; Copies launcher code from SLOT2 ROM to P2000T RAM
; 
; hl - source in SLOT2 ROM
; de - destination in P2000T RAM
; bc - number of bytes
;-------------------------------------------------------------------------------
copy_launcher:
    xor a               ; set a = 0
    out (ROM_BANK),a    ; set SLOT2 ROM to bank 0
    out (RAM_BANK),a    ; set SLOT2 RAM to bank 0
    ld bc,LAUNCHER_SIZE ; number of bytes to load from SLOT2's internal RAM
    ld hl,LAUNCHER_SRC  ; location on SLOT2 ROM where to read (source)
    ld de,LAUNCHER_ADDR ; location on P2000T RAM where to write (dest)
cl_loop:
    call read_rom_byte  ; load byte from SLOT2 ROM
    ld (de),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jr nz,cl_loop
    ret

load_program:
    ld a,1
    out (RAM_BANK), a   ; load programs from second RAM bank
put_transfer_addr_into_de:
    ld hl,PRG_SRC_META
    call read_ram_byte  ; load low byte transfer addr
    ld e,a
    ld hl,PRG_SRC_META+1
    call read_ram_byte  ; load high byte transfer addr
    ld d,a              ; de now contains transfer address
put_file_size_into_bc:
    ld hl,PRG_SRC_META+2
    call read_ram_byte  ; load low byte file size
    ld c,a
    ld hl,PRG_SRC_META+3
    call read_ram_byte  ; load high byte file size
    ld b,a              ; bc now contains number of bytes
put_source_addr_into_hl:
    ld hl,PRG_SRC_ADDR  ; hl now contains start of SLOT2 ram address
    jp copy_program

run_program:
put_start_addr_into_de:
    ld hl,PRG_SRC_META+4
    call read_ram_byte  ; load low byte start addr
    ld e,a
    ld hl,PRG_SRC_META+5
    call read_ram_byte  ; load high byte start addr
    ld d,a              ; de now contains start address
call_start_address:
    push de
    pop hl
    xor a               ; set flags z, nc (needed for BASIC run command)
    jp (hl)             ; $28D4 = run BASIC program ; $1fc6 = return to BASIC

;-------------------------------------------------------------------------------
; Copy data from SLOT2 RAM to P2000T RAM and set BASIC pointers
;
; hl - source in SLOT2 RAM
; de - destination in P2000T RAM
; bc - number of bytes
;-------------------------------------------------------------------------------
copy_program:
    push bc
    push de
cp_loop:
    call read_ram_byte  ; load from SLOT2 ram into a register
    ld (de),a
    inc de
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,cp_loop
set_deploy_addr:
    pop hl              ; read destination addres back in hl
    ld ($625C), hl
set_basic_pointers:
    pop bc
    add hl,bc           ; add program length
    ld ($6405),hl       ; set BASIC pointers to variable space
    ld ($6407),hl
    ld ($6409),hl
    ret

;-------------------------------------------------------------------------------
; clear the screen
;-------------------------------------------------------------------------------
clrscrn:
    ld hl,$5000         ; start of screen memory
    ld a,24             ; 24 lines to clear
    jp $0035            ; call Monitor's clear_lines routine

;-------------------------------------------------------------------------------
; Print message to screen
; de - pointer to string data
;-------------------------------------------------------------------------------
printmsg:
    call clrscrn
    ld bc,$5000 + $50*10 ; print on screen line 10
pmprint:
    ld a,(de)
    and a
    ret z
    ld (bc),a
    inc de
    inc bc
    jp pmprint

;-------------------------------------------------------------------------------
; read a byte from SLOT2 ROM into a register
;
; hl - address
;-------------------------------------------------------------------------------
read_rom_byte:
    ld a,h
    out (IO_AH),a       ; store upper bytes in register
    ld a,l
    out (IO_AL),a       ; store lower bytes in register
    in a,(ROM_IO)       ; load byte
    ret

;-------------------------------------------------------------------------------
; read a byte from SLOT2 RAM into a register
;
; hl - address
;-------------------------------------------------------------------------------
read_ram_byte:
    ld a,h
    out (IO_AH),a       ; store upper bytes in register
    ld a,l
    out (IO_AL),a       ; store lower bytes in register
    in a,(RAM_IO)       ; load byte
    ret

; Messages
msg_booting:
    DB $06,$0D,"Booting launcher",0
msg_loading:
    DB $06,$0D,"Loading program",0