#include "lm3s6965.h"
#include "uart.h"

void uart_init(void) {
    *RCGC1 |= (1 << 0);
    *RCGC2 |= (1 << 0);
    *GPIOAFSEL |= ((1 << 0) | (1 << 1));
    *GPIOA_DEN |= ((1 << 0) | (1 << 1));
    *UARTCTL = 0;
    *UARTIBRD = 6;
    *UARTFBRD = 33;
    *UARTLCRH |= ((0x3 << 5) | (1 << 4));
    *UARTCTL = ((1 << 8) | (1 << 9) | 1);
}

void uart_putc(char c) {
    while ((*UARTFR & (1 << 5))) {}
    *UARTDR = c;
}
