all: flasher launcher launcher-slot1 ezlaunch

clean:
	rm -f *.bin *.BIN *.map

flasher: fat32.c flasher.c memory.c sst39sf.c util.c sdcard.c sdcard.asm terminal.c ram.asm util.asm rom.asm
	zcc \
	+embedded -clib=sdcc_iy \
	fat32.c flasher.c memory.c sst39sf.c \
	util.c sdcard.c sdcard.asm terminal.c \
	ram.asm sst39sf.asm rom.asm \
	util.asm \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CLIB_MALLOC_HEAP_SIZE=0 \
	-pragma-define:CLIB_STDIO_HEAP_SIZE=0 \
	--max-allocs-per-node3000 \
	-SO3 -bn FLASHER.BIN \
	-create-app -m \
	&& mv FLASHER.bin FLASHER.BIN \
	&& wc -c < FLASHER.BIN

launcher: main.c commands.c fat32.c memory.c terminal.c sdcard.c sdcard.asm ram.asm util.asm rom.asm crc16.asm launch_cas.asm
	zcc \
	+embedded -clib=sdcc_iy \
	commands.c fat32.c main.c memory.c terminal.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm rom.asm \
	terminal_ext.c crc16.asm launch_cas.asm \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0x7000 \
	-pragma-define:REGISTER_SP=-1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x12 \
	-pragma-define:CLIB_MALLOC_HEAP_SIZE=0 \
	-pragma-define:CLIB_STDIO_HEAP_SIZE=0 \
	--max-allocs-per-node200000 \
	--opt-code-size \
	-SO3 -bn LAUNCHER.BIN \
	-create-app -m \
	&& mv LAUNCHER.bin LAUNCHER.BIN \
	&& wc -c < LAUNCHER.BIN \
	&& truncate -s 11520 LAUNCHER.BIN

launcher-slot1: main.c commands.c fat32.c memory.c terminal.c sdcard.c sdcard.asm ram.asm util.asm rom.asm crc16.asm launch_cas.asm
	zcc \
	+embedded -clib=sdcc_iy \
	commands.c fat32.c main.c memory.c terminal.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm rom.asm \
	terminal_ext.c crc16.asm launch_cas.asm \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CLIB_MALLOC_HEAP_SIZE=0 \
	-pragma-define:CLIB_STDIO_HEAP_SIZE=0 \
	--max-allocs-per-node3000 \
	--opt-code-size \
	-SO3 -bn LAUNCHER-SLOT1.BIN \
	-create-app -m \
	&& mv LAUNCHER-SLOT1.bin LAUNCHER-SLOT1.BIN \
	&& wc -c < LAUNCHER-SLOT1.BIN

ezlaunch: easy-launcher.c fat32-easy.c memory.c sdcard.c sst39sf.c sdcard.asm ram.asm rom.asm launch_cas.asm sst39sf.asm
	zcc \
	-DNON_VERBOSE \
	+embedded -clib=sdcc_iy \
	easy-launcher.c fat32-easy.c memory.c sdcard.c sst39sf.c \
	sdcard.asm ram.asm rom.asm launch_cas.asm sst39sf.asm \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0x7000 \
	-pragma-define:REGISTER_SP=-1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x12 \
	-pragma-define:CLIB_MALLOC_HEAP_SIZE=0 \
	-pragma-define:CLIB_STDIO_HEAP_SIZE=0 \
	--max-allocs-per-node3000 \
	--opt-code-size \
	-SO3 -bn EZLAUNCH.BIN \
	-create-app -m \
	&& mv EZLAUNCH.bin EZLAUNCH.BIN \
	&& wc -c < EZLAUNCH.BIN \
	&& truncate -s 11520 EZLAUNCH.BIN

# @if grep -E 'sprintf|printf|fread|fwrite' EZLAUNCH.map ; then \
# 	echo "ERROR: stdio symbols found in EZLAUNCH.map!"; \
# 	exit 1; \
# fi