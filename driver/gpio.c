//
// Created by yqszxx on 1/25/22.
//

#include "gpio.h"
#include "mmio.h"

#define INPUT_VAL   (0x00)
#define INPUT_EN    (0x04)
#define OUTPUT_EN   (0x08)

void gpioSetMode(uint8_t pin, GpioMode mode) {
    uint32_t inputEn  = mmioRead32(GPIO_0 +  INPUT_EN);
    uint32_t outputEn = mmioRead32(GPIO_0 + OUTPUT_EN);
    uint32_t pinMask = 1 << pin;

    switch (mode) {
        case INPUT:
            inputEn  |=  pinMask;
            outputEn &= ~pinMask;
            break;
        case OUTPUT:
            inputEn  &= ~pinMask;
            outputEn |=  pinMask;
            break;
    }

    mmioWrite32(GPIO_0 +  INPUT_EN, inputEn);
    mmioWrite32(GPIO_0 + OUTPUT_EN, outputEn);
}

GpioState gpioRead(uint8_t pin) {
    uint32_t inputVal = mmioRead32(GPIO_0 + INPUT_VAL);
    uint32_t pinMask = 1 << pin;

    if ((inputVal & pinMask) == pinMask) {
        return HIGH;
    } else {
        return LOW;
    }
}
