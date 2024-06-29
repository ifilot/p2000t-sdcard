#!/usr/bin/env python

def main():
    pass

def main():
    create_rom_default()

def create_rom_default():
    """
    Build the multirom image
    """
    data = bytearray()

    datalist = [
        'basicmod/BASICROM.BIN',      # regular BASIC NL v1.1    (00)
        'roms/basic_nl_v1_1_a2.bin',  # regular BASIC NL v1.1a2  (01)
        'roms/basic_en_v1_0.bin',     # regular BASIC EN v1.1a2  (02)
        'roms/JWSBasic.bin',          # JWS Basic                (03)
        'BASICBOOTSTRAP.BIN',         # SD-CARD BASIC            (04)
        'FLASHER.BIN',                # flasher utility          (05)
        'RAMTEST.BIN',                # ram test                 (06)
        'roms/maintenance_v2.bin',    # maintenance utility      (07)
        'roms/Forth.bin',             # Forth programming lang   (08)
        'roms/assembler_v5_9.bin',    # Assembler 5.9            (09)
        'roms/zemon_v1_4.bin',        # Zemon v1.4               (0A)
        'roms/flexbase_v1_6.bin',     # Flexbase v1.6            (0B) 
        'roms/familiegeheugen_v4.bin',# Familiegeheugen v4       (0C)
        'roms/tekst_v1.bin',          # Tekst v1                 (0D)
        'roms/text2000_v3.bin',       # Text2000 v3              (0E)
        'roms/text2_de_v2.bin',       # Text2 DE v2              (0F)
        'roms/wordproc_v2.bin',       # Wordproc v2              (10)
    ]

    for rom in datalist:
        data.extend(read_rom(rom, False))

    data.extend([0x00] * (512 * 1024 - len(data)))

    print(len(data) / 1024)
        
    f = open('MULTIROM.BIN', 'wb')
    f.write(data)
    f.close()

def read_rom(filename, swap=False):
    """
    Read a ROM file from BIN file and automatically expand it to 16KiB
    """
    f = open(filename, 'rb')
    data = bytearray(f.read())
    data.extend([0x00] * (0x4000 - len(data)))
    
    # swap upper and lower halfs
    if swap:
        dataswapped = data[0x2000:0x4000]
        dataswapped.extend(data[0x0000:0x2000])
        data = dataswapped
       
    f.close()
    return data

if __name__ == '__main__':
    main()