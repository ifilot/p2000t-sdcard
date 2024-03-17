#
# Quick upload method to development cartridge
#

import numpy as np
import serial
import serial.tools.list_ports
from tqdm import tqdm

def main():
    print('======================================')
    print('PYTHON FLASH ROUTINE FOR P2K CARTRIDGE')
    print('======================================')
    ser = connect()
    test_board_id(ser)
    upload_rom(ser, 'BASICBOOTSTRAP.bin')
    ser.close()

def connect():
    # autofind any available boards
    ports = serial.tools.list_ports.comports()
    portfound = None
    for port in ports:
        #print(port.pid, port.vid)
        if port.pid == 54 and port.vid == 0x2341:
            portfound = port.device
            break

    # specify the COM port below
    if portfound:
        ser = serial.Serial(portfound, 
                            19200, 
                            bytesize=serial.EIGHTBITS,
                            parity=serial.PARITY_NONE,
                            stopbits=serial.STOPBITS_ONE,
                            timeout=None)  # open serial port
                   
        if not ser.isOpen():
            ser.open()
    
    return ser

def test_board_id(ser):
    ser.write(b'READINFO')
    res = ser.read(8)
    #print(res)
    res = ser.read(16)
    #print(res)
    
    if res == b'Ph2k-32u4-v1.0.3':
        print('Connection established. All ok!')
    else:
        raise Exception('Cannot connect. Invalid response.')

def upload_rom(ser, filename):
    ser.write(b'DEVIDSST')
    rsp = ser.read(8)
    rsp = ser.read(2)
    if rsp == bytearray([0xBF,0xB7]):
        print('Chip ID verified: %02X %02X' % (rsp[0], rsp[1]))
    else:
        raise Exception("Incorrect chip id.")
        
    f = open(filename, 'rb')
    data = bytearray(f.read())
    f.close()
    
    # wipe first bank
    print('Wiping banks: ', end='')
    for i in range(0,4):
        ser.write(b'ESST00%02X' % (i * 0x10))
        res = ser.read(8)
        res = ser.read(2)
        print("%02X" % i, end='')
    print()

    for i in tqdm(range(0, 16 * 1024 // 256), desc='Writing blocks'):
        ser.write(b'WRBK%04X' % i)
        res = ser.read(8)
        #print(res)
        parcel = data[i*256:(i+1)*256]
        ser.write(parcel)
        checksum = np.uint8(ser.read(1)[0])
        comp_checksum = np.sum(parcel) & 0xFF
        
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'        
        
if __name__ == '__main__':
    main()