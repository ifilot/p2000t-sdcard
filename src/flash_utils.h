#ifndef _FLASH_UTILS_H
#define _FLASH_UTILS_H

uint8_t flash_rom(uint32_t faddr);
uint8_t store_file_rom(uint32_t faddr, uint16_t rom_addr);

#endif // _FLASH_UTILS_H