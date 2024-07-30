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

PUBLIC _read_block
PUBLIC _write_block
PUBLIC _fast_sd_to_ram_first_0x100
PUBLIC _fast_sd_to_ram_last_0x100
PUBLIC _fast_sd_to_ram_full
PUBLIC _fast_sd_to_intram_full

PUBLIC _read_sector
PUBLIC _write_sector

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
; CMD17: Read block
;
; uint8_t cmd17(uint32_t addr);
;
; garbles: a,b,de,hl,iy
; result of R1 is stored in l
;-------------------------------------------------------------------------------
_cmd17:
    ld a,17|0x40
    out (SERIAL),a
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
    out (SERIAL),a
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
; Input: None
; Garbles: a,c,hl
; Output: None
;-------------------------------------------------------------------------------
_read_block:
    ld a,0x02
    out (LED_IO),a              ; turn write led on
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld hl,SDCACHE0              ; set external RAM address
    ld c,2                      ; number of outer loops
blockouter:
    ld b,0                      ; 256 iterations for inner loop
blocknext:
    ld a,h
    out (ADDR_HIGH),a           ; set high byte
    ld a,l
    out (ADDR_LOW),a            ; set low byte
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    out (RAM_IO),a              ; write to RAM
    inc hl                      ; increment RAM pointer
    djnz blocknext
    dec c
    jp nz, blockouter
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a            ; which are ignored
    ld a,0x00
    out (LED_IO),a              ; turn write led off
    ret

;-------------------------------------------------------------------------------
; Write block to SD card
;
; void write_block(void);
;
; Input: None
; Garbles: a,bc,hl
;-------------------------------------------------------------------------------
_write_block:
    ld hl,SDCACHE0              ; set external RAM address
    ld a,0xFE                   ; send start block token
    out (SERIAL),a              ; store in shift register
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    ld c,2                      ; number of outer loops
wblockouter:
    ld b,0                      ; 256 iterations for inner loop
wblocknext:
    ld a,h
    out (ADDR_HIGH),a           ; set high byte
    ld a,l
    out (ADDR_LOW),a            ; set low byte
    in a,(RAM_IO)               ; read from RAM
    out (SERIAL),a              ; store in shift register
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    inc hl                      ; increment RAM pointer
    djnz wblocknext
    dec c
    jp nz, wblockouter
    ld a,0xFF                   ; flush serial register
    out (SERIAL),a              ; store in shift register
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a            ; which are ignored
    ret

;-------------------------------------------------------------------------------
; Read a sector from the SD card
;
; INPUT: DEHL - 32 bit SD card sector address
; OUTPUT: L - read token (0xFE is success, failure otherwise)
;-------------------------------------------------------------------------------
_read_sector:
    di
    call _open_command
    call _cmd17                 ; return SD card status
    ld a,l                      ; load response into a
    cp 0xFE                     ; check if equal to success token
    jp nz,readsectorexit        ; if not, exit with an error
    call _read_block            ; if success token, read block
    jmp readsectorsuccess
readsectorfail:
    jmp readsectorexit
readsectorsuccess:
    ld l,0xFE
readsectorexit:
    call _close_command
    ei
    ret

;-------------------------------------------------------------------------------
; Write a sector to the SD card
;
; INPUT: DEHL - 32 bit SD card sector address
;-------------------------------------------------------------------------------
_write_sector:
    di
    call _open_command
    call _cmd24                 ; call CMD24, token is stored in l
    ld a,l
    cp 0x00                     ; check if return value is 0x00
    jp nz,exitfail              ; if not, exit function
    call _write_block           ; call write block
    ; data has been written, now wait until a response from the SD-card is
    ; being received
    ld bc,TIMEOUT_WRITE
writewaitresp:
    dec bc
    ld a,b
    or c
    jp z,responsetimeout        ; send response wait timeout
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a,(SERIAL)               ; read from register
    cp 0xFF                     ; is token equal to ones?
    jp z,writewaitresp          ; try again until non-0xFF is being read
    ; response from SD-card is received, now check if the data has been accepted
    ; by comparing response (after ANDING with 0x0F) to 0x05
    and 0x0F
    cp 0x05
    jp nz,exitl2a               ; exit function with value in a put into l
    ; successfull response received, not wait until write cycle is finished
    ld bc,TIMEOUT_WRITE
writewaitdone:
    dec bc
    ld a,b
    or c
    jp z,exitl2a                ; exit function with value in a put into l
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a,(SERIAL)               ; read from register
    cp 0x00                     ; check for busy token
    jp z,writewaitdone          ; if busy token received, try again
    ld l,0x05                   ; if not, write is complete, exit function
    jmp exitwrite               ; exit with success write as response
responsetimeout:
    ld l,0xFF
    jmp exitwrite
exitl2a:
    ld l,a
exitfail:
    or 0x80                     ; set MSB to 1 and return the error code
    ld l,a
exitwrite:
    call _close_command         ; return with token value from write block in l
    ei
    ret

;-------------------------------------------------------------------------------
; Copy the first 0x100 bytes from a block to external RAM
;
; void fast_sd_to_ram_first_0x100(uint16_t ram_addr);
;-------------------------------------------------------------------------------
_fast_sd_to_ram_first_0x100:
    di
    ld a,0x02
    out (LED_IO),a              ; turn WRITE led on
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld b,0                      ; perform 0x100 iterations with copy
nb10:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld d,a                      ; store temporarily in d
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,d                      ; put value back in a
    out (RAM_IO),a              ; store in external memory
    inc hl                      ; increment RAM pointer
    djnz nb10
    ld b,0                      ; perform another 0x100 iterations without copy
nb11:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    djnz nb11
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ld a,0
    out (LED_IO),a              ; turn write LED off
    ei
    ret

;-------------------------------------------------------------------------------
; Copy the last 0x100 bytes from a block to external RAM
;
; void fast_sd_to_ram_last_0x100(uint16_t ram_addr);
;-------------------------------------------------------------------------------
_fast_sd_to_ram_last_0x100:
    di
    ld a,0x02
    out (LED_IO),a              ; turn WRITE led on
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld b,0                      ; perform 0x100 iterations without copy
nb20:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    djnz nb20
    ld b,0                      ; perform another 0x100 iterations but with copy
nb21:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld d,a                      ; store temporarily in d
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,d                      ; put value back in a
    out (RAM_IO),a              ; store in external memory
    inc hl                      ; increment RAM pointer
    djnz nb21
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ld a,0
    out (LED_IO),a              ; turn write LED off
    ei
    ret

;-------------------------------------------------------------------------------
; Copy the full 0x200 bytes from a block to external RAM
;
; void fast_sd_to_ram_full(uint16_t ram_addr);
;-------------------------------------------------------------------------------
_fast_sd_to_ram_full:
    di
    ld a,0x02
    out (LED_IO),a              ; turn WRITE led on
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,2                      ; number of outer loops
ornb30:
    ld b,0                      ; 256 iterations for inner loop
nb30:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld d,a                      ; store temporarily in d
    ld a,h
    out (ADDR_HIGH), a          ; store upper byte address
    ld a,l
    out (ADDR_LOW), a           ; store lower byte address
    ld a,d                      ; put value back in a
    out (RAM_IO),a              ; store in external memory
    inc hl                      ; increment RAM pointer
    djnz nb30
    dec c
    jp nz, ornb30
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ld a,0
    out (LED_IO),a              ; turn write LED off
    ei
    ret

;-------------------------------------------------------------------------------
; Copy the full 0x200 bytes from a block to internal RAM
;
; void fast_sd_to_intram_full(uint16_t ram_addr);
;-------------------------------------------------------------------------------
_fast_sd_to_intram_full:
    di
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
    ei
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