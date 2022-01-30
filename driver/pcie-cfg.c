//
// Created by yqszxx on 1/29/22.
//

#include <assert.h>
#include <stdbool.h>

#include "pcie.h"
#include "mmio.h"
#include "sleep.h"

#define PCIE_DBI_ATU_OFFSET     (0x300000)
#define PCIE_ATU_REGION_OFFSET  (0x200)

#define PCIE_ATU_REGION_CTRL1   (0x00)
#define PCIE_ATU_REGION_CTRL2   (0x04)
#define PCIE_ATU_ENABLE     (0x1 << 31)
#define PCIE_ATU_LOWER_BASE     (0x08)
#define PCIE_ATU_UPPER_BASE     (0x0C)
#define PCIE_ATU_LIMIT          (0x10)
#define PCIE_ATU_LOWER_TARGET   (0x14)
#define PCIE_ATU_UPPER_TARGET   (0x18)

#define PCIE_TYPE_IO    (0x2)
#define PCIE_TYPE_CFG0  (0x4)
#define PCIE_TYPE_CFG1  (0x5)

void pcieProgramATU(int index, uint64_t cpuAddr, uint64_t pcieAddr, uint32_t size, uint8_t type) {
    assert(index == 0 || index == 1);

    uint64_t atuBaseAddr = PCIE_DBI + PCIE_DBI_ATU_OFFSET + PCIE_ATU_REGION_OFFSET * index;

    // base
    mmioWrite32(atuBaseAddr + PCIE_ATU_LOWER_BASE, cpuAddr & 0xFFFFFFFF);  // lower
    mmioWrite32(atuBaseAddr + PCIE_ATU_UPPER_BASE, (cpuAddr >> 32) & 0xFFFFFFFF);  // upper

    // limit
    mmioWrite32(atuBaseAddr + PCIE_ATU_LIMIT, (cpuAddr + size - 1) & 0xFFFFFFFF);

    // target
    mmioWrite32(atuBaseAddr + PCIE_ATU_LOWER_TARGET, pcieAddr & 0xFFFFFFFF); // lower
    mmioWrite32(atuBaseAddr + PCIE_ATU_UPPER_TARGET, (pcieAddr >> 32) & 0xFFFFFFFF); // upper

    // type
    mmioWrite32(atuBaseAddr + PCIE_ATU_REGION_CTRL1, type);

    // enable
    mmioWrite32(atuBaseAddr + PCIE_ATU_REGION_CTRL2, PCIE_ATU_ENABLE);

    for (int retries = 0; retries < 5; retries++) { // check if atu is enabled in 50ms
        if ((mmioRead32(atuBaseAddr + PCIE_ATU_REGION_CTRL2) & PCIE_ATU_ENABLE) == PCIE_ATU_ENABLE) {
            return;
        }
        msleep(10);
    }

    assert(false && "ATU program timed out");
}

uint32_t pcieCfgRead32(uint8_t b, uint8_t d, uint8_t f, uint8_t offset) {
    assert(d < 32);
    assert(f < 8);
    assert((offset & 0x3) == 0);    // 32 bit aligned

    // construct bdf address, pcie-spec-1.0, fig 2-12 (p68)
    uint32_t bdf = (b << 24) | (d << 19) | (f << 16);

    if (b == 0) {   // bus 0 (bus inside rc)
        if (d == 0) {
            // access using dbi
            return mmioRead32(PCIE_DBI + offset);
        } else {
            // bus 0 contains only one device
            return 0xFFFFFFFFUL;
        }
    } else if (b == 1) { // bus 1 (bus between rc and outside)
        if (d == 0) {
            // on local bus, TLP header type is CFG0
            pcieProgramATU(1, PCIE_CFG, bdf, PCIE_CFG_SIZE, PCIE_TYPE_CFG0);

            uint32_t val = mmioRead32(PCIE_CFG + offset);

            // restore io space mapping
            pcieProgramATU(1, PCIE_IO, PCIE_IO, PCIE_IO_SIZE, PCIE_TYPE_IO);

            return val;
        } else {
            // bus 1 contains only one device
            return 0xFFFFFFFFUL;
        }
    } else {
        // on local bus, TLP header type is CFG0
        pcieProgramATU(1, PCIE_CFG, bdf, PCIE_CFG_SIZE, PCIE_TYPE_CFG1);

        uint32_t val = mmioRead32(PCIE_CFG + offset);

        // restore io space mapping
        pcieProgramATU(1, PCIE_IO, PCIE_IO, PCIE_IO_SIZE, PCIE_TYPE_IO);

        return val;
    }
}

uint16_t pcieCfgRead16(uint8_t b, uint8_t d, uint8_t f, uint8_t offset) {
    assert(d < 32);
    assert(f < 8);
    assert((offset & 0x1) == 0);    // 16 bit aligned

    uint32_t val = pcieCfgRead32(b, d, f, offset & (~0x3));

    switch (offset & 0x1) {
        case 0:
            return (val & 0xFFFF);
        case 1:
            return ((val >> 16) & 0xFFFF);
    }
}

uint8_t pcieCfgRead8(uint8_t b, uint8_t d, uint8_t f, uint8_t offset) {
    assert(d < 32);
    assert(f < 8);

    uint32_t val = pcieCfgRead32(b, d, f, offset & (~0x3));

    switch (offset & 0x3) {
        case 0:
            return (val & 0xFF);
        case 1:
            return ((val >> 8) & 0xFF);
        case 2:
            return ((val >> 16) & 0xFF);
        case 3:
            return ((val >> 24) & 0xFF);
    }
}

typedef enum {
    WRITE32,
    WRITE16,
    WRITE8,
} WriteWidth;

static uint32_t updateVal(uint32_t old, uint32_t value, uint8_t offset, WriteWidth width) {
    uint32_t offsetMask;
    uint32_t valueMask;

    switch (width) {
        case WRITE8:
            offsetMask = 0x3;
            valueMask = 0xFF;
            break;
        case WRITE16:
            offsetMask = 0x2;
            valueMask = 0xFFFF;
            break;
        default:
            return value;
    }

    int shift = (offset & offsetMask) * 8;

    return (old & ~(valueMask << shift)) | ((value & valueMask) << shift);
}

static void pcieCfgWrite(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint32_t data, WriteWidth width) {
    assert(d < 32);
    assert(f < 8);
    assert((offset & 0x3) == 0);    // 32 bit aligned

    // construct bdf address, pcie-spec-1.0, fig 2-12 (p68)
    uint32_t bdf = (b << 24) | (d << 19) | (f << 16);

    if (b == 0) {   // bus 0 (bus inside rc)
        if (d == 0) {
            // access using dbi
            uint32_t val = mmioRead32(PCIE_DBI + offset);
            val = updateVal(val, data, offset, width);
            mmioWrite32(PCIE_DBI + offset, val);
        } else {
            // bus 0 contains only one device
            // do nothing
        }
    } else if (b == 1) { // bus 1 (bus between rc and outside)
        if (d == 0) {
            // on local bus, TLP header type is CFG0
            pcieProgramATU(1, PCIE_CFG, bdf, PCIE_CFG_SIZE, PCIE_TYPE_CFG0);

            uint32_t val = mmioRead32(PCIE_CFG + offset);
            val = updateVal(val, data, offset, width);
            mmioWrite32(PCIE_CFG + offset, val);

            // restore io space mapping
            pcieProgramATU(1, PCIE_IO, PCIE_IO, PCIE_IO_SIZE, PCIE_TYPE_IO);
        } else {
            // bus 1 contains only one device
            // do nothing
        }
    } else {
        // on local bus, TLP header type is CFG0
        pcieProgramATU(1, PCIE_CFG, bdf, PCIE_CFG_SIZE, PCIE_TYPE_CFG1);

        uint32_t val = mmioRead32(PCIE_CFG + offset);
        val = updateVal(val, data, offset, width);
        mmioWrite32(PCIE_CFG + offset, val);

        // restore io space mapping
        pcieProgramATU(1, PCIE_IO, PCIE_IO, PCIE_IO_SIZE, PCIE_TYPE_IO);
    }
}

void pcieCfgWrite32(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint32_t data) {
    pcieCfgWrite(b, d, f, offset, data, WRITE32);
}

void pcieCfgWrite16(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint16_t data) {
    pcieCfgWrite(b, d, f, offset, data, WRITE16);
}

void pcieCfgWrite8(uint8_t b, uint8_t d, uint8_t f, uint8_t offset, uint8_t data) {
    pcieCfgWrite(b, d, f, offset, data, WRITE8);
}
