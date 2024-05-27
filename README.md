# P2000T SD-CARD cartridge

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/p2000t-sdcard?label=version)
[![build](https://github.com/ifilot/p2000t-sdcard/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/p2000t-sdcard/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![License: CC-BY-SA v4](https://img.shields.io/badge/license-CC--4.0--BY--SA-lightgrey)](https://creativecommons.org/licenses/by-sa/4.0/deed.en)

## Purpose

Perhaps the most elegant solution for loading CAS files into your P2000T is via
a cartridge that hosts an SD-card slot. Even the smallest SD-cards have enough
capacity to store the complete P2000T tape archive. Reading from such an SD-card
is non-trivial and requires both a hardware solution to interface with the I/O
port of the P2000T as well as a software solution to navigate through the files
hosted on a FAT32 partition. This repository contains the PCB for interfacing
a P2000T with an SD-card and the necessary software routines to grab .CAS files
from said SD card and load them into memory.

> [!WARNING] 
> Since version rev6 of the PCB, the I/O port has been changed to `0x40-0x4F`,
> while older versions of the PCB use I/O port `0x60-0x6F`. Depending on which
> version of the PCB you have, you need to use the right software packages.
> The different versions are indicated by `0x40` and `0x60`.

> [!TIP]
> **Documentation**
> * Detailed documentation included how-to procedures can be found [here](https://www.philips-p2000t.nl/cartridges/sdcard-cartridge.html#sdcard-cartridge)
> * There is also a [manual in Dutch](https://github.com/ifilot/P2000T-SD-kaart-handleiding) available.

## Downloads 

### 0x40 version

Compatible with PCB rev6 or newer.

> [!NOTE] 
> Do not forget to change the `LAUNCHER-0x40.BIN` to `LAUNCHER.BIN` when copying
> the file to the SD-card.

* [BASICBOOTSTRAP-0x40.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/BASICBOOTSTRAP-0x40)
* [FLASHER-0x40.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/FLASHER-0x40)
* [LAUNCHER-0x40.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/LAUNCHER-0x40)
* [PROGRAMS.ZIP](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/PROGRAMS.ZIP)

### 0x60 version

Compatible with PCB rev0-rev5.

> [!NOTE] 
> Do not forget to change the `LAUNCHER-0x60.BIN` to `LAUNCHER.BIN` when copying
> the file to the SD-card.

* [BASICBOOTSTRAP-0x60.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/BASICBOOTSTRAP-0x60.BIN)
* [FLASHER-0x60.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/FLASHER-0x60.BIN)
* [LAUNCHER-0x60.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/LAUNCHER-0x60.BIN)
* [PROGRAMS.ZIP](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/PROGRAMS.ZIP)

## Commands

Upon booting into the LAUNCHER application, the user interfaces with the SD-card
using a set of short commands. Below, the list of commands is provided

| **Command**         | **Description**                                                   |
| ------------------- | ------------------------------------------------------------------|
| `ls`                | List contents of current folder                                   |
| `lscas`             | List contents of current folder, listing contents of CAS files    |
| `cd <number>`       | Change directory                                                  |
| `run <number>`      | Run .CAS file                                                     |
| `hexdump <number>`  | Performs a 120-byte hexdump of a file                             |
| `fileinfo <number>` | Provides location details of a file                               |
| `ledtest`           | Performs a quick test on the read/write LEDs                      |
| `stack`             | Show current position of the stack pointer                        |
| `dump<XXXX>`        | Perform a 120-byte hexdump of main memory starting at `0xXXXX`    |
| `romdump<XXXX>`     | Perform a 120-byte hexdump of cartridge ROM starting at `0xXXXX`  |
| `ramdump<XXXX>`     | Perform a 120-byte hexdump of cartridge RAM starting at `0xXXXX`  |

Note that `<number>` needs to replaced with the specific number of a file. Users
who are familiar with command line interfaces are probably used to specifying
filenames rather than numbers. This reason this approach was chosen is mainly
because it is simpler to program and furthermore a bit quicker to type.

## Compilation

Compilation is done using the [z88dk Docker](https://hub.docker.com/r/z88dk/z88dk)
by means of `compile.sh`. To compile the launcher, run

```bash
cd src
./compile launcher
```

and for the flasher utility

```bash
cd src
./compile flasher
```

## Downloads

Binaries can directly downloaded from the releases.

* [BASICBOOTSTRAP.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/BASICBOOTSTRAP.BIN)
* [FLASHER.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/FLASHER.BIN)
* [LAUNCHER.BIN](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/LAUNCHER.BIN)
* [PROGRAMS.ZIP](https://github.com/ifilot/p2000t-sdcard/releases/latest/download/PROGRAMS.ZIP)

## Images

![P2000T SD-card PCB screenshots](img/p2000t-sdcard-pcb-rev4.jpg)

![P2000T SD-card cartridge set and monitor](img/p2000t-sdcard-cartridge-and-monitor.jpg)

## Contents

* [Cartridge cases](cases/)
* [PCB layout files (KiCAD)](pcb/port2-sdcard-interface/)
* [Modified BASIC ROM source files](basicmod/)
* [Launcher and Flasher programs](src/)
* [Custom programs](programs/)

## License

* All software is shared under the [GPL v3 license](https://www.gnu.org/licenses/gpl-3.0).
* All hardware (e.g. KiCAD files and .stl files) are shared under the [CC-BY-SA 4.0 license](https://creativecommons.org/licenses/by-sa/4.0/).
