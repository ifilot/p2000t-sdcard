# DEMO - IOLOADER

This example shows how via a small piece of injected code, another
piece of code can be loaded from the I/O port.

## Mechanism

To insert the custom code in the system RAM, two hijacks are made. The first
one is done in the startup procedure. Normally, at 0x1F71, a call is made to 
address 0x1056 which contains a jump to address 0x1F5A. This call is hijacked 
by changing the address pointer at 0x1057-0x1058 to 0x4EE0, which is an empty 
part of the BASIC rom. At this point, custom code is inserted which at its
end makes the required jump to 0x1F5A, from which the initiation procedure 
continues.

The small piece of code that is inserted at 0x4EE0 stores the value 0x55
at memory address 0x6150 (which is free to use). When this value is present,
another hijacked memory routine is activated. This routine is rather small
and resides at 0x4CE7h. Here, a check is made whether memory address 0x6150
contains the value 0x55 and if so, a jump is made to 0x6151 where the bootloader
can place a custom pointer to any desired piece of code to be executed. When
desired, this custom piece of code can always return with a `jp 0029h` to
return to the BASIC execution (assuming no drastic changes are made to the
BASIC infrastructure).

## Output

* `BASICBOOTSTRAP.bin`: Modified BASIC cartridge

## Compilation instructions

Prepare the Docker compiler image

```bash
docker build . -t basicmod-compiler
```

Use the Docker image for the compilation

```bash
./compile
```