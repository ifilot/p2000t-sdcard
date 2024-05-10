all: helloworld

#
# Compilation settings explained
#
# CRT_ORG_CODE=A000    ; place program at $A000
# REGISTER_SP=0xDEFF   ; position stack top at 0xDEFF
# CRT_ON_EXIT=0x10002  ; return to caller --> return to LAUNCHER menu
# CRT_ENABLE_EIDI=0x22 ; set ei on both start and exit, ei is required for
#                        the keyboard to function

helloworld: main.c util.c
	zcc \
	+embedded -clib=sdcc_iy \
	main.c memory.c util.c \
	-startup=0 \
	-pragma-define:CRT_ORG_CODE=0xA000 \
	-pragma-define:REGISTER_SP=0xDEFF \
	-pragma-define:CLIB_FOPEN_MAX=0 \
	-pragma-define:CRT_ON_EXIT=0x10002 \
	-pragma-define:CRT_ENABLE_EIDI=0x22 \
	-pragma-define:CRT_INCLUDE_PREAMBLE=1 \
	--max-allocs-per-node2000 \
	--opt-code-size \
	--list \
	-SO3 -bn HELLOWORLD.BIN \
	-create-app -m \
	&& mv -v HELLOWORLD.bin HELLOWORLD.PRG \
	&& sleep 0.01 \
	&& wc -c < HELLOWORLD.PRG