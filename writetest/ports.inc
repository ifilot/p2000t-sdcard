BASEPORT        EQU     $40

SERIAL          EQU     (BASEPORT | $00)
CLKSTART        EQU     (BASEPORT | $01)
DESELECT        EQU     (BASEPORT | $02)
SELECT          EQU     (BASEPORT | $03)
LED_IO          EQU     (BASEPORT | $04)
ADDR_LOW        EQU     (BASEPORT | $08)
ADDR_HIGH       EQU     (BASEPORT | $09)
ROM_BANK        EQU     (BASEPORT | $0A)
RAM_BANK        EQU     (BASEPORT | $0B)
ROM_IO          EQU     (BASEPORT | $0C)
RAM_IO          EQU     (BASEPORT | $0D)