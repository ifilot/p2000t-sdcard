SECTION code_user

SERIAL          EQU  $60
CLKSTART        EQU  $61

ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
RAM_BANK        EQU  $6B
RAM_IO          EQU  $6D
LED_IO          EQU  $64

PUBLIC _read_block
PUBLIC _fast_sd_to_ram_first_0x100
PUBLIC _fast_sd_to_ram_last_0x100
PUBLIC _fast_sd_to_ram_full

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