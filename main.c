#include <assert.h>
#include <stdio.h>

#include "driver/pwm.h"
#include "driver/sleep.h"

int main() {
    printf("Hello!\r\n");
    pwmTest();
    printf("sleep 5s\r\n");
    sleep(5);
    printf("wake\r\n");
    assert(1 == 2);
    printf("Hello?\r\n");
    return 0;
}
