SECTION code_user

; SD-card ports
SERIAL          EQU  $60
CLKSTART        EQU  $61

; ROM ports
ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
ROM_BANK        EQU  $6A
ROM_IO          EQU  $6C
LED_IO          EQU  $64

PUBLIC _crc16_romchip
PUBLIC _copy_to_rom
PUBLIC _fast_sd_to_rom_full

;-------------------------------------------------------------------------------
; Generate a 16 bit checksum
;
; input:  bc - number of bytes
;         hl - start of memory address
; output: de - crc16 checksum
; uses: a, bc, de, hl
;
; source: https://mdfs.net/Info/Comp/Comms/CRC16.htm
;-------------------------------------------------------------------------------
_crc16_romchip:
    ld a,1
    out (LED_IO),a              ; turn ROM led on
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; number of bytes
    push de                     ; put return address back on stack
    ld de,$0000                 ; set de to $0000
nextbyte:
    push bc                     ; push counter onto stack
    ld a,h                      ; set upper address memory
    out (ADDR_HIGH),a
    ld a,l                      ; set lower address memory
    out (ADDR_LOW),a
    in a, (ROM_IO)              ; read byte from ram chip
    xor d                       ; xor byte into CRC top byte
    ld b,8                      ; prepare to rotate 8 bits
rot:
    sla e                       ; rotate crc
    adc a,a
    jp nc,clr                   ; bit 15 was zero
    ld d,a                      ; put crc high byte back into d
    ld a,e                      ; crc = crc ^ $1021 (xmodem polynomic)
    xor $21
    ld e,a
    ld a,d                      ; get crc top byte back into a
    xor $10
clr:
    dec b                       ; decrement bit counter
    jp nz,rot                 ; loop for 8 bits
    ld d,a                      ; put crc top byte back into d
    inc hl                      ; step to next byte
    pop bc                      ; get counter back from stack
    dec bc                      ; decrement counter
    ld a,b                      ; check if counter is zero
    or c
    jp nz,nextbyte              ; if not zero, go to next byte
    ex de,hl
    ld a,0
    out (LED_IO),a              ; turn ROM led off
    ret                         ; return value is stored in hl

;-------------------------------------------------------------------------------
; Copy bytes to external ROM chip
;
; void copy_to_ram(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;
;
; input:  hl - source address
;         de - destination address
;         bc - number of bytes
; uses: all
;-------------------------------------------------------------------------------
_copy_to_rom:
    di
    ld a,1
    out (LED_IO),a              ; turn ROM led on
    pop iy                      ; return address
    pop hl                      ; src
    pop de                      ; dest
    pop bc                      ; number of bytes
    push iy                     ; put return address back on stack
next:
    ; send 0xAA to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$AA
    out (ROM_IO),a
    
    ; send 0x55 to 0x2AAAA
    ld a,$2A
    out (ADDR_HIGH),a
    ld a,$AA
    out (ADDR_LOW),a
    ld a,$55
    out (ROM_IO),a

    ; send 0xA0 to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$A0
    out (ROM_IO),a

    ; send byte from (hl) to ROM address in (de)
    ld a,d
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    ld a,(hl)
    out (ROM_IO),a

    ; increment addresses and decrement counters
    inc de
    inc hl
    dec bc
    ld a,c
    or b
    jp nz, next
    ld a,0
    out (LED_IO),a              ; turn ROM led off
    ei
    ret

;-------------------------------------------------------------------------------
; Copy bytes to external ROM chip
;
; void void fast_sd_to_rom_full(uint16_t ram_addr) __z88dk_callee;
;-------------------------------------------------------------------------------
_fast_sd_to_rom_full:
    di
    ld a,1
    out (LED_IO),a              ; turn ROM led on
    pop iy                      ; return address
    pop de                      ; dest
    push iy                     ; put return address back on stack
    ld a,$FF
    out (SERIAL),a              ; flush shift register with ones
    ld c,0
copyouter:
    ld b,0
copynext:
    ; send 0xAA to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$AA
    out (ROM_IO),a
    
    ; send 0x55 to 0x2AAAA
    ld a,$2A
    out (ADDR_HIGH),a
    ld a,$AA
    out (ADDR_LOW),a
    ld a,$55
    out (ROM_IO),a

    ; send 0xA0 to 0x5555
    ld a,$55
    out (ADDR_HIGH),a
    ld a,$55
    out (ADDR_LOW),a
    ld a,$A0
    out (ROM_IO),a

    ; send byte from (hl) to ROM address in (de)
    ld a,d
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    out (CLKSTART),a            ; pulse clock, does not care about value of a
    in a, (SERIAL)              ; read value
    out (ROM_IO),a              ; store in ROM

    ; increment addresses and decrement counters
    inc de
    djnz copynext
    dec c
    jp nz, copyouter
    out (CLKSTART),a            ; two more pulses for the checksum
    out (CLKSTART),a
    ld a,0
    out (LED_IO),a              ; turn ROM led off
    ei
    ret