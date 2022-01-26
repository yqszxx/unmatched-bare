//
// Created by yqszxx on 1/25/22.
//

#include <stdint.h>

#include "pwm.h"
#include "mmio.h"

#define CFG         (0x00)
#define EN_ALWAYS   (1 << 12)
#define CMP0        (0x20)
#define CMP1        (0x24)
#define CMP2        (0x28)
#define CMP3        (0x2C)
/*
 * Main clock is HFPCLK = 125.125MHz
 * PWM frequency = HFPCLK frequency / 2 ^ (16 + scale)
 * scale    freq / Hz
 *  0       1909.25
 *  1       954.6
 *  2       477.3
 *  3       238.65
 *  4       119.3
 *  5       59.7
 *  6       29.8
 *  7       14.9
 *  8       7.45
 *  9       3.73
 *  10      1.86
 *  11      0.93        1.073
 *  12      0.466       2.145
 *  13                  4.29
 *  14                  8.58
 *  15                  17.16
 */

void pwmTest() {
    uint8_t scale = 12;
    uint32_t cfg = scale | EN_ALWAYS;
    uint16_t cmp0 = 32768;

    mmioWrite32(PWM_0 + CFG, cfg);
    mmioWrite32(PWM_0 + CMP0, cmp0);
    mmioWrite32(PWM_0 + CMP1, 0xFFFF);
    mmioWrite32(PWM_0 + CMP2, 0xFFFF);
    mmioWrite32(PWM_0 + CMP3, 0xFFFF);
}