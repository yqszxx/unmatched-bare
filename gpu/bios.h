#ifndef _GPU_BIOS_
#define _GPU_BIOS_

#include <stdint.h>

#define VBIOS_ADDR (0x70420000)
#define VBIOS_SIZE (128 * 1024)

extern uint8_t vbios[VBIOS_SIZE];

int read_bios();

#endif // _GPU_BIOS_