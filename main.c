#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint64_t readMcycle() {
    unsigned long mcycle;
    asm volatile ("csrr %0, mcycle" : "=r"(mcycle));
    return mcycle;
}

int main() {
    printf("Hello Newlib!\r\n");
    printf("Current mcycle: %lu\r\n", readMcycle());
    void* mem = malloc(128);
    printf("malloc: 0x%016lX\r\n", (uint64_t)mem);
    free(mem);
    return 0;
}
