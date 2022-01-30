//
// Created by yqszxx on 1/26/22.
//

#ifndef __DRIVER_PCIE__
#define __DRIVER_PCIE__

#include <stdint.h>

void pcieInit();

uint32_t pcieCfgRead32(uint8_t b, uint8_t d, uint8_t f, uint8_t offset);
uint16_t pcieCfgRead16(uint8_t b, uint8_t d, uint8_t f, uint8_t offset);
uint8_t pcieCfgRead8(uint8_t b, uint8_t d, uint8_t f, uint8_t offset);

void pcieCfgWrite32(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint32_t data);
void pcieCfgWrite16(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint16_t data);
void pcieCfgWrite8(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint8_t data);

#endif //__DRIVER_PCIE__
