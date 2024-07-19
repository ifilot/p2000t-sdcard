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

PUBLIC _replace_bytes
PUBLIC _read_uint16_t
PUBLIC _read_uint32_t
PUBLIC _get_stack_location
PUBLIC _call_program
PUBLIC _hexcode_to_uint16t

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

;-------------------------------------------------------------------------------
; uint16_t read_uint16_t(const uint8_t* data)
;-------------------------------------------------------------------------------
_read_uint16_t:
    pop iy                  ; return address
    pop de                  ; retrieve memory pointer
    ld a,(de)               ; lower byte
    ld l,a
    inc de
    ld a,(de)               ; upper byte
    ld h,a
    push iy
    ret                     ; result in HL

;-------------------------------------------------------------------------------
; uint32_t read_uint32_t(const uint8_t* data)
;-------------------------------------------------------------------------------
_read_uint32_t:
    pop iy                  ; return address
    pop bc                  ; retrieve memory pointer
    ld a,(bc)               ; byte 0
    ld l,a
    inc bc
    ld a,(bc)               ; byte 1
    ld h,a
    inc bc
    ld a,(bc)               ; byte 2
    ld e,a
    inc bc
    ld a,(bc)               ; byte 3
    ld d,a
    push iy
    ret                     ; result in DEHL

;-------------------------------------------------------------------------------
; uint16_t get_stack_location(void)
;-------------------------------------------------------------------------------
_get_stack_location:
    ld hl,0
    add hl,sp
    ret                     ; result in HL

;-------------------------------------------------------------------------------
; void call_program(uint16_t) __z88dk_callee;
;
; This subroutine is a bit of a hack. It mimics an indirect call.
;
;-------------------------------------------------------------------------------
_call_program:
    pop iy                  ; return address
    pop hl                  ; pointer address
    push iy                 ; put return address on the stack
    jp (hl)

;-------------------------------------------------------------------------------
; uint16_t hexcode_to_uint16t(uint8_t *addr) __z88dk_callee;
;-------------------------------------------------------------------------------
_hexcode_to_uint16t:
    pop iy                  ; get return address
    pop hl                  ; get pointer to memory address
    call hex_to_uint8_t     ; returns num in a and increments hl by 2
    ld d,a                  ; store upper byte
    call hex_to_uint8_t     ; returns num in a and increments hl by 2
    ld e,a                  ; store lower byte
    ex de,hl                ; put ex into hl
    push iy                 ; put return address back onto the stack
    ret                     ; return result in HL

;-------------------------------------------------------------------------------
; convert two hex chars to 8-bit unsigned integer
;
; input    - hl : memory position
;
; output   - a : 8 bit integer
;
; garbles: - c
;          - hl' = hl + 2
;-------------------------------------------------------------------------------
hex_to_uint8_t:
    ld a,(hl)
    call hex_to_int         ; convert a
    or a                    ; clear carry
    rla                     ; shift left four times
    rla
    rla
    rla
    and 0xF0                ; zero lower bits
    ld c,a                  ; store upper nibble
    inc hl                  ; increment pointer
    ld a,(hl)               ; load next char
    call hex_to_int         ; convert to num and store in a
    or c                    ; place upper nibble into a
    inc hl
    ret                     ; a contains 8-bit unsigned integer

;-------------------------------------------------------------------------------
; convert hexadecimal character stored in 'A' to numeric value
; input:  a - hexadecimal character
; output: a - numeric value
; 
; sets 0x00 for invalid number
;-------------------------------------------------------------------------------
hex_to_int:
    ld a,(hl)               ; load character into a
    cp '0'
    jr c, hex_end
    cp '9'
    jr nc, hex_alpha_check
    sub '0'
    ret
hex_alpha_check:
    cp 'A'
    jr c, hex_invalid
    cp 'F'
    jr nc, hex_invalid
    sub 'A' - 10
hex_end:
    ret
hex_invalid:
    ld de,0
    ret