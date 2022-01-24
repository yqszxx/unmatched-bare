#include "bios.h"
#include "util/pr.h"

#define ATOM_ROM_TABLE_PTR	0x48

#define ATOM_ROM_MAGIC_PTR	4

#define ATOM_ROM_MSG_PTR	0x10

#define R8(i)  (vbios[i])
#define R16(i) (R8(i) | (R8((i) + 1) << 8))

uint8_t vbios[VBIOS_SIZE];

int read_bios() {
    for (int i = 0; i < VBIOS_SIZE; i++) {
        vbios[i] = *((volatile uint8_t *)VBIOS_ADDR + i);
    }
    
    ps("Copy done\r\n");

    if (R8(0) != 0x55 || R8(1) != 0xaa) {
        ps("Bad BIOS ROM header\r\n");
        return 0;
    }

    uint16_t rom_table = R16(ATOM_ROM_TABLE_PTR);

    ps(vbios + rom_table + ATOM_ROM_MAGIC_PTR);
    ps(" BIOS: ");

    char *str = (char *)(vbios + R16(rom_table + ATOM_ROM_MSG_PTR));
    char name[10];
    name[9] = '\0';

    while (*str && (*str == '\r' || *str == '\n')) str++;

    for (int i = 0; i < 9; i++) {
        name[i] = str[i];
        if (name[i] < '.' || name[i] > 'z') {
            name[i] = '\0';
            break;
        }
    }

    ps(name);
    ps("\r\n");

    return 1;
}
