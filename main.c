//#include "gpu/bios.h"
#include "util/pr.h"

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

void main() {
    while (UART[0]);

//    ps("Hello unmatched! @" __TIME__ "\r\n");

//    read_bios();
    pc(read_csr(mhartid) + '0');

    while (1);
}
