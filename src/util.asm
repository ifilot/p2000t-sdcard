SECTION code_user

PUBLIC _replace_bytes

;-------------------------------------------------------------------------------
; void replace_bytes(uint8_t* str, uint8_t org, uint8_t rep, uint16_t nrbytes) __z88dk_callee;
;-------------------------------------------------------------------------------
_replace_bytes:
    pop iy                  ; retrieve return address
    pop hl                  ; ramptr
    pop de                  ; e = org, d = rep
    pop bc                  ; number of bytes
    push iy                 ; put return address back on stack
nextbyte:
    ld a,(hl)
    cp e
    jr nz,skip
    ld a,d
    ld (hl),a
skip:
    inc hl                  ; next character
    dec bc                  ; decrement counter
    ld a,b
    or c
    jr nz,nextbyte
    ret
