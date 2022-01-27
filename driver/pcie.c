//
// Created by yqszxx on 1/26/22.
//

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "pcie.h"
#include "gpio.h"
#include "mmio.h"
#include "sleep.h"

#define PERSTN_PIN          (8)
#define PWREN_PIN           (5)

#define ASSERTION_DELAY     (  100)

#define MGMT_PERST_N        ( 0x00)
#define MGMT_HOLD_PHY_RST	( 0x18)

#define MGMT_LTSSM_ENABLE   ( 0x10)

#define MGMT_DEVICE_TYPE    (0x708)
#define DEVICE_TYPE_RC      ( 0x04)

#define MGMT_PHY0_ADDR      (0x860)
#define MGMT_PHY0_RD_EN     (0x870)
#define MGMT_PHY0_RD_DATA   (0x878)
#define MGMT_PHY0_SEL       (0x880)
#define MGMT_PHY0_WR_DATA   (0x888)
#define MGMT_PHY0_WR_EN     (0x890)
#define MGMT_PHY0_ACK       (0x898)
#define MGMT_PHY1_ADDR      (0x8a0)
#define MGMT_PHY1_RD_EN     (0x8b0)
#define MGMT_PHY1_RD_DATA   (0x8b8)
#define MGMT_PHY1_SEL       (0x8c0)
#define MGMT_PHY1_WR_DATA   (0x8c8)
#define MGMT_PHY1_WR_EN     (0x8d0)
#define MGMT_PHY1_ACK       (0x8d8)

#define PCI_BASE_ADDRESS_0  (0x10)
#define PCI_BASE_ADDRESS_1  (0x14)
#define PCI_BASE_ADDRESS_MEM_TYPE_64    (0x04)

#define PCI_INTERRUPT_LINE  (0x3c)

#define PCI_PRIMARY_BUS     (0x18)
//#define PCI_COMMAND         (0x04)
#define PCI_COMMAND_IO      (0x001)
#define PCI_COMMAND_MEMORY  (0x002)
#define PCI_COMMAND_MASTER  (0x004)
#define PCI_COMMAND_SERR    (0x100)

#define PCI_CLASS_DEVICE    (0x0a)
#define PCI_CLASS_BRIDGE_PCI    (0x0604)

#define PCIE_MISC_CONTROL_1 (0x8bc)
#define PCIE_DBI_RO_WR_EN   (1)

#define PCIE_LINK_WIDTH_SPEED_CONTROL   (0x80C)
#define PORT_LOGIC_SPEED_CHANGE (1 << 17)

#define PCIE_LINK_STATUS    (0x80)
#define PCIE_LINK_STATUS_SPEED_OFF  (16)
#define PCIE_LINK_STATUS_SPEED_MASK (0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF  (20)
#define PCIE_LINK_STATUS_WIDTH_MASK	(0xf << PCIE_LINK_STATUS_WIDTH_OFF)

#define PHY_DEBUG_R1    (0x72c)
#define PHY_DEBUG_R1_LINK_UP            (1 <<  4)
#define PHY_DEBUG_R1_LINK_IN_TRAINING   (1 << 29)

#define PF0_PCIE_CAP_LINK_CAP   (0x7C)
#define PCIE_LINK_CAP_MAX_SPEED_MASK    (0xf)

static void powerOnReset() {
    gpioSetMode(PERSTN_PIN, OUTPUT);
    gpioSetMode(PWREN_PIN, OUTPUT);

    // power down
    gpioWrite(PWREN_PIN, LOW);
    msleep(ASSERTION_DELAY);

    // assert reset
    gpioWrite(PERSTN_PIN, LOW);
    mmioWrite32(PCIE_MGMT + MGMT_PERST_N, 0);
    msleep(ASSERTION_DELAY);

    // power up
    gpioWrite(PWREN_PIN, HIGH);
    msleep(ASSERTION_DELAY);

    // deassert reset
    mmioWrite32(PCIE_MGMT + MGMT_PERST_N, 1);
    gpioWrite(PERSTN_PIN, HIGH);
    msleep(ASSERTION_DELAY);
}

static void setAuxClk(bool enable) {
#define PCIE_AUX_CFG    (0x10000014ULL)
    if (enable) {
        mmioClearSet32(PCIE_AUX_CFG, 0, 1);
    } else {
        mmioClearSet32(PCIE_AUX_CFG, 1, 0);
    }
#undef PCIE_AUX_CFG
}

static void deassertReset() {
#define DEVICES_RESET_N    (0x10000028ULL)
    // active low reset
    mmioClearSet32(DEVICES_RESET_N, 0, (1 << 4));
#undef PCIE_AUX_CFG
}

static void phyInit() {
    // enable PHY cr_para_sel interfaces
    mmioWrite32(PCIE_MGMT + MGMT_PHY0_SEL, 1);
    mmioWrite32(PCIE_MGMT + MGMT_PHY1_SEL, 1);
    msleep(1);

    // set PHY AC termination mode
    for (int lane = 0; lane < 8; lane++) {  // x8
        uint32_t laneRegAddress = 0x1008 + (0x0100 * lane);

        // PHY0
        mmioWrite32(PCIE_MGMT + MGMT_PHY0_ADDR, laneRegAddress);
        mmioWrite32(PCIE_MGMT + MGMT_PHY0_WR_DATA, 0x0e21);
        mmioWrite32(PCIE_MGMT + MGMT_PHY0_WR_EN, 1);
        while (!mmioRead32(PCIE_MGMT + MGMT_PHY0_ACK));
        msleep(1);
        mmioWrite32(PCIE_MGMT + MGMT_PHY0_WR_EN, 0);
        while (mmioRead32(PCIE_MGMT + MGMT_PHY0_ACK));

        // PHY1
        mmioWrite32(PCIE_MGMT + MGMT_PHY1_ADDR, laneRegAddress);
        mmioWrite32(PCIE_MGMT + MGMT_PHY1_WR_DATA, 0x0e21);
        mmioWrite32(PCIE_MGMT + MGMT_PHY1_WR_EN, 1);
        while (!mmioRead32(PCIE_MGMT + MGMT_PHY1_ACK));
        msleep(1);
        mmioWrite32(PCIE_MGMT + MGMT_PHY1_WR_EN, 0);
        while (mmioRead32(PCIE_MGMT + MGMT_PHY1_ACK));
    }
}

void rcInit() { // init root complex host bridge
    // setup RC BARs
    mmioWrite32(PCIE_DBI + PCI_BASE_ADDRESS_0, PCI_BASE_ADDRESS_MEM_TYPE_64);
    mmioWrite32(PCIE_DBI + PCI_BASE_ADDRESS_1, 0);

    // setup interrupt pins
    mmioClearSet32(PCIE_DBI + PCI_INTERRUPT_LINE, 0xff00, 0x100);

    // setup bus numbers
    mmioClearSet32(PCIE_DBI + PCI_PRIMARY_BUS, 0xffffff, 0x00ff0100);

    // setup command register
    mmioClearSet32(PCIE_DBI + PCI_PRIMARY_BUS,
                    0xffff,
                    PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
                    PCI_COMMAND_MASTER | PCI_COMMAND_SERR);

    // enable write permission for the DBI read-only register
    mmioClearSet32(PCIE_DBI + PCIE_MISC_CONTROL_1, 0, PCIE_DBI_RO_WR_EN);

    // program correct class for RC
    mmioWrite16(PCIE_DBI + PCI_CLASS_DEVICE, PCI_CLASS_BRIDGE_PCI);

    // disable write permission right after the update
    mmioClearSet32(PCIE_DBI + PCIE_MISC_CONTROL_1, PCIE_DBI_RO_WR_EN, 0);

    mmioClearSet32(PCIE_DBI + PCIE_LINK_WIDTH_SPEED_CONTROL, 0, PORT_LOGIC_SPEED_CHANGE);
}

bool isLinkup() {
    uint32_t val = mmioRead32(PCIE_DBI + PHY_DEBUG_R1);
    return (val & PHY_DEBUG_R1_LINK_UP) &&
           !(val & PHY_DEBUG_R1_LINK_IN_TRAINING);
}

void startLink() {
    if (isLinkup()) {
        puts("PCIe link is already up\r\n");
        return;
    }

    // force gen 1
    // enable writing
    mmioClearSet32(PCIE_DBI + PCIE_MISC_CONTROL_1, 0, PCIE_DBI_RO_WR_EN);
    // configure link cap FIXME: what's this?
    mmioClearSet32(PCIE_DBI + PF0_PCIE_CAP_LINK_CAP, 0, PCIE_LINK_CAP_MAX_SPEED_MASK);
    // disable writing
    mmioClearSet32(PCIE_DBI + PCIE_MISC_CONTROL_1, PCIE_DBI_RO_WR_EN, 0);

    // enable ltssm
    mmioClearSet32(PCIE_MGMT + MGMT_LTSSM_ENABLE, 0, 1);

    // wait for link up
    // wait for train
    msleep(20);

    uint64_t timeout = 20;

    do {
        msleep(1);
    } while (--timeout && !isLinkup());

    if (!isLinkup()) {
        // assert phy reset
        mmioWrite32(PCIE_MGMT + MGMT_HOLD_PHY_RST, 1);
        assert(isLinkup());
    }
}

uint8_t getLinkSpeed() {
    return (mmioRead32(PCIE_DBI + PCIE_LINK_STATUS) & PCIE_LINK_STATUS_SPEED_MASK) >> PCIE_LINK_STATUS_SPEED_OFF;
}

uint8_t getLinkWidth() {
    return (mmioRead32(PCIE_DBI + PCIE_LINK_STATUS) & PCIE_LINK_STATUS_WIDTH_MASK) >> PCIE_LINK_STATUS_WIDTH_OFF;
}

void pcieInit() {
    powerOnReset();

    setAuxClk(true);

    // assert hold_phy_rst (hold the controller LTSSM in reset
    // after power_up_rst_n for register programming with cr_para)
    mmioWrite32(PCIE_MGMT + MGMT_HOLD_PHY_RST, 1);

    deassertReset();

    phyInit();

    setAuxClk(false);
    // deassert hold_phy_rst
    mmioWrite32(PCIE_MGMT + MGMT_HOLD_PHY_RST, 0);
    setAuxClk(true);

    // set desired mode while core is not operational
    mmioWrite32(PCIE_MGMT + MGMT_DEVICE_TYPE, DEVICE_TYPE_RC);
    // confirm desired mode from operational core
    assert(mmioRead32(PCIE_MGMT + MGMT_DEVICE_TYPE) == DEVICE_TYPE_RC);

    rcInit();

    startLink();

    printf("PCIe: Link up, Gen%d, x%d\r\n", getLinkSpeed(), getLinkWidth());
}