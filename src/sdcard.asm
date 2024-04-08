SECTION code_user

SERIAL         EQU  $60
CLKSTART       EQU  $61

PUBLIC _read_block

;-------------------------------------------------------------------------------
; Read block from SD card
;-------------------------------------------------------------------------------
_read_block:
    di
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; put return address back on stack
    ld bc,514                   ; number of bytes
nextbyte:
    ld a,$FF
    out (SERIAL),a              ; load ones
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    ld (hl),a
    inc hl
    dec bc
    ld a,c
    or b
    jp nz,nextbyte
    ei
    ret