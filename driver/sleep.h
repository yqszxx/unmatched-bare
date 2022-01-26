//
// Created by yqszxx on 1/26/22.
//

#ifndef __DRIVER_SLEEP__
#define __DRIVER_SLEEP__

#include <stdint.h>

static inline void usleep(uint64_t microSeconds) {
#define MTIME           (*(volatile uint64_t *)0x0200BFF8)
#define RTC_PERIOD_US   (1ULL)

    uint64_t currentTime = MTIME;
    uint64_t endTime = currentTime + microSeconds / RTC_PERIOD_US + 1;

    do {
        currentTime = MTIME;
    } while (currentTime < endTime);

#undef MTIME
#undef RTC_PERIOD_NS
}

static inline void msleep(uint64_t milliSeconds) {
    usleep(milliSeconds * 1000ULL);
}

static inline void sleep(uint64_t seconds) {
    msleep(seconds * 1000ULL);
}

#endif //__DRIVER_SLEEP__
