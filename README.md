# P2000T SD-CARD cartridge

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/p2000t-sdcard?label=version)
[![build](https://github.com/ifilot/p2000t-sdcard/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/p2000t-sdcard/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![License: CC-BY-SA v4](https://img.shields.io/badge/license-CC--4.0--BY--SA-lightgrey)](https://creativecommons.org/licenses/by-sa/4.0/deed.en)
[![discord](https://img.shields.io/discord/1167925678868541451?logo=discord&logoColor=white)](https://discord.gg/YtzJTWYAxy)

## Table of contents

* [Purpose](#purpose)
* [Downloads](#downloads)
* [How it functions](#how-it-functions)
* [Images](#images)
* [Commands](#commands)
* [Compilation instructions](#compilation-instructions)
* [Repository contents](#repository-contents)
* [Building the hardware](#building-the-hardware)
* [Ordering a cartridge](#ordering-a-cartridge)
* [Internal SD-card board](#internal-sd-card-board)
* [Formatting SD-card](#formatting-sd-card)
* [License](#license)

> [!TIP] 
> Have questions or comments about the **P2000T SD-card cartridge**? Join
> the conversation and chat with the developers on our [Discord
> server](https://discord.gg/YtzJTWYAxy).

## YouTube

Check out the YouTube video below for a detailed review of the SD-card cartridge.

[![YouTube](http://i.ytimg.com/vi/OKvVlIvrEu0/hqdefault.jpg)](https://www.youtube.com/watch?v=OKvVlIvrEu0)

## Purpose

Perhaps the most elegant solution for loading CAS files into your P2000T or
making backups of your tapes is via a cartridge that hosts an SD-card slot. Even
the smallest SD-cards have enough capacity to store the complete P2000T tape
archive. Reading from such an SD-card is non-trivial and requires both a
hardware solution to interface with the I/O port of the P2000T as well as a
software solution to navigate through the files hosted on a FAT32 partition.
This repository contains the PCB for interfacing a P2000T with an SD-card and
the necessary software routines to grab .CAS files from said SD card and load
them into memory.

> [!TIP]
> **Documentation**
> * Detailed documentation included how-to procedures can be found [here](https://www.philips-p2000t.nl/cartridges/sdcard-cartridge.html#sdcard-cartridge)
> * There is also a [manual in Dutch](https://github.com/ifilot/P2000T-SD-kaart-handleiding) ![Dutch flag](img/nl_flag.png) available.

## Downloads 

Read the section ["How it functions"](#how-it-functions) to understand what each
download file does.

> [!WARNING] 
> The files shown here cater specifically to PCB version 6 or later
> where the SD-card listens to I/O port 0x4X. Earlier version of the PCB are no
> longer actively maintained.

* [BASICBOOTSTRAP.BIN](https://github.com/ifilot/p2000t-sdcard/releases/download/latest-stable/BASICBOOTSTRAP.BIN)
* [FLASHER.BIN](https://github.com/ifilot/p2000t-sdcard/releases/download/latest-stable/FLASHER.BIN)
* [LAUNCHER.BIN](https://github.com/ifilot/p2000t-sdcard/releases/download/latest-stable/LAUNCHER.BIN)

> [!TIP]
> * Precompiled SD-card images, including flashing instructions, can be found [here](https://github.com/ifilot/p2000t-sdcard-image)
> * A list of PRG programs, specifically designed to operate with the SD-card cartridge, can be found [here](https://github.com/ifilot/p2000t-sdcard-cartridge-programs)

## How it functions

To interface with the SD-card, an SD-card slot is connected to the I/O port of
the P2000T. Interfacing with the SD-card proceeds via the SPI protocol which is
encoded in hardware (in contrast to bit-banging the protocol in software). From
the perspective of the P2000T, simple 1-byte instructions can be sent and
retrieved from the SD-card. By sending specific commands, data is sent or
retrieved from the SD-card.

While the SLOT2 cartridge (which hooks up to the I/O port of the P2000T) hosts
the circuitry to interface with the SD-card, a SLOT1 cartridge is required that
hosts the software. In order to run BASIC programs that are stored on the
SD-card as `.CAS` files, the regular BASIC environment **and** the required
routines to interface with the SD-card need to be present in memory. For this
reason, a modified version of the BASIC environment is used which upon boot will
load a launcher application from the ROM chip on the SLOT2 cartridge into main
memory. This launcher application allows the user to navigate the SD-card and
load BASIC programs into memory and run these. Detailed instructions on this
process are found [here](https://www.philips-p2000t.nl/cartridges/sdcard-cartridge.html#launching-cas-programs-from-the-sd-card-cartridge).

The launcher application needs to be placed on the ROM chip by means of a
flasher program. The flasher program is a different SLOT1 cartridge which can be
downloaded from the [download section](#downloads). The flasher program will
copy a `LAUNCHER.BIN` file from the root folder of the SD-card to the ROM chip
on the SLOT2 cartridge. This procedure is also used to update the launcher
application. Details on this process can be found [here](https://www.philips-p2000t.nl/cartridges/sdcard-cartridge.html#flashing-the-launcher-onto-the-rom-chip).

It is also possible to make copies of existing tapes to the SD-card. For this
purpose, a `CASDUMP.PRG` program is written. These special types of programs can
be loaded from the SD-card and executes via the launcher application. After
completion, one automatically returns to the launcher, allowing one to load one
of the freshly copied programs.

## Images

SLOT2 cartridge enclosure and populated PCB.
![Image of cartridge and PCB](img/p2000t-sdcard-cartridge.jpg)

Screenshots of navigation and execution of .CAS files.
![Image of cartridge and PCB](img/p2000t-sdcard-interface.jpg)

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

## Compilation instructions

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

## Repository contents

* [Cartridge cases](cases/)
* [PCB layout files (KiCAD)](pcb/port2-sdcard-interface/)
* [Modified BASIC ROM source files](basicmod/)
* [Launcher and Flasher programs](src/)
* [Custom programs](programs/)

## Building the hardware

### Gerber files

To create your own SLOT2 cartridge, download the GERBER files from
[here](pcb/port2-sdcard-interface/GERBERS/), put them in a `.ZIP` file and send
them to your favorite PCB print farm. The author of this repository mainly uses
[JLCPCB](https://jlcpcb.com/), but plenty of alternatives are available. The
SLOT2 PCB has four layers. It is recommended to apply an ENIG (gold) coating and
chamfer the edge connector for facile insertion into the P2000T. After ordering,
one needs to either hand-solder the components (which can be tricky for the SMD
chips) or alternatively make use of an assembly service by which the PCB print
farm will solder these components for you.

### PCB assembly service

The folder [pcb/port2-sdcard-interface](pcb/port2-sdcard-interface) contains
`port2-sdcard-interface_bom_bot.csv` and `port2-sdcard-interface-bottom-pos.csv`
which can be used for the JLCPCB assembly service. An example of the part
placement can be found in the image below. At the time of writing, two
components were unavailable, being the memory chip `62128` and a 0 ohm resistor,
which is the reason that these two parts are missing. Note that all through-hole
parts, e.g. LEDs, oscillator and PLCC32 socket, have to be hand-soldered in.

![img PCB assembly](img/sdcard-pcb-part-placement.png)

### Ordering a cartridge

It is also possible to order a cartridge (typically there are always a few
of them in stock). If you are interested, feel free to contact the author of
this repository at `ivo at ivofilot dot nl` to discuss pricing and shipping.

## Internal SD-card board

This project also includes an [internal SD-card
board](pcb/internal-sdcard-interface/) designed to mount on top of the main
baseboard inside the P2000T. In addition to the SD-card reader functionality,
the board also provides 64 KiB of additional memory, making it an all-in-one
solution.

To install this board, you will need two custom brackets. You can find the
necessary bracket files [here](brackets).

![internal SD-card board](img/pcb-design-internal-board.jpg)

## Formatting SD-card

The way that the SD-card is formatted is critical. The SD-card needs be
formatted using a MBR/FAT32 partition. Often, directly formatting the SD-card
via Windows Explorer or similar works well, but sometimes a more rigorous
procedure is needed, which is explained below.

1. Type `WINDOWS + R` and run `CMD`.
2. Run `diskpart`.
3. Type `list disk` followed by `select disk <NR>` where `<NR>` corresponds to
   your SD-card.
4. Type `clean`. You get a response with `DiskPart succeeded in cleaning the
   disk.`.
5. Type `list volume`, followed by `select volume <ID>` where `<ID>` corresponds
   to your SD-card drive.
6. Type `convert mbr`. You should get the response `DiskPart successfully
   converted the selected disk to MBR format.`.
7. Type `create partition primary`. The response should be `DiskPart succeeded
   in creating the specified partition.`.
8. Finally, type `format fs=fat32 quick label="P2000T"`, after which the
   response is `DiskPart successfully formatted the volume.`.

Your SD-card should now be ready to work in the SD-cartridge. Of course, you
still need to copy files to it in order to load something of it.

## License

![License facts](img/oshw_facts.svg)

* All software is shared under the [GPL v3 license](https://www.gnu.org/licenses/gpl-3.0).
* All hardware (e.g. KiCAD files and .stl files) are shared under the [CERN-OHL-S license](https://ohwr.org/project/cernohl/-/wikis/uploads/819d71bea3458f71fba6cf4fb0f2de6b/cern_ohl_s_v2.txt).
* All documentation is shared under the [CC-BY-SA 4.0 license](https://creativecommons.org/licenses/by-sa/4.0/).
