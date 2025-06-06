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

SDCACHE0        EQU  $0000
SDCACHE1        EQU  $0200
SDCACHE2        EQU  $0400
TIMEOUT_READ    EQU  4000
TIMEOUT_WRITE   EQU  10000

PUBLIC _sdpulse
PUBLIC _cmd0
PUBLIC _cmd8
PUBLIC _cmd17
PUBLIC _cmd24
PUBLIC _cmd55
PUBLIC _cmd58
PUBLIC _acmd41
PUBLIC _receive_R1

PUBLIC _open_command
PUBLIC _close_command

PUBLIC _fast_sd_to_intram_full
PUBLIC _read_sector_to

PUBLIC _sdout_set
PUBLIC _sdout_reset
PUBLIC _sdcs_set
PUBLIC _sdcs_reset

;-------------------------------------------------------------------------------
; SD card command bytes
;-------------------------------------------------------------------------------
cmd0str:
defb 0 |0x40,0x00,0x00,0x00,0x00,0x94|0x01

cmd8str:
defb 8 |0x40,0x00,0x00,0x01,0xaa,0x86|0x01
;                      VHS  CHK  CRC

cmd55str:
defb 55|0x40,0x00,0x00,0x00,0x00,0x00|0x01

cmd58str:
defb 58|0x40,0x00,0x00,0x00,0x00,0x00|0x01

acmd41str:
defb 41|0x40,0x40,0x00,0x00,0x00,0x00|0x01

;-------------------------------------------------------------------------------
; Send pulses to the SD-card to trigger it into a reset state
;-------------------------------------------------------------------------------
_sdpulse:
    ld b,12
    ld a,0xFF
    out (SERIAL),a
nextpulse:
    out (CLKSTART),a
    djnz nextpulse
    ret

;-------------------------------------------------------------------------------
; CMD0: Reset the SD Memory Card
;
; garbles: a,b,hl
; result of R1 is stored in l, but ignored
;-------------------------------------------------------------------------------
_cmd0:
    ld hl,cmd0str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; CMD8: Sends interface condition
;
; garbles: a,b,hl
;-------------------------------------------------------------------------------
_cmd8:
    ex de,hl
    ld hl,cmd8str               ; load command list
    call sendcommand            ; garbles a,b,hl
    call receiveR7              ; garbles a,b,de
    ret

;-------------------------------------------------------------------------------
; Send command and address to the SD card
;
; a contains the command byte
; dehl contains the address to send
;-------------------------------------------------------------------------------
sd_send_command_and_address:
    out (SERIAL),a              ; a contains the command byte
    out (CLKSTART),a            ; send out

    ld a,d
    out (SERIAL),a              ; byte 0
    out (CLKSTART),a            ; send out

    ld a,e
    out (SERIAL),a              ; byte 1
    out (CLKSTART),a            ; send out

    ld a,h
    out (SERIAL),a              ; byte 2
    out (CLKSTART),a            ; send out

    ld a,l
    out (SERIAL),a              ; byte 3
    out (CLKSTART),a            ; send out

    ld a,0x00|0x01
    out (SERIAL),a
    out (CLKSTART),a            ; send out
    ret

;-------------------------------------------------------------------------------
; CMD17: Read block
;
; uint8_t cmd17(uint32_t addr);
;
; garbles: a,b,de,hl,iy
; result of R1 is stored in l
;-------------------------------------------------------------------------------
_cmd17:
    ld a,17|0x40
    call sd_send_command_and_address
    call _receive_R1
    ld a,0xFF                   ; flush with ones
    out (SERIAL),a
    ld bc,TIMEOUT_READ          ; set timeout timer
cmd17next:
    dec bc
    ld a,b
    or c
    jp z,cmd17timeout
    out (CLKSTART),a            ; send out
    in a,(SERIAL)
    cp 0xFE                     ; wait for 0xFE to be received
    jr nz,cmd17next
    ld l,a
    ret
cmd17timeout:
    ld l,0xFF
    ret

;-------------------------------------------------------------------------------
; CMD24: Write block
;
; void cmd24(uint32_t addr);
;
; garbles: a,b,de,hl,iy
; result of R1 is stored in l
;-------------------------------------------------------------------------------
_cmd24:
    ld a,24|0x40
    call sd_send_command_and_address
    call _receive_R1
    ret

;-------------------------------------------------------------------------------
; CMD55: Next command is application specific command
;
; garbles: a,b,hl
; result of R1 is stored in l, but ignored
;-------------------------------------------------------------------------------
_cmd55:
    ld hl,cmd55str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; CMD58: Read OCR register
;
; input: hl - pointer to response array
; garbles: a,b,hl
; result of R1 is stored in l, but ignored
;-------------------------------------------------------------------------------
_cmd58:
    ex de,hl                    ; store pointer in de
    ld hl,cmd58str              ; load command list
    call sendcommand            ; garbles a,b,hl
    call receiveR3              ; garbles a,b,de; de is pointer address for R3
    ret

;-------------------------------------------------------------------------------
; ACMD41: Send host capacity support information
;
; garbles: a,b,hl
; result of R1 is stored in l
;-------------------------------------------------------------------------------
_acmd41:
    ld hl,acmd41str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; Send command and receive R1 combined
;
; garbles: a,b,hl
; result of R1 is stored in l
;-------------------------------------------------------------------------------
sendr1:
    call sendcommand
    call _receive_R1
    ret

;-------------------------------------------------------------------------------
; Send a command to the SD card
;
; garbles: a,b,hl
;-------------------------------------------------------------------------------
sendcommand:
    ld b,6                      ; number of bytes to send
sendnextbyte:
    ld a,(hl)
    out (SERIAL),a              ; flush shift register with ones
    out (CLKSTART),a            ; send out
    inc hl
    djnz sendnextbyte
    ret

;-------------------------------------------------------------------------------
; Obtain R1 response from SD card
;
; uint8_t receive_R1(void);
;
; Garbles: A
; Return: R1 value in l
;-------------------------------------------------------------------------------
_receive_R1:
    ld a,0xFF
    out (SERIAL),a
    out (CLKSTART),a            ; send out
    out (SERIAL),a
    out (CLKSTART),a            ; send out
    in a,(SERIAL)               ; retrieve byte
    ld l,a
    ret

;-------------------------------------------------------------------------------
; Obtain R3 or R7 response from SD card
;
; Input: de - memory pointer to store response in
;-------------------------------------------------------------------------------
receiveR3:
receiveR7:
    ld a,0xFF
    out (SERIAL),a
    out (CLKSTART),a            ; send out
    ld b,5
recvbyte:
    out (CLKSTART),a            ; send out
    in a,(SERIAL)               ; retrieve byte
    ld (de),a                   ; store in memory
    inc de                      ; increment memory position
    djnz recvbyte
    ret

;-------------------------------------------------------------------------------
; Read block from SD card
;
; void read_block(void);
;
; Input: DE - external RAM address
; Garbles: a,b,c
; Output: None
;-------------------------------------------------------------------------------
read_block:
    ld a,0x02
    out (LED_IO),a              ; turn write led on
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,2                      ; number of outer loops
blockouter:
    ld b,0                      ; 256 iterations for inner loop
blocknext:
    ld a,d
    out (ADDR_HIGH),a           ; set high byte
    ld a,e
    out (ADDR_LOW),a            ; set low byte
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    out (RAM_IO),a              ; write to RAM
    inc de                      ; increment RAM pointer
    djnz blocknext
    dec c
    jp nz, blockouter
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a            ; which are ignored
    ld a,0x00
    out (LED_IO),a              ; turn write led off
    ret

;-------------------------------------------------------------------------------
; Read a sector from the SD card
;
; INPUT: stack contains the following:
;        - return address
;        - low word of sector address
;        - high word of sector address
;        - target address in external RAM
; OUTPUT: L - read token (0xFE is success, failure otherwise)
;-------------------------------------------------------------------------------
_read_sector_to:
    pop iy                      ; return address
    pop hl                      ; retrieve sector address (low)
    pop de                      ; retrieve sector address (high) 
    push iy                     ; put return address back on stack
    call _open_command
    call _cmd17                 ; return SD card status
    pop iy                      ; return address
    pop de                      ; retrieve target address external RAM
    push iy                     ; put return address back on stack
    ld a,l                      ; load response into a
    cp 0xFE                     ; check if equal to success token
    jp nz,readsectorexit        ; if not, exit with an error
    call read_block             ; if success token, read block
    jmp readsectorsuccess
readsectorfail:
    jmp readsectorexit
readsectorsuccess:
    ld l,0xFE
readsectorexit:
    call _close_command
    ret

;-------------------------------------------------------------------------------
; Copy the full 0x200 bytes from a block to internal RAM
;
; void fast_sd_to_intram_full(uint16_t ram_addr);
;-------------------------------------------------------------------------------
_fast_sd_to_intram_full:
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,2                      ; number of outer loops
fstifouter:
    ld b,0                      ; 256 iterations for inner loop
fstifinner:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld (hl),a
    inc hl                      ; increment RAM pointer
    djnz fstifinner
    dec c
    jp nz, fstifouter
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ret

;-------------------------------------------------------------------------------
; void open_command(void);
;
; See also this post: 
; https://electronics.stackexchange.com/questions/303745/sd-card-initialization-problem-cmd8-wrong-response
;
; Garbles: a
;-------------------------------------------------------------------------------
_open_command:
    ld a,0xFF
    out (SERIAL),a              ; flush shift register with ones
    out (CLKSTART),a            ; send out
    out (SELECT),a              ; activate SD card
    out (CLKSTART),a            ; send another byte of ones
    ret

;-------------------------------------------------------------------------------
; void close_command(void);
;
; Garbles: a
;-------------------------------------------------------------------------------
_close_command:
    ld a,0xFF
    out (SERIAL),a              ; flush shift register with ones
    out (CLKSTART),a            ; send out
    out (DESELECT),a            ; deactivate (release) SD card
    out (CLKSTART),a            ; send another byte of ones
    ret

;-------------------------------------------------------------------------------
; void sdout_set(void);
; pull MISO low over 10k resistor
;-------------------------------------------------------------------------------
_sdout_set:
    in a,(DESELECT)             ; value in a is ignored when setting
    ret

;-------------------------------------------------------------------------------
; void sdout_reset(void);
; pull MISO high over 10k resistor
;-------------------------------------------------------------------------------
_sdout_reset:
    in a,(SELECT)               ; value in a is ignored when setting
    ret

;-------------------------------------------------------------------------------
; void sdcs_set(void);
; pull ~CS high (deactivate SD card)
;-------------------------------------------------------------------------------
_sdcs_set:
    out (DESELECT),a            ; value in a is ignored when setting
    ret

;-------------------------------------------------------------------------------
; void sdcs_reset(void);
; pull ~CS low (activate SD card)
;-------------------------------------------------------------------------------
_sdcs_reset:
    out (SELECT),a              ; value in a is ignored when setting
    ret
