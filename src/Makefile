all: flasher launcher launcher-slot1

flasher: fat32.c flasher.c memory.c sst39sf.c util.c sdcard.c sdcard.asm terminal.c copy.c copy.asm ram.asm util.asm
	zcc \
	+embedded -clib=sdcc_iy \
	fat32.c flasher.c memory.c sst39sf.c \
	util.c sdcard.c sdcard.asm terminal.c \
	copy.c copy.asm ram.asm sst39sf.asm \
	util.asm \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	--max-allocs-per-node2000 \
	-SO3 -bn FLASHER.BIN \
	-create-app -m \
	&& mv FLASHER.bin FLASHER.BIN \
	&& wc -c < FLASHER.BIN

launcher: main.c commands.c fat32.c memory.c terminal.c sdcard.c sdcard.asm ram.asm util.asm rom.asm
	zcc \
	+embedded -clib=sdcc_iy \
	commands.c fat32.c main.c memory.c terminal.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm rom.asm \
	terminal_ext.c \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0x7000 \
	-pragma-define:REGISTER_SP=-1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x33 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn LAUNCHER.BIN \
	-create-app -m \
	&& mv LAUNCHER.bin LAUNCHER.BIN \
	&& wc -c < LAUNCHER.BIN \
	&& truncate -s 11520 LAUNCHER.BIN

launcher-slot1: main.c commands.c fat32.c memory.c terminal.c sdcard.c sdcard.asm ram.asm util.asm rom.asm
	zcc \
	+embedded -clib=sdcc_iy \
	commands.c fat32.c main.c memory.c terminal.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm rom.asm \
	terminal_ext.c \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn LAUNCHER-SLOT1.BIN \
	-create-app -m \
	&& mv LAUNCHER-SLOT1.bin LAUNCHER-SLOT1.BIN \
	&& wc -c < LAUNCHER-SLOT1.BIN