#ifndef _PORTS_H
#define _PORTS_H

#define BASEPORT        0x40

#define PORT_LED_IO     (BASEPORT | 0x04)
#define PORT_ADDR_LOW   (BASEPORT | 0x08)
#define PORT_ADDR_HIGH  (BASEPORT | 0x09)
#define PORT_ROM_BANK   (BASEPORT | 0x0A)
#define PORT_RAM_BANK   (BASEPORT | 0x0B)
#define PORT_ROM_IO     (BASEPORT | 0x0C)
#define PORT_RAM_IO     (BASEPORT | 0x0D)

#endif // _PORTS_H