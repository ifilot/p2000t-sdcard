# -*- coding: utf-8 -*-

import numpy as np

f = open('BASICROM.BIN', 'rb')
rom = bytearray(f.read())
f.close()

# verify checksum
llb = rom[0x0001] # low byte length
lhb = rom[0x0002] # high byte length
nbytes = (lhb * 256 + llb)

# calculate checksum
checksum = np.uint16(0)
for i in range(0x0005, 0x0005 + nbytes):
    checksum += rom[i]
checksum &= 0xFFFF

# verify whether initial checksum passes
if rom[0x0003] == checksum & 0xFF and rom[0x0004] == (checksum >> 8):
    print('Checksum passed')

# here a call is made to a hook address (0x1056)
print(['%02X' % rom[i] for i in range(0x0F71,0x0F74)])

# at 0x1056, a jump is made to the proper point, so by making
# an adjustment here to 0x4EEO, we can insert our own
# bootstrap code (bootstrap.asm)
print(['%02X' % rom[i] for i in range(0x0056,0x0059)])
rom[0x0057] = 0xE0 # lower byte
rom[0x0058] = 0x4E # upper byte

# adjust the check kb status call which normally links
# to jp 0029h (launcher.asm)
rom[0x008EB] = 0xC7
rom[0x008EC] = 0x4E

# recalculate checksum
checksum = np.uint16(0)
for i in range(0x0005, 0x0005 + nbytes):
    checksum += rom[i]
checksum &= 0xFFFF

# adjust checksum such that the 16-bit sum is zero
rom[0x0003] = (~(checksum & 0xFF) + 1) & 0xFF
rom[0x0004] = ~((checksum >> 8) & 0xFF) & 0xFF
#print("%04X" % checksum)
#print("%02X%02X" % (rom[0x0004],rom[0x0003]))

# insert new data
f = open('bootstrap.bin', 'rb')
bootstrap = bytearray(f.read())
f.close()
rom[0x3EE0:0x3EE0+len(bootstrap)] = bootstrap
print("Inserting custom boostrap: %i / 287 bytes" % len(bootstrap))

# auto-launcher
f = open('launcher.bin', 'rb')
launcher = bytearray(f.read())
f.close()
rom[0x3EC7:0x3EC7+len(launcher)] = launcher

print('Writing modified BASIC cartridge as BASICBOOTSTRAP.BIN...')
f = open('BASICBOOTSTRAP.BIN', 'wb')
f.write(rom)
f.close()