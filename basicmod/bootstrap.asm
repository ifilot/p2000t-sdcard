;-----------------------------------------------------
; bootstrap.asm
;
; Upon the first keyboard parse routine of the basic
; rom, executes the code located at $4EE0. This will
; modify a few pointers and loads in the code from
; the I/O port to ADDR EXCODE and launches it from there.
;
;-----------------------------------------------------

;-----------------------------------------------------
; VARIABLES
;-----------------------------------------------------
EXCODE:         EQU $7000       ; address to put and launch external code from
ROM_IO:         EQU $6C         ; external ROM
RAM_IO:         EQU $6D         ; external RAM
IO_AL:          EQU $68         ; address low
IO_AH:          EQU $69         ; address high
ROM_BANK:       EQU $6A         ; address ROM bank
RAM_BANK:       EQU $6B         ; address RAM bank
NUMBYTES:       EQU $2D00       ; hardcoded length of launcher program
PROGADDR:       EQU $0000       ; location on ROM where program resides
DEPLOYADDR:     EQU $6152       ; storage location of deploy addr

;-----------------------------------------------------
; CODE INJECTION PART
;-----------------------------------------------------
	; This part is injected into the standard BASIC cartridge
	; starting at $4EE0
    org $4EE0
	
    ; store bootstrap enable launch code  
    ld a,$55
    ld ($6150),a
    
    ; store byte character for jump
    ld a,$C3
    ld ($6151),a   

    ; store loadcode pointer (will be executed after rom boot)
    ld hl,loadcode
    ld ($6152),hl
    
    ; continue with normal execution of the ROM startup
    jp $1f5a

;-----------------------------------------------------
; OTHER CODE
;
; Load code from I/O port and launch the code
;
; bc - number of bytes
; de - start location on ROM
; hl - destination in RAM
;
;-----------------------------------------------------
loadcode:
    ld a,0
    ld ($6150),a        ; disable bootstrap routine
    ld hl,msgbl
    call printmsg
    di
    ld a,0
    out (ROM_BANK),a    ; set to bank 0
    out (RAM_BANK),a    ; set to bank 0
    ld bc,NUMBYTES      ; load number of bytes from internal ram
    ld hl,EXCODE        ; location where to write
    ld de,PROGADDR      ; location where to read
lcnextbyte:
    call read_rom
    ld (hl),a
    inc hl
    inc de
    dec bc
    ld a,b
    or c
    jr nz,lcnextbyte
    call EXCODE         ; call custom firmware code (will return here)
    call zeroram
    jp loadrom

msgbl:
    DB $06,$0D,"Booting launcher",$FF

;-----------------------------------------------------
; Load data from external rom
;-----------------------------------------------------
loadrom:
    ld a,1
    out (RAM_BANK), a   ; load programs from second RAM bank
    ld hl,msglp
    call printmsg
    ld de,$8000+1
    call read_ram       ; load high byte deploy addr
    ld (DEPLOYADDR+1),a
    ld de,$8000
    call read_ram       ; load low byte deploy addr
    ld (DEPLOYADDR),a
    ld de,$8000+3
    call read_ram       ; load high byte file size
    ld b,a
    ld de,$8000+2
    call read_ram       ; load low byte file size
    ld c,a
    call copydata       ; bc contains number of bytes
    xor a               ; set flags z, nc
    jp $28d4            ; launch basic program

msglp:
    DB $06,$0D,"Launching program",$FF

;-----------------------------------------------------
; Copy data from external ram to internal memory
;
; bc - number of bytes
;-----------------------------------------------------
copydata:
    di
    push bc                 ; store number of bytes
    ld de,$0000             ; start of external ram address
    ld hl,(DEPLOYADDR)      ; start of ram address
cdnextbyte:
    call read_ram           ; load from external ram into a register
    ld (hl),a
    inc de
    inc hl
    dec bc
    ld a,b
    or c
    jp nz,cdnextbyte
    pop bc

    ld hl,(DEPLOYADDR)      ; set deploy address
    ld ($625C), hl

    add hl,bc               ; add program length

    ; set basic pointers to variable space
    ld ($6405),hl
    ld ($6407),hl
    ld ($6409),hl

    ei
    ret

;-----------------------------------------------------
; Clear any remains from the previous program
;-----------------------------------------------------
zeroram:
    ld hl,EXCODE
    ld de,EXCODE+1
    ld bc,NUMBYTES-1
    ld a,0
    ld (hl),a
    ldir
    ret

;-----------------------------------------------------
; clear the screen
;-----------------------------------------------------
clrscrn:
    ld a,0 ; load 0 into first byte
    ld ($5000),a
    ld de,$5001
    ld bc,$1000
    dec bc
    ld hl,$5000
    ldir ; copy next byte from previous
    ret

;-----------------------------------------------------
; Print message to screen
; hl - pointer to string
;----------------------------------------------------
printmsg:
    call clrscrn
    ld bc,$5000 + $50*10
pmprint:
    ld a,(hl)
    cp 255
    ret z
    ld (bc),a
    inc hl
    inc bc
    jp pmprint

;-----------------------------------------------------
; go into infinite loop
;-----------------------------------------------------
loop:
    jp loop

;-----------------------------------------------------
; Reads a byte from the I/O port and put it in reg A.
; The address stored at IOREADADDR is used as the load
; address for the I/O port. Upon loading a byte, this
; address is incremented and stored back into the storage
; location.
;
; input: (IOREADADDR) I/O port address to read code from
;  uses: a,de
;
;-----------------------------------------------------
read_rom:
    ld a,d
    out (IO_AH),a         ; store upper bytes in register
    ld a,e
    out (IO_AL),a         ; store lower bytes in register
    in a,(ROM_IO)      ; load byte
    ret

;-----------------------------------------------------
; read a byte to external ram
;
;  a - byte to write
; de - address
;-----------------------------------------------------
read_ram:
    ld a,d
    out (IO_AH),a         ; store upper bytes in register
    ld a,e
    out (IO_AL),a         ; store lower bytes in register
    in a,(RAM_IO)       ; load byte
    ret

;-----------------------------------------------------
; write a byte to external ram
;
;  a - byte to write
; hl - address
;-----------------------------------------------------
write_ram:
    push af
    ld a,h
    out (IO_AH),a         ; store upper bytes in register
    ld a,l
    out (IO_AL),a         ; store lower bytes in register
    pop af
    out (RAM_IO),a      ; write byte
    ret
