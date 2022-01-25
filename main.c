#include <stdio.h>
#include <stdint.h>

uint64_t readMcycle() {
    unsigned long mcycle;
    asm volatile ("csrr %0, mcycle" : "=r"(mcycle));
    return mcycle;
}

int main() {
    printf("Hello Newlib!\r\n");
    printf("Current mcycle: %lu\r\n", readMcycle());
    return 0;
}
