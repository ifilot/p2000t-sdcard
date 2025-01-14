# Bootstrapping the Launcher app from BASIC

By injecting two small pieces of custom code into the BASIC ROM, we are able to
load the Launcher app from SLOT2 ROM via the I/O port and then start it up. 
After the user has selected a P2000T .cas progam to run, the Launcher will
return to the custom bootstrap code, which will then load the P2000T program 
from SLOT2 RAM via the I/O port into P2000T RAM and then start it up.

## Injection mechanism

### 1. Injecting custom bootstap code for the Launcher
Our custom bootstap code is injected at the end of BASIC ROM at position $4EC7
(see hackrom.py).

### 2. Injecting an intercept/hook in BASIC's interpreter loop
To run our custom bootstrap code directly after BASIC startup, we need to place
an intercept/hook at address 0x60D3 (see 'Adresboekje' page 18). 
Note that the initial values for P2000T's addresses 0x6090-0x60EA are copied
from a ROM-table located at addresses 0x18A4-0x18FF during BASIC's startup 
(see 'Adresboekje' page 13). So this means that we can simply inject our hook 
at position 0x08E7 in the BASIC ROM (see hackrom.py).

This intercept/hook is cleared right at the start of our custom bootstrap code, 
so it won't get in the way when loading and running BASIC programs.

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