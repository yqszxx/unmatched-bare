//
// Created by yqszxx on 1/25/22.
//

#ifndef __DRIVER_GPIO__
#define __DRIVER_GPIO__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    INPUT,
    OUTPUT
} GpioMode;

typedef enum {
    LOW = 0,
    HIGH = 1,
} GpioState;

void gpioSetMode(uint8_t pin, GpioMode mode);

GpioState gpioRead(uint8_t pin);

void gpioWrite(uint8_t pin, GpioState state);

#endif //__DRIVER_GPIO__
