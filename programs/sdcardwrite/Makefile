all: sdcardwrite

#
# Compilation settings explained
#
# CRT_ORG_CODE=A000    ; place program at $A000
# REGISTER_SP=0xDEFF   ; position stack top at 0xDEFF
# CRT_ON_EXIT=0x10002  ; return to caller --> return to LAUNCHER menu
# CRT_ENABLE_EIDI=0x22 ; set ei on both start and exit, ei is required for
#                        the keyboard to function

sdcardwrite: *.c *.asm
	zcc \
	+embedded -clib=sdcc_iy \
	main.c \
	memory.c \
	util.c \
	terminal.c \
	util.asm \
	crc16.asm \
	fat32.c \
	sdcard.c \
	sdcard.asm \
	ram.asm \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0xA000 \
	-pragma-define:REGISTER_SP=0xDEFF \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x22 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn SDCARDWR.BIN \
	-create-app -m \
	&& mv -v SDCARDWR.bin SDCARDWR.PRG \
	&& sleep 0.01 \
	&& wc -c < SDCARDWR.PRG