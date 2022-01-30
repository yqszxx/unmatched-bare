//
// Created by yqszxx on 1/25/22.
//

#ifndef __DRIVER_MMIO__
#define __DRIVER_MMIO__

#include <stdint.h>

#define PWM_0       ( 0x10020000ULL)
#define GPIO_0      ( 0x10060000ULL)
#define PCIE_MGMT   ( 0x100D0000ULL)
#define PCIE_CFG    ( 0x60070000ULL)
#define PCIE_CFG_SIZE   (0x10000ULL)
#define PCIE_IO     ( 0x60080000ULL)
#define PCIE_IO_SIZE    (0x10000ULL)
#define PCIE_DBI    (0xE00000000ULL)

static inline uint32_t mmioRead32(uint64_t address) {
    return *((volatile uint32_t*)address);
}

static inline void mmioWrite32(uint64_t address, uint32_t data) {
    *((volatile uint32_t*)address) = data;
}

static inline void mmioWrite16(uint64_t address, uint16_t data) {
    *((volatile uint16_t*)address) = data;
}

static inline void mmioClearSet32(uint64_t address, uint32_t clear, uint32_t set) {
    mmioWrite32(address, mmioRead32(address) & ~clear | set);
}

#endif //__DRIVER_MMIO__
