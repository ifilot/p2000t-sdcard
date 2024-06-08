all: sdcardtest

sdcardtest: *.asm *.c *.h
	zcc \
	+embedded -clib=sdcc_iy \
	ram.asm sdcard.asm util.asm *.c \
	-startup=1 \
	-pragma-define:CRT_ORG_CODE=0x1000 \
	-pragma-define:CRT_ORG_DATA=0x6100 \
	-pragma-define:REGISTER_SP=0x9FFF \
	-pragma-define:CRT_STACK_SIZE=256 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	--max-allocs-per-node2000 \
	-SO3 -bn SDCARDTEST.BIN \
	-create-app -m \
	&& mv SDCARDTEST.bin SDCARDTEST.BIN \
	&& wc -c < SDCARDTEST.BIN