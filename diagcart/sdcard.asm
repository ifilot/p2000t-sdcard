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

PUBLIC _init_sdcard

PUBLIC _sdpulse
PUBLIC _cmd0
PUBLIC _cmd8
PUBLIC _cmd17
PUBLIC _cmd55
PUBLIC _cmd58
PUBLIC _acmd41
PUBLIC _receive_R1

PUBLIC _open_command
PUBLIC _close_command

PUBLIC _read_block
PUBLIC _fast_sd_to_ram_first_0x100
PUBLIC _fast_sd_to_ram_last_0x100
PUBLIC _fast_sd_to_ram_full
PUBLIC _fast_sd_to_intram_full

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
; Initialize the SD card
;
; uint8_t init_sdcard(uint8_t *resp8, uint8_t *resp58);
;
; return 0 on success, return 1 on fail
;-------------------------------------------------------------------------------
_init_sdcard:
    di                          ; disable interrupts
    pop hl                      ; retrieve return address
    exx                         ; swap to shadow registers
    call _sdcs_set
    call _sdout_set
    ld b,12
    ld a,0xFF
    out (SERIAL),a
pulse:
    out (CLKSTART),a
    djnz pulse
    call _open_command
    call _cmd0
    call _cmd8                  ; this command will retrieves resp8 from stack
    ld b,5
decde:                          ; decrement address counter by 5
    dec de
    djnz decde
    ld a,(de)                   ; grab echo byte in a
    cp 0x02                     ; bigger than 1?
    jp nc,carderror
hosttry:
    call _cmd55
    call _acmd41                ; value in l
    ld a,l
    cp 0
    jr nz,hosttry               ; if not zero, try again
    call _cmd58                 ; this command will retrieve resp58 from stack
    ld iyl,0                    ; store return value
exitinit:
    call _close_command
    exx                         ; swap back from shadow registers
    push hl                     ; put return address back onto stack
    ei                          ; enable interrupts
    ld a,iyl                    ; retrieve return value
    ld l,a
    ret
carderror:
    ld iyl,1                    ; store return value
    jp exitinit

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
    pop iy                      ; retrieve return address
    pop de                      ; retrieve ram pointer
    ld hl,cmd8str               ; load command list
    call sendcommand            ; garbles a,b,hl
    call receiveR7              ; garbles a,b,de
    push iy                     ; push return address back onto stack
    ret

;-------------------------------------------------------------------------------
; CMD17: Read block
;
; void cmd17(uint32_t addr);
;
; garbles: a,b,de,hl,iy
; result of R1 is stored in l, but ignored
;-------------------------------------------------------------------------------
_cmd17:
    pop iy                      ; retrieve return address
    pop hl                      ; retrieve upper bytes of 32 bit address
    pop de                      ; retrieve lower bytes of 32 bit address
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
cmd17next:
    out (CLKSTART),a            ; send out
    in a,(SERIAL)
    cp 0xFE
    jr nz,cmd17next
    push iy                     ; put return address back on stack
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
; garbles: a,b,hl
; result of R1 is stored in l, but ignored
;-------------------------------------------------------------------------------
_cmd58:
    pop iy                      ; retrieve return address
    pop de                      ; retrieve ram pointer
    ld hl,cmd58str              ; load command list
    call sendcommand            ; garbles a,b,hl
    call receiveR3              ; garbles a,b,de
    push iy                     ; push return address back onto stack
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
; Result is stored in l
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
;-------------------------------------------------------------------------------
_read_block:
    ld a,0x02
    out (LED_IO),a              ; turn write led on
    di
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
    ei
    ld a,0x00
    out (LED_IO),a              ; turn write led off
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