#include <stdio.h>

#include "driver/pwm.h"
#include "driver/pcie.h"

int main() {
    printf("Hello!\r\n");
    pwmTest();
    pcieInit();
    return 0;
}
