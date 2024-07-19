# -*- coding: utf-8 -*-

#
# Sign PRG file
#

import os
import argparse

def main():
    parser = argparse.ArgumentParser(
                    prog='PRG sign tool',
                    description='Insert CRC16 checksum in a blank PRG file')

    parser.add_argument('filename')           # positional argument

    args = parser.parse_args()

    if os.path.exists(args.filename):
        with open(args.filename, 'rb') as f:

            # load data
            data = bytearray(f.read())

            # check signature byte
            if data[0x0000] != 0x50:
                raise Exception('Invalid first byte: %02X' % data[0x0000])
            
            # calculate CRC
            length = len(data) - 0x10
            crc = crc16(data[0x10:])

            # give user output
            print('Data length: %i bytes' % length)
            print('CRC-16 checksum: 0x%04X' % crc)

            # insert length and crc into file (note little endian)
            data[0x01] = length & 0xFF
            data[0x02] = (length >> 8) & 0xFF
            data[0x03] = crc & 0xFF
            data[0x04] = (crc >> 8) & 0xFF

            # close file and reopen for writing
            f.close()
            with open(args.filename, 'wb') as f:
                f.write(data)
                f.close()

            # output
            print('Writing to: %s' % args.filename)
            print('Signed file: 0x%04X 0x%04X' % (length, crc))

    else:
        print('File does not exist: %s')

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