all: cassettedump-slot1 cassettedump-prg

cassettedump-slot1: main.c fat32.c memory.c terminal.c sdcard.c sdcard.asm util.asm
	cp -v crt_preamble_cart.asm crt_preamble.asm && \
	zcc \
	+embedded -clib=sdcc_iy \
	fat32.c main.c memory.c terminal.c terminal_ext.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm tape.asm \
	-startup=1 \
	-pragma-define:SLOT1=0 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn CASSETTEDUMP.BIN \
	-create-app -m \
	&& mv CASSETTEDUMP.bin CASSETTEDUMP.BIN \
	&& sleep 0.01 \
	&& wc -c < CASSETTEDUMP.BIN \
	&& truncate -s 16k CASSETTEDUMP.BIN

cassettedump-prg: main.c fat32.c memory.c terminal.c sdcard.c sdcard.asm util.asm
	cp -v crt_preamble_prg.asm crt_preamble.asm && \
	zcc \
	+embedded -clib=sdcc_iy \
	fat32.c main.c memory.c terminal.c terminal_ext.c \
	util.c sdcard.c sdcard.asm ram.asm util.asm tape.asm \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0xA000 \
	-pragma-define:REGISTER_SP=0xDEFF \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x22 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	-SO3 -bn CASDUMP.BIN \
	-create-app -m \
	&& mv CASDUMP.bin CASDUMP.PRG \
	&& sleep 0.01 \
	&& wc -c < CASDUMP.PRG