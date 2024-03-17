main.rom: main.c
	zcc \
	+embedded -clib=sdcc_iy \
	main.c \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0x7000 \
	-pragma-define:REGISTER_SP=-1 \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x33 \
	--max-allocs-per-node2000 \
	-SO3 -bn main.bin \
	-create-app -m \
	&& mv main.rom firmware.rom \
	&& truncate -s 8k firmware.rom