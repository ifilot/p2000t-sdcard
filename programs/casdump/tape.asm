;-------------------------------------------------------------------------------
; !!NOTE!!
;
; It is assumed that this file is compiled used SDCC_IY, by which the IX
; register is used for the frame pointer. Any function that uses IX should
; therefore push ix to the stack and pop it before exiting.
;
; See for example _tape_read_block
;-------------------------------------------------------------------------------

SECTION code_user

PUBLIC _tape_rewind
PUBLIC _tape_read_block
PUBLIC _tape_skip_forward

; constants for cassette instructions
CAS_INIT:   equ $00
CAS_REWIND: equ $01
CAS_SKIPF:  equ $02
CAS_SKIPB:  equ $03
CAS_EOT:    equ $04
CAS_WRITE:  equ $05
CAS_READ:   equ $06
CAS_STATUS: equ $07

; variables for cassette
CASSTAT:    equ $6017
TRANSFER:   equ $6030
LENGTH:     equ $6032
FILESIZE:   equ $6034
DESC1:      equ $6036
DESC2:      equ $6047
EXT:        equ $603E
FILETYPE:   equ $6041
BLOCKCTR:   equ $604F
MEMSIZE:    equ $605C
TAPE:       equ $0018   ; address of the "tape" function
;BUFFER:     equ $6100   ; position to store tape data

;-------------------------------------------------------------------------------
; Rewind the cassette
;-------------------------------------------------------------------------------
_tape_rewind:
    push ix
    ld a,CAS_INIT
    call TAPE
    ld a,CAS_REWIND
    call TAPE
    pop ix
    ret

;-------------------------------------------------------------------------------
; Read a single block from the tape to the buffer area
;
; INPUT: HL - buffer location
;-------------------------------------------------------------------------------
_tape_read_block:
    push ix             ; conserve ix because it is used as frame pointer
    ld a,(CASSTAT)      ; load tape status
    cp 'M'              ; check for M
    jp z,tprdexit
    ld ($6030),hl       ; store buffer location
    ld hl,$0400
    ld ($6032),hl       ; store amount of bytes to read
    ld ($6034),hl
    ld a,CAS_READ
    call TAPE
tprdexit:
    pop ix              ; retrieve ix
    ret

;-------------------------------------------------------------------------------
; Read a single block from the tape to the buffer area
;-------------------------------------------------------------------------------
_tape_skip_forward:
    push ix             ; conserve ix because it is used as frame pointer
    ;ld a,CAS_INIT
    ;call TAPE
    ld a,CAS_SKIPF
    call TAPE
    pop ix              ; retrieve ix
    ret