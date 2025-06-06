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

PUBLIC _set_ram_address
PUBLIC _set_ram_bank

PUBLIC _ram_read_uint8_t
PUBLIC _ram_read_uint16_t
PUBLIC _ram_read_uint32_t

PUBLIC _ram_write_uint8_t
PUBLIC _ram_write_uint16_t

PUBLIC _copy_to_ram
PUBLIC _copy_from_ram
PUBLIC _ram_transfer

;-------------------------------------------------------------------------------
; SETTER FUNCTIONS
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Set the external ram pointer
;
; input: hl
; garbles: a
;-------------------------------------------------------------------------------
_set_ram_address:
    ld a,h
    out (ADDR_HIGH),a           ; set upper byte address
    ld a,l
    out (ADDR_LOW),a            ; set lower byte address
    ret

;-------------------------------------------------------------------------------
; void set_ram_bank(uint8_t val) __z88dk_fastcall;
;
; input: l - ram bank
; garbles: a
;-------------------------------------------------------------------------------
_set_ram_bank:
    ld a,l
    out (RAM_BANK),a
    ret

;-------------------------------------------------------------------------------
; READ FUNCTIONS
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Read a byte from external RAM
;
; uint8_t ram_read_byte(uint16_t addr);
;
; input: hl - pointer to ram address
;
; return: l - value stored at memory address
;-------------------------------------------------------------------------------
_ram_read_uint8_t:
    call ram_ptr_hl_ret_a
    ld l,a                      ; put return value in l-register
    ret

;-------------------------------------------------------------------------------
; uint16_t ram_read_uint16_t(uint16_t addr) __z88dk_fastcall;
;
; Input: hl - ram address
; Garbles: a,de,hl
; Return: hl - result
;-------------------------------------------------------------------------------
_ram_read_uint16_t:
    call ram_ptr_hl_ret_a
    ld e,a                      ; store lower byte first (little endian)
    inc hl
    call ram_ptr_hl_ret_a
    ld d,a                      ; then store upper byte
    ex de,hl                    ; transfer de to return register hl
    ret                         ; result stored in HL

;-------------------------------------------------------------------------------
; uint32_t ram_read_uint32_t(uint16_t addr) __z88dk_fastcall;
;
; Input: hl - ram address
; Garbles: all
; Return: hl - result
;-------------------------------------------------------------------------------
_ram_read_uint32_t:
    push hl                     ; put ram pointer on stack
    call _ram_read_uint16_t     ; retrieve low word in hl (little endian)
    ex de,hl                    ; low word placed in de
    pop hl                      ; retrieve ram pointer from stack
    push de                     ; put low word on stack
    inc hl                      ; increment ram pointer twice
    inc hl
    call _ram_read_uint16_t     ; retrieve high word in hl, low word on stack
    pop de                      ; retrieve low word from stack
    ex de,hl                    ; swap high and low word
    ret                         ; result stored in DEHL

;-------------------------------------------------------------------------------
; WRITE FUNCTIONS
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; void ram_write_uint8_t(uint16_t addr, uint8_t val) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_write_uint8_t:
    pop de                      ; return address
    pop hl                      ; ramptr
    call _set_ram_address
    dec sp                      ; decrement sp for 1-byte argument
    pop af                      ; byte to write (stored in b)
    push de                     ; push return address back onto stack
    out (RAM_IO), a
    ret

;-------------------------------------------------------------------------------
; void ram_write_uint16_t(uint16_t addr, uint16_t val) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_write_uint16_t:
    pop de                      ; return address
    pop hl                      ; ramptr
    pop bc                      ; value to store
    push de                     ; push return address back onto stack
    call _set_ram_address
    ld a,c                      ; grab lower byte
    out (RAM_IO),a              ; store lower byte first (little endian)
    inc hl                      ; increment ram pointer
    call _set_ram_address
    ld a,b                      ; grab upper byte
    out (RAM_IO), a             ; store upper byte next
    ret

;-------------------------------------------------------------------------------
; COPY FUNCTIONS
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Copy bytes to external RAM chip
;
; void copy_to_ram(uint8_t *src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;
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
nextto:
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
    jp nz,nextto
    ld a,0x00
    out (LED_IO),a              ; turn RAM led off
    ret

;-------------------------------------------------------------------------------
; Copy bytes from external RAM chip
;
; void copy_from_ram(uint16_t src, uint8_t *dest, uint16_t nrbytes) __z88dk_callee;
;
; input:  hl - source address
;         de - destination address
;         bc - number of bytes
; uses: all
;-------------------------------------------------------------------------------
_copy_from_ram:
    ld a,0x01
    out (LED_IO),a              ; turn read LEd on
    pop iy                      ; return address
    pop hl                      ; src
    pop de                      ; dest
    pop bc                      ; number of bytes
    push iy                     ; put return address back on stack
nextfrom:
    ld a,h
    out (ADDR_HIGH),a
    ld a,l
    out (ADDR_LOW),a
    in a,(RAM_IO)
    ld (de),a
    inc de
    inc hl
    dec bc
    ld a,c
    or b
    jr nz,nextfrom
    ld a,0x00
    out (LED_IO),a              ; turn RAM led off
    ret

;-------------------------------------------------------------------------------
; void ram_transfer(uint16_t src, uint16_t dest, uint16_t nrbytes) __z88dk_callee;
;-------------------------------------------------------------------------------
_ram_transfer:
    ld a,0x03
    out (LED_IO),a              ; turn read and write LEDs on (transfer)
    pop iy                      ; return address
    pop hl                      ; src
    pop de                      ; dest
    pop bc                      ; number of bytes
    push iy                     ; put return address back on stack
transferbyte:
    ld a,h                      ; set source address
    out (ADDR_HIGH),a
    ld a,l
    out (ADDR_LOW),a
    in a,(RAM_IO)               ; retrieve byte
    ld iyl,a                    ; store temporarily in iyl
    
    ld a,d                      ; set destination address
    out (ADDR_HIGH),a
    ld a,e
    out (ADDR_LOW),a
    ld a,iyl                    ; retrieve byte
    out (RAM_IO),a              ; put at destination
    
    inc de
    inc hl
    dec bc
    ld a,c
    or b
    jp nz,transferbyte
    ld a,0x00
    out (LED_IO),a              ; turn leds off
    ret

;-------------------------------------------------------------------------------
; AUXILIARY ROUNTINES
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Set the external ram pointer
;
; input: hl
; return: a - result
;-------------------------------------------------------------------------------
ram_ptr_hl_ret_a:
    ld a,h
    out (ADDR_HIGH),a           ; set upper byte address
    ld a,l
    out (ADDR_LOW),a            ; set lower byte address
    in a,(RAM_IO)
    ret