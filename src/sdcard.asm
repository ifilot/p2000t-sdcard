SECTION code_user

SERIAL          EQU  $60
CLKSTART        EQU  $61
DESELECT        EQU  $62
SELECT          EQU  $63

ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
RAM_BANK        EQU  $6B
RAM_IO          EQU  $6D
LED_IO          EQU  $64

PUBLIC _cmd0
PUBLIC _cmd8
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

;-------------------------------------------------------------------------------
; SD card command bytes
;-------------------------------------------------------------------------------
cmd0str:
defb 0 |0x40,0x00,0x00,0x00,0x00,0x94|0x01

cmd8str:
defb 8 |0x40,0x00,0x00,0x01,0xaa,0x86|0x01

cmd55str:
defb 55|0x40,0x00,0x00,0x00,0x00,0x00|0x01

cmd58str:
defb 58|0x40,0x00,0x00,0x00,0x00,0x00|0x01

acmd41str:
defb 41|0x40,0x40,0x00,0x00,0x00,0x00|0x01

;-------------------------------------------------------------------------------
; CMD0: Reset the SD Memory Card
;-------------------------------------------------------------------------------
_cmd0:
    ld hl,cmd0str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; CMD8: Sends interface condition
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
; CMD55: Next command is application specific command
;-------------------------------------------------------------------------------
_cmd55:
    ld hl,cmd55str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; CMD58: Read OCR register
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
; Return value stored in l (received from sendr1)
;-------------------------------------------------------------------------------
_acmd41:
    ld hl,acmd41str
    call sendr1
    ret

;-------------------------------------------------------------------------------
; Send command and receive R1 combined
;
; Return value stored in l
;-------------------------------------------------------------------------------
sendr1:
    call sendcommand
    call _receive_R1
    ret

;-------------------------------------------------------------------------------
; Send a command to the SD card
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
;-------------------------------------------------------------------------------
_read_block:
    di
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,2                      ; number of outer loops
ornb00:
    ld b,0                      ; 256 iterations for inner loop
nb00:
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld (hl),a
    inc hl
    djnz nb00
    dec c
    jp nz, ornb00
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ei
    ret

;-------------------------------------------------------------------------------
; Copy the first 0x100 bytes from a block to external RAM
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
; void open_command(void)
;-------------------------------------------------------------------------------
_open_command:
    ld a,0xFF
    out (SERIAL),a              ; flush shift register with ones
    out (CLKSTART),a            ; send out
    out (SELECT),a              ; activate SD card
    out (CLKSTART),a            ; send another byte of ones
    ret

;-------------------------------------------------------------------------------
; void close_command(void)
;-------------------------------------------------------------------------------
_close_command:
    ld a,0xFF
    out (SERIAL),a              ; flush shift register with ones
    out (CLKSTART),a            ; send out
    out (DESELECT),a            ; deactivate (release) SD card
    out (CLKSTART),a            ; send another byte of ones
    ret