SECTION code_user

ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
RAM_BANK        EQU  $6B
RAM_IO          EQU  $6D
LED_IO          EQU  $64

PUBLIC _crc16_ramchip
PUBLIC _copy_to_ram

PUBLIC _ram_write_byte
PUBLIC _ram_read_byte

PUBLIC _ram_write_uint16_t
PUBLIC _ram_write_uint32_t

PUBLIC _ram_read_uint16_t
PUBLIC _ram_read_uint32_t

PUBLIC _set_ram_bank

;-------------------------------------------------------------------------------
; uint16_t ram_read_uint16_t(uint16_t addr) __z88dk_callee;
;
; Note that Z80 is little endian and we adopt this standard also when writing to
; memory, so the lower byte is stored first.
;-------------------------------------------------------------------------------
_ram_read_uint16_t:
    pop de                      ; return address
    pop bc                      ; ramptr
    push de                     ; push return address back onto stack
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld l,a                      ; store lower byte first (little endian)
    inc bc
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld h,a                      ; then store upper byte
    ret                         ; result stored in HL

;-------------------------------------------------------------------------------
; uint32_t ram_read_uint32_t(uint16_t addr) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_read_uint32_t:
    pop de                      ; return address
    pop bc                      ; ramptr
    push de                     ; push return address back onto stack
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld e,a                      ; store byte 3
    inc bc                      ; next byte
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld d,a                      ; store byte 2
    inc bc                      ; next byte
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld l,a                      ; store byte 1
    inc bc                      ; next byte
    ld a,b
    out (ADDR_HIGH), a
    ld a,c
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld h,a                      ; store byte 0
    ret                         ; result stored in DEHL

;-------------------------------------------------------------------------------
; PUBLIC _set_ram_bank(uint8_t val)
;-------------------------------------------------------------------------------
_set_ram_bank:
    pop de                      ; return address
    dec sp                      ; increment sp for single byte argument
    pop af                      ; retrieve argument (stored in reg a)
    push de                     ; push return address back onto stack
    out (RAM_BANK),a
    ret

;-------------------------------------------------------------------------------
; ram_write_uint16_t(uint16_t addr, uint16_t val) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_write_uint16_t:
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; value to store
    push de                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,c                      ; grab lower byte
    out (RAM_IO),a              ; store lower byte first (little endian)
    inc hl                      ; next byte
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,b                      ; grab upper byte
    out (RAM_IO), a             ; store upper byte next
    ret

;-------------------------------------------------------------------------------
; ram_write_uint32_t(uint16_t addr, uint32_t val) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_write_uint32_t:
    pop iy                      ; return address
    pop hl                      ; ramptr
    pop de                      ; value to store upper bytes
    pop bc                      ; value to store lower bytes
    push iy                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,c                      ; grab byte 0
    out (RAM_IO),a              ; store byte 0 (little endian)
    inc hl                      ; next byte
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,b                      ; grab byte 1
    out (RAM_IO), a             ; store byte 1
    inc hl                      ; next byte
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,e                      ; grab byte 2
    out (RAM_IO),a              ; store byte 2
    inc hl                      ; next byte
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,d                      ; grab byte 3
    out (RAM_IO), a             ; store byte 3
    ret

;-------------------------------------------------------------------------------
; Write a byte to external RAM
;
; void ram_write_byte(uint16_t addr, uint8_t val);
;
; input:  hl pointer to ram address
;         b  byte to write
; return: void
;-------------------------------------------------------------------------------
_ram_write_byte:
    pop de                      ; return address
    pop hl                      ; ramptr
    dec sp                      ; decrement sp for 1-byte argument
    pop bc                      ; byte to write (stored in b)
    push de                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld a,b
    out (RAM_IO), a
    ret

;-------------------------------------------------------------------------------
; Read a byte from external RAM
;
; uint8_t ram_read_byte(uint16_t addr);
;
; input:  hl pointer to ram address
;
; return: value stored at memory address
;-------------------------------------------------------------------------------
_ram_read_byte:
    pop de                      ; return address
    pop hl                      ; ramptr
    push de                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    in a,(RAM_IO)
    ld h,a                      ; put return value in h-register
    ret

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
_crc16_ramchip:
    ld a,0x01
    out (LED_IO),a              ; turn RAM led on
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
    in a, (RAM_IO)              ; read byte from ram chip
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
    ld a,0x00
    out (LED_IO),a              ; turn RAM led off
    ret                         ; return value is stored in hl

;-------------------------------------------------------------------------------
; Copy bytes to external RAM chip
;
; void copy_to_ram(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;
;
; input:  hl - source address
;         de - destination address
;         bc - number of bytes
; uses: all
;-------------------------------------------------------------------------------
_copy_to_ram:
    ld a,0x02
    out (LED_IO),a              ; turn RAM led on
    pop iy                      ; return address
    pop hl                      ; src
    pop de                      ; dest
    pop bc                      ; number of bytes
    push iy                     ; put return address back on stack
next:
    ld a,d
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    ld a,(hl)
    out (RAM_IO),a
    inc de
    inc hl
    dec bc
    ld a,c
    or b
    jp nz, next
    ld a,0x00
    out (LED_IO),a              ; turn RAM led off
    ret