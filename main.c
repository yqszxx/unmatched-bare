#include <stdio.h>

#include "driver/gpio.h"

int main() {
    gpioSetMode(0, INPUT);
    printf("GPIO 0: %c\r\n", gpioRead(0) == HIGH ? 'H' : 'L');
    return 0;
}
