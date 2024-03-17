SECTION code_user

ADDR_LOW        EQU  $68
ADDR_HIGH       EQU  $69
RAM_IO          EQU  $6D

PUBLIC _ram_write_byte
PUBLIC _ram_read_byte

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;
; IMPORTANT: THE FUNCTIONS AS SHOWN BELOW DO NOT WORK; SOMETHING STILL GOES
;            WRONG WHEN PARSING THE VARIABLES FROM AND TO STACK / REGISTERS
;            DO NOT USE UNTIL FIXED!!!
;
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

;-------------------------------------------------------------------------------
; Write a byte to external RAM
;
; void ram_write_byte(uint16_t addr, uint8_t val);
;
; input:  hl pointer to ram address
;         bc byte to write (stored in b)
; return: void
;-------------------------------------------------------------------------------
_ram_write_byte:
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; byte to write (stored in b)
    push de                     ; push return address back onto stack
    ld a,h
    out (ADDR_HIGH), a
    ld a,l
    out (ADDR_LOW), a
    ld b,a
    out (RAM_IO), a
    ret

;-------------------------------------------------------------------------------
; Read a byte from external RAM
;
; uint8_t ram_read_byte(uint16_t addr);
;
; input:  hl pointer to ram address
;
; return: void
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
    ld l,a                      ; place return value in l
    ret