;-----------------------------------------------------
; launcher.asm
;
; Small snippet of code that is inserted in the
; BASIC rom and allows for toggling execution of
; custom piece of code indicated by the hook located
; at $6150
; 
;-----------------------------------------------------
    org $4EC7
    ld a,($6150)
    cp $55
    jr z,run
    jp 0029h
run:
    jp 6151h