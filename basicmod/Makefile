BASICBOOTSTRAP.BIN: bootstrap.bin launcher.bin
	python3 hackrom.py

bootstrap.bin: bootstrap.asm
	z80asm bootstrap.asm -o bootstrap.bin

launcher.bin: launcher.asm
	z80asm launcher.asm -o launcher.bin