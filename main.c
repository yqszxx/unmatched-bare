#include <stdio.h>

#include "driver/pwm.h"
#include "driver/pcie.h"

#define LOG_PCIE_CFG_READ32(b, d, f, o) printf("(" #b "," #d "," #f "," #o ")=0x%08X\r\n", pcieCfgRead32((b), (d), (f), (o)))
#define LOG_PCIE_CFG_READ16(b, d, f, o) printf("(" #b "," #d "," #f "," #o ")=0x%04X\r\n", pcieCfgRead16((b), (d), (f), (o)))
#define LOG_PCIE_CFG_READ8(b, d, f, o) printf("(" #b "," #d "," #f "," #o ")=0x%02X\r\n", pcieCfgRead8((b), (d), (f), (o)))
#define LOG_PCIE_CFG_WRITE8(b, d, f, o, dt) do { printf("(" #b "," #d "," #f "," #o ")<=0x%02X\r\n", dt); pcieCfgWrite8((b), (d), (f), (o), (dt)); } while (0)

int main() {
    printf("Hello!\r\n");
    pwmTest();
    pcieInit();
    pcieEnum();
    return 0;
}

void sw_irq_handler() {
    uint64_t mcause, mepc, mtval;
    asm volatile ("csrr %0, mcause" : "=r"(mcause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));
    asm volatile ("csrr %0, mtval" : "=r"(mtval));

    printf("!!%016lX %016lX %016lX!!\r\n", mcause, mepc, mtval);

    while (1) {
        asm volatile ("wfi");
    }
}