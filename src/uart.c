#include "lm3s6965.h"
#include "uart.h"
#include "ring_buffer.h"

static ring_buffer rx_buf;

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
    *UARTIM |= (1 << 4);
    *NVIC_EN0 |= (1 << 5);
}

void uart_putc(char c) {
    while ((*UARTFR & (1 << 5))) {}
    *UARTDR = c;
}

int uart_getc(void) {
    if (rb_is_empty(&rx_buf)) {
        return UART_NO_DATA;
    }
    char c = rb_pop(&rx_buf);
    return c;
}

void UART0_Handler(void) {
    char c = *UARTDR;
    rb_push(&rx_buf, c);
}
