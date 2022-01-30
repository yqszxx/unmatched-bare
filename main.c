#include <stdio.h>

#include "driver/pwm.h"
#include "driver/pcie.h"

#define LOG_PCIE_CFG_READ(b, d, f, o) printf("(" #b "," #d "," #f "," #o ")=0x%08X\r\n", pcieCfgRead32((b), (d), (f), (o)))

int main() {
    printf("Hello!\r\n");
    pwmTest();
    pcieInit();
    LOG_PCIE_CFG_READ(0, 0, 0, 0);
    LOG_PCIE_CFG_READ(0, 1, 0, 0);
    LOG_PCIE_CFG_READ(1, 0, 0, 0);
    return 0;
}
