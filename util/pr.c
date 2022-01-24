#include "pr.h"

void pc(char c) {
    UART[0] = c;
    while (UART[0]);
}

void ps(char* s) {
    for (char c = *s; c = *s; s++) {
        pc(c);
    }
}
