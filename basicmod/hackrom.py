# -*- coding: utf-8 -*-

# This script adds the bootstrap code at the end of the BASIC ROM, and modifies
# the ROM to call the bootstrap after startup.

CART_ORIGIN = 0x1000
BOOTSTRAP_ADDR = 0x4EC7
BOOTSTRAP_OFFSET = BOOTSTRAP_ADDR - CART_ORIGIN
BOOTSTRAP_MAX_SIZE = 0x4000 - BOOTSTRAP_OFFSET

with open('BASICROM.BIN', 'rb') as f:
    rom = bytearray(f.read())

# To call our custom code (at $4EC7) after startup, page 18 of 'Adresboekje' 
# tells us to use addresses $60D0 or $60D3 to hook into the Basic interpreter.
# Address $60D3 maps to position $08E7 in Basic ROM (see 'Adresboekje' page 13)
rom[0x08E7] = 0xC3 # 'jp nn' opcode
rom[0x08E8:0x08EA] = BOOTSTRAP_ADDR.to_bytes(2, byteorder='little')

# recalculate checksum
checksum = 0
nbytes = int.from_bytes(rom[0x0001:0x0003], byteorder='little')
for i in range(0x0005, 0x0005 + nbytes):
    checksum += rom[i]

# put the 2's complement negation of the checksum back in the ROM header
rom[0x0003:0x0005] = ((~checksum+1) & 0xFFFF).to_bytes(2, byteorder='little')

# insert bootstrap code at 0x3EC7
with open('bootstrap.bin', 'rb') as f:
    bootstrap = bytearray(f.read())
    rom[BOOTSTRAP_OFFSET : BOOTSTRAP_OFFSET + len(bootstrap)] = bootstrap
    print(f"Inserting boostrap: {len(bootstrap)} / {BOOTSTRAP_MAX_SIZE} bytes")

print('Writing modified BASIC cartridge as BASICBOOTSTRAP.BIN...')
with open('BASICBOOTSTRAP.BIN', 'wb') as f:
    f.write(rom)