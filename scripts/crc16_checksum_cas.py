# -*- coding: utf-8 -*-

#
# Test CRC16 on dummy data
#

import os
import struct

def main():
    f = open(os.path.join('D:/', 'PROGRAMMING', 'P2000T', 'software', 'cassettes', 'games', 'U Hangt.cas'), 'rb')
    data = bytearray(f.read())
    f.close()

    # grab length from first preamble
    length = struct.unpack('<H', data[0x32:0x34])[0]
    print('Length: %04X' % length)
    
    stripped_data = bytearray()
    for i in range(len(data) // 0x500):
        stripped_data += data[i*0x500+0x100:(i+1)*0x500]
    stripped_data = stripped_data[:length]
    
    print(len(stripped_data))
        
    checksum = crc16(stripped_data)
    print('Checksum: %04X' % checksum)

def crc16(data):
    crc = int(0)
    
    poly = 0x1021
    
    for c in data: # fetch byte
        crc ^= (c << 8) # xor into top byte
        for i in range(8): # prepare to rotate 8 bits
            crc = crc << 1 # rotate
            if crc & 0x10000:
                crc = (crc ^ poly) & 0xFFFF # xor with XMODEN polynomic
    
    return crc

if __name__ == '__main__':
    main()