                    ld        d,b                           ;[0000] 50
                    nop                                     ;[0001] 00
                    nop                                     ;[0002] 00
                    nop                                     ;[0003] 00
                    nop                                     ;[0004] 00
                    ld        c,b                           ;[0005] 48
                    ld        b,l                           ;[0006] 45
                    ld        c,h                           ;[0007] 4c
                    ld        c,h                           ;[0008] 4c
                    ld        c,a                           ;[0009] 4f
                    ld        d,a                           ;[000a] 57
                    ld        c,a                           ;[000b] 4f
                    ld        d,d                           ;[000c] 52
                    ld        d,b                           ;[000d] 50
                    ld        d,d                           ;[000e] 52
                    ld        b,a                           ;[000f] 47
                    jp        $a013                         ;[0010] c3 13 a0
                    ld        ($a1d9),sp                    ;[0013] ed 73 d9 a1
                    ld        sp,$deff                      ;[0017] 31 ff de
                    ld        hl,$a155                      ;[001a] 21 55 a1
                    ld        bc,$0080                      ;[001d] 01 80 00
                    call      $a050                         ;[0020] cd 50 a0
                    ld        hl,$ffff                      ;[0023] 21 ff ff
                    add       hl,sp                         ;[0026] 39
                    ld        bc,$a1db                      ;[0027] 01 db a1
                    or        a                             ;[002a] b7
                    sbc       hl,bc                         ;[002b] ed 42
                    jp        c,$a048                       ;[002d] da 48 a0
                    ld        bc,$020e                      ;[0030] 01 0e 02
                    sbc       hl,bc                         ;[0033] ed 42
                    jp        c,$a048                       ;[0035] da 48 a0
                    ld        bc,$000f                      ;[0038] 01 0f 00
                    add       hl,bc                         ;[003b] 09
                    ld        b,h                           ;[003c] 44
                    ld        c,l                           ;[003d] 4d
                    ld        hl,$a1db                      ;[003e] 21 db a1
                    call      $a050                         ;[0041] cd 50 a0
                    ei                                      ;[0044] fb
                    call      $a0cc                         ;[0045] cd cc a0
                    push      hl                            ;[0048] e5
                    ei                                      ;[0049] fb
                    pop       hl                            ;[004a] e1
                    ld        sp,($a1d9)                    ;[004b] ed 7b d9 a1
                    ret                                     ;[004f] c9

                    ld        d,h                           ;[0050] 54
                    ld        e,l                           ;[0051] 5d
                    push      hl                            ;[0052] e5
                    push      bc                            ;[0053] c5
                    ld        c,$01                         ;[0054] 0e 01
                    call      $a0ad                         ;[0056] cd ad a0
                    jp        c,$a073                       ;[0059] da 73 a0
                    ld        hl,$0006                      ;[005c] 21 06 00
                    add       hl,de                         ;[005f] 19
                    ex        de,hl                         ;[0060] eb
                    pop       bc                            ;[0061] c1
                    add       hl,bc                         ;[0062] 09
                    xor       a                             ;[0063] af
                    dec       hl                            ;[0064] 2b
                    ld        (hl),a                        ;[0065] 77
                    dec       hl                            ;[0066] 2b
                    ld        (hl),a                        ;[0067] 77
                    ex        de,hl                         ;[0068] eb
                    ld        (hl),e                        ;[0069] 73
                    inc       hl                            ;[006a] 23
                    ld        (hl),d                        ;[006b] 72
                    inc       hl                            ;[006c] 23
                    call      $a0a0                         ;[006d] cd a0 a0
                    pop       hl                            ;[0070] e1
                    ret                                     ;[0071] c9

                    pop       hl                            ;[0072] e1
                    pop       hl                            ;[0073] e1
                    pop       hl                            ;[0074] e1
                    ld        l,$ff                         ;[0075] 2e ff
                    ld        h,$00                         ;[0077] 26 00
                    ld        ($a153),hl                    ;[0079] 22 53 a1
                    jp        $a083                         ;[007c] c3 83 a0
                    pop       hl                            ;[007f] e1
                    pop       hl                            ;[0080] e1
                    pop       hl                            ;[0081] e1
                    pop       hl                            ;[0082] e1
                    ld        hl,$0000                      ;[0083] 21 00 00
                    scf                                     ;[0086] 37
                    ret                                     ;[0087] c9

                    ld        (hl),a                        ;[0088] 77
                    inc       hl                            ;[0089] 23
                    ld        (hl),a                        ;[008a] 77
                    inc       hl                            ;[008b] 23
                    ld        (hl),a                        ;[008c] 77
                    inc       hl                            ;[008d] 23
                    ld        (hl),a                        ;[008e] 77
                    inc       hl                            ;[008f] 23
                    ld        (hl),a                        ;[0090] 77
                    inc       hl                            ;[0091] 23
                    ld        (hl),a                        ;[0092] 77
                    inc       hl                            ;[0093] 23
                    ld        (hl),a                        ;[0094] 77
                    inc       hl                            ;[0095] 23
                    ld        (hl),a                        ;[0096] 77
                    inc       hl                            ;[0097] 23
                    ld        (hl),a                        ;[0098] 77
                    inc       hl                            ;[0099] 23
                    ld        (hl),a                        ;[009a] 77
                    inc       hl                            ;[009b] 23
                    ld        (hl),a                        ;[009c] 77
                    inc       hl                            ;[009d] 23
                    ld        (hl),a                        ;[009e] 77
                    inc       hl                            ;[009f] 23
                    ld        (hl),a                        ;[00a0] 77
                    inc       hl                            ;[00a1] 23
                    ld        (hl),a                        ;[00a2] 77
                    inc       hl                            ;[00a3] 23
                    ld        (hl),a                        ;[00a4] 77
                    inc       hl                            ;[00a5] 23
                    ld        (hl),a                        ;[00a6] 77
                    inc       hl                            ;[00a7] 23
                    ret                                     ;[00a8] c9

                    pop       hl                            ;[00a9] e1
                    pop       hl                            ;[00aa] e1
                    pop       hl                            ;[00ab] e1
                    ret                                     ;[00ac] c9

                    ld        a,c                           ;[00ad] 79
                    and       $f8                           ;[00ae] e6 f8
                    jr        nz,$00c7                      ;[00b0] 20 15
                    ld        a,c                           ;[00b2] 79
                    and       $07                           ;[00b3] e6 07
                    jr        z,$00c7                       ;[00b5] 28 10
                    xor       a                             ;[00b7] af
                    call      $a09c                         ;[00b8] cd 9c a0
                    dec       hl                            ;[00bb] 2b
                    dec       hl                            ;[00bc] 2b
                    dec       hl                            ;[00bd] 2b
                    ld        (hl),$fe                      ;[00be] 36 fe
                    dec       hl                            ;[00c0] 2b
                    dec       hl                            ;[00c1] 2b
                    ld        (hl),c                        ;[00c2] 71
                    ld        hl,$0000                      ;[00c3] 21 00 00
                    ret                                     ;[00c6] c9

                    ld        hl,$0001                      ;[00c7] 21 01 00
                    scf                                     ;[00ca] 37
                    ret                                     ;[00cb] c9

                    ld        hl,($a14b)                    ;[00cc] 2a 4b a1
                    ld        (hl),$00                      ;[00cf] 36 00
                    ld        e,l                           ;[00d1] 5d
                    ld        d,h                           ;[00d2] 54
                    inc       de                            ;[00d3] 13
                    ld        bc,$0fff                      ;[00d4] 01 ff 0f
                    ldir                                    ;[00d7] ed b0
                    ld        de,($a14b)                    ;[00d9] ed 5b 4b a1
                    ld        bc,$000b                      ;[00dd] 01 0b 00
                    ld        hl,$a112                      ;[00e0] 21 12 a1
                    ldir                                    ;[00e3] ed b0
                    ld        hl,($a14b)                    ;[00e5] 2a 4b a1
                    ld        de,$0050                      ;[00e8] 11 50 00
                    add       hl,de                         ;[00eb] 19
                    ex        de,hl                         ;[00ec] eb
                    ld        bc,$0020                      ;[00ed] 01 20 00
                    ld        hl,$a11f                      ;[00f0] 21 1f a1
                    ldir                                    ;[00f3] ed b0
                    call      $a0fc                         ;[00f5] cd fc a0
                    ld        hl,$0000                      ;[00f8] 21 00 00
                    ret                                     ;[00fb] c9

                    ld        hl,($a14d)                    ;[00fc] 2a 4d a1
                    ld        bc,$000c                      ;[00ff] 01 0c 00
                    add       hl,bc                         ;[0102] 09
                    xor       a                             ;[0103] af
                    ld        (hl),a                        ;[0104] 77
                    ld        hl,($a14d)                    ;[0105] 2a 4d a1
                    ld        de,$000c                      ;[0108] 11 0c 00
                    add       hl,de                         ;[010b] 19
                    ld        a,(hl)                        ;[010c] 7e
                    or        a                             ;[010d] b7
                    jr        z,$0105                       ;[010e] 28 f5
                    ret                                     ;[0110] c9

                    nop                                     ;[0111] 00
                    ld        c,b                           ;[0112] 48
                    ld        h,l                           ;[0113] 65
                    ld        l,h                           ;[0114] 6c
                    ld        l,h                           ;[0115] 6c
                    ld        l,a                           ;[0116] 6f
                    jr        nz,$0190                      ;[0117] 20 77
                    ld        l,a                           ;[0119] 6f
                    ld        (hl),d                        ;[011a] 72
                    ld        l,h                           ;[011b] 6c
                    ld        h,h                           ;[011c] 64
                    ld        hl,$5000                      ;[011d] 21 00 50
                    ld        (hl),d                        ;[0120] 72
                    ld        h,l                           ;[0121] 65
                    ld        (hl),e                        ;[0122] 73
                    ld        (hl),e                        ;[0123] 73
                    jr        nz,$0187                      ;[0124] 20 61
                    ld        l,(hl)                        ;[0126] 6e
                    ld        a,c                           ;[0127] 79
                    jr        nz,$0195                      ;[0128] 20 6b
                    ld        h,l                           ;[012a] 65
                    ld        a,c                           ;[012b] 79
                    jr        nz,$01a2                      ;[012c] 20 74
                    ld        l,a                           ;[012e] 6f
                    jr        nz,$0196                      ;[012f] 20 65
                    ld        a,b                           ;[0131] 78
                    ld        l,c                           ;[0132] 69
                    ld        (hl),h                        ;[0133] 74
                    jr        nz,$01a6                      ;[0134] 20 70
                    ld        (hl),d                        ;[0136] 72
                    ld        l,a                           ;[0137] 6f
                    ld        h,a                           ;[0138] 67
                    ld        (hl),d                        ;[0139] 72
                    ld        h,c                           ;[013a] 61
                    ld        l,l                           ;[013b] 6d
                    ld        l,$2e                         ;[013c] 2e 2e
                    ld        l,$00                         ;[013e] 2e 00
                    in        a,($a1)                       ;[0140] db a1
                    ld        d,l                           ;[0142] 55
                    and       c                             ;[0143] a1
                    nop                                     ;[0144] 00
                    nop                                     ;[0145] 00
                    ld        b,h                           ;[0146] 44
                    and       c                             ;[0147] a1
                    ld        bc,$0000                      ;[0148] 01 00 00
                    nop                                     ;[014b] 00
                    ld        d,b                           ;[014c] 50
                    nop                                     ;[014d] 00
                    ld        h,b                           ;[014e] 60
                    nop                                     ;[014f] 00
                    and       b                             ;[0150] a0
                    nop                                     ;[0151] 00
                    ret       po                            ;[0152] e0
                    nop                                     ;[0153] 00
                    nop                                     ;[0154] 00
                    nop                                     ;[0155] 00
                    nop                                     ;[0156] 00
                    nop                                     ;[0157] 00
                    nop                                     ;[0158] 00
                    nop                                     ;[0159] 00
                    nop                                     ;[015a] 00
                    nop                                     ;[015b] 00
                    nop                                     ;[015c] 00
                    nop                                     ;[015d] 00
                    nop                                     ;[015e] 00
                    nop                                     ;[015f] 00
                    nop                                     ;[0160] 00
                    nop                                     ;[0161] 00
                    nop                                     ;[0162] 00
                    nop                                     ;[0163] 00
                    nop                                     ;[0164] 00
                    nop                                     ;[0165] 00
                    nop                                     ;[0166] 00
                    nop                                     ;[0167] 00
                    nop                                     ;[0168] 00
                    nop                                     ;[0169] 00
                    nop                                     ;[016a] 00
                    nop                                     ;[016b] 00
                    nop                                     ;[016c] 00
                    nop                                     ;[016d] 00
                    nop                                     ;[016e] 00
                    nop                                     ;[016f] 00
                    nop                                     ;[0170] 00
                    nop                                     ;[0171] 00
                    nop                                     ;[0172] 00
                    nop                                     ;[0173] 00
                    nop                                     ;[0174] 00
                    nop                                     ;[0175] 00
                    nop                                     ;[0176] 00
                    nop                                     ;[0177] 00
                    nop                                     ;[0178] 00
                    nop                                     ;[0179] 00
                    nop                                     ;[017a] 00
                    nop                                     ;[017b] 00
                    nop                                     ;[017c] 00
                    nop                                     ;[017d] 00
                    nop                                     ;[017e] 00
                    nop                                     ;[017f] 00
                    nop                                     ;[0180] 00
                    nop                                     ;[0181] 00
                    nop                                     ;[0182] 00
                    nop                                     ;[0183] 00
                    nop                                     ;[0184] 00
                    nop                                     ;[0185] 00
                    nop                                     ;[0186] 00
                    nop                                     ;[0187] 00
                    nop                                     ;[0188] 00
                    nop                                     ;[0189] 00
                    nop                                     ;[018a] 00
                    nop                                     ;[018b] 00
                    nop                                     ;[018c] 00
                    nop                                     ;[018d] 00
                    nop                                     ;[018e] 00
                    nop                                     ;[018f] 00
                    nop                                     ;[0190] 00
                    nop                                     ;[0191] 00
                    nop                                     ;[0192] 00
                    nop                                     ;[0193] 00
                    nop                                     ;[0194] 00
                    nop                                     ;[0195] 00
                    nop                                     ;[0196] 00
                    nop                                     ;[0197] 00
                    nop                                     ;[0198] 00
                    nop                                     ;[0199] 00
                    nop                                     ;[019a] 00
                    nop                                     ;[019b] 00
                    nop                                     ;[019c] 00
                    nop                                     ;[019d] 00
                    nop                                     ;[019e] 00
                    nop                                     ;[019f] 00
                    nop                                     ;[01a0] 00
                    nop                                     ;[01a1] 00
                    nop                                     ;[01a2] 00
                    nop                                     ;[01a3] 00
                    nop                                     ;[01a4] 00
                    nop                                     ;[01a5] 00
                    nop                                     ;[01a6] 00
                    nop                                     ;[01a7] 00
                    nop                                     ;[01a8] 00
                    nop                                     ;[01a9] 00
                    nop                                     ;[01aa] 00
                    nop                                     ;[01ab] 00
                    nop                                     ;[01ac] 00
                    nop                                     ;[01ad] 00
                    nop                                     ;[01ae] 00
                    nop                                     ;[01af] 00
                    nop                                     ;[01b0] 00
                    nop                                     ;[01b1] 00
                    nop                                     ;[01b2] 00
                    nop                                     ;[01b3] 00
                    nop                                     ;[01b4] 00
                    nop                                     ;[01b5] 00
                    nop                                     ;[01b6] 00
                    nop                                     ;[01b7] 00
                    nop                                     ;[01b8] 00
                    nop                                     ;[01b9] 00
                    nop                                     ;[01ba] 00
                    nop                                     ;[01bb] 00
                    nop                                     ;[01bc] 00
                    nop                                     ;[01bd] 00
                    nop                                     ;[01be] 00
                    nop                                     ;[01bf] 00
                    nop                                     ;[01c0] 00
                    nop                                     ;[01c1] 00
                    nop                                     ;[01c2] 00
                    nop                                     ;[01c3] 00
                    nop                                     ;[01c4] 00
                    nop                                     ;[01c5] 00
                    nop                                     ;[01c6] 00
                    nop                                     ;[01c7] 00
                    nop                                     ;[01c8] 00
                    nop                                     ;[01c9] 00
                    nop                                     ;[01ca] 00
                    nop                                     ;[01cb] 00
                    nop                                     ;[01cc] 00
                    nop                                     ;[01cd] 00
                    nop                                     ;[01ce] 00
                    nop                                     ;[01cf] 00
                    nop                                     ;[01d0] 00
                    nop                                     ;[01d1] 00
                    nop                                     ;[01d2] 00
                    nop                                     ;[01d3] 00
                    nop                                     ;[01d4] 00
                    nop                                     ;[01d5] 00
                    nop                                     ;[01d6] 00
                    nop                                     ;[01d7] 00
                    nop                                     ;[01d8] 00
                    nop                                     ;[01d9] 00
                    nop                                     ;[01da] 00
