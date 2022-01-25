//
// Created by yqszxx on 1/25/22.
//

#ifndef __DRIVER_MMIO__
#define __DRIVER_MMIO__

#include <stdint.h>

#define GPIO_0 (0x10060000UL)

static inline uint32_t mmioRead32(uint64_t address) {
    return *((volatile uint32_t*)address);
}

static inline void mmioWrite32(uint64_t address, uint32_t data) {
    *((volatile uint32_t*)address) = data;
}

#endif //__DRIVER_MMIO__
