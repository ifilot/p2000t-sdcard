# -*- coding: utf-8 -*-

import os
import struct

#
# INSPIRATION: https://www.pjrc.com/tech/8051/ide/fat32.html
#

f = open(os.path.join('D:/', 'SDCARD', 'sdcard_rawdata_4G_cas_test03'), 'rb')
BYTES_PER_SECTOR = 512

def main():   
    # grab MBR
    data = get_sector(0)
    
    # establish LBA0 (start position first partition)
    lba0 = struct.unpack('<L', data[454:458])[0]
    print('LBA: 0x%04X' % (lba0 * BYTES_PER_SECTOR))
    
    # read VOLUME ID
    data = get_sector(lba0)
    bytes_per_sector = struct.unpack('<H', data[0x0B:0x0D])[0]
    sectors_per_cluster = data[0x0D]
    reserved_sectors = struct.unpack('<H', data[0x0E:0x10])[0]
    number_of_fats = data[0x10]
    sectors_per_fat = struct.unpack('<L', data[0x24:0x28])[0]
    root_dir_first_cluster = struct.unpack('<L', data[0x2C:0x30])[0]
    signature = struct.unpack('<H', data[0x1FE:0x200])[0]
    
    # list data
    print('Reserved sectors: %i' % reserved_sectors)
    print('Number of FATS: %i' % number_of_fats)
    print('Sectors per FAT: %i' % sectors_per_fat)
    print('Root dir first cluster: %i' % root_dir_first_cluster)
    print('Signature: %04X %s' % (signature, 'OK' if signature == 0xAA55 else 'FAIL'))
    print()
    
    # consolidate variables (all numbers are in 'sector-units')
    fat_begin_lba = lba0 + reserved_sectors
    cluster_begin_lba = lba0 + reserved_sectors + (number_of_fats * sectors_per_fat)
    lba_addr_root_dir = cluster_begin_lba + (root_dir_first_cluster - 2) * sectors_per_cluster
    
    print('Start address LBA0: %08X' % (lba0 * bytes_per_sector))
    print('Start address FAT: %08X' % (fat_begin_lba * bytes_per_sector))
    print('Start address root directory: %08X' % (lba_addr_root_dir * bytes_per_sector))
    
    # print all the files in the root directory
    ll = find_linked_list(fat_begin_lba, root_dir_first_cluster)
    print(['%08X' % x for x in ll])
    files = read_files_root_directory(cluster_begin_lba, ll, sectors_per_cluster)
    
    ll = find_linked_list(fat_begin_lba, 0x000000BC)
    files = read_files_root_directory(cluster_begin_lba, ll, sectors_per_cluster)
    
    # try to open a file
    # file = files[0]
    # print(file)
    # fll = find_linked_list(fat_begin_lba, file[2])
    # print(fll)
    # data = read_file(cluster_begin_lba, fll, file[1], sectors_per_cluster)
    # print_data(data, 32)
  
def find_linked_list(fat_begin_lba, nextcluster):
    clusters = []
    while nextcluster < 0x0FFFFFF8 and nextcluster != 0:
        clusters.append(nextcluster)
        data = get_sector(fat_begin_lba + (nextcluster >> 7))
        item = nextcluster & 0b01111111
        nextcluster = struct.unpack('<L', data[item*4:(item+1)*4])[0]    

    return clusters

def read_files_root_directory(cluster_begin_lba, linked_list, sectors_per_cluster):
    files = []
    
    for cluster in linked_list:
        addr = cluster_begin_lba + (cluster - 2) * sectors_per_cluster
        #print('Cluster addr: %08X' % addr)
        for i in range(sectors_per_cluster):
            data = get_sector(addr + i)
            for j in range(0,512//32):
                if data[j*32] == 0x00: # check if first byte is 0x00 -> end of dir
                    print('End of directory byte: entry %i, sector %i' % (j,i))
                    break
                else:
                    attrib = data[j*32+0x0B]
                    binstring = "{:08b}".format(attrib)
                    
                    if attrib & 0x1F == 0x00: # this is a file
                        try:
                            name = data[j*32:j*32+8].decode('ascii') + "." + data[j*32+8:j*32+11].decode('ascii')
                            size = struct.unpack('<L', data[j*32+28:(j+1)*32])[0]
                            fch = struct.unpack('<H', data[j*32+0x14:j*32+0x16])[0]
                            fcl = struct.unpack('<H', data[j*32+0x1A:j*32+0x1C])[0]
                            fc = (fch << 16) | fcl
                            files.append(
                                (name, size, fc)
                            )
                            print('[%s] (%03i) %s %i bytes (%08X)' % (binstring, len(files), name, size, fc))
                        except Exception as e:
                            print('Error: %s' % e)
                    elif attrib & 0x0F == 0x00 and attrib & (1 << 4): # this is a folder
                        fch = struct.unpack('<H', data[j*32+0x14:j*32+0x16])[0]
                        fcl = struct.unpack('<H', data[j*32+0x1A:j*32+0x1C])[0]
                        fc = (fch << 16) | fcl
                        print('[%s] (DIR) %s (%08X)' % (binstring,data[j*32:j*32+11].decode('ascii'),fc))
    
    return files

def read_file(cluster_begin_lba, linked_list, nrbytes, sectors_per_cluster):
    data = bytearray()

    for cluster in linked_list:
        addr = cluster_begin_lba + (cluster - 2) * sectors_per_cluster
        for i in range(sectors_per_cluster):
            sectordata = get_sector(addr + i)
            data += sectordata
    
    return data[0:nrbytes]

def print_data(data, nrlines):
    for i in range(nrlines):
        for j in range(0,16):
            print('%02X ' % data[i*8+j], end='')
        print('  ', end='')
        for j in range(0,16):
            if data[i*16+j] >= 32 and data[i*16+j] <= 126 :
                print(chr(data[i*16+j]), end='')
            else:
                print('.', end='')
        print()

def get_sector(sector_id):
    f.seek(sector_id * BYTES_PER_SECTOR)
    return f.read(BYTES_PER_SECTOR)
 
if __name__ == '__main__':
    main()