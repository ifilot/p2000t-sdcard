all: diagcart

diagcart: main.c fat32.c memory.c terminal.c sdcard.c sdcard.asm util.asm
	zcc \
	+embedded -clib=sdcc_iy \
	fat32.c main.c memory.c terminal.c terminal_ext.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm tape.asm \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6500 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn CASSETTEDUMP.BIN \
	-create-app -m \
	&& mv CASSETTEDUMP.bin CASSETTEDUMP.BIN \
	&& wc -c < CASSETTEDUMP.BIN \
	&& truncate -s 16k CASSETTEDUMP.BIN