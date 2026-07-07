#include "lm3s6965.h"
#include "uart.h"

int main(void) {
    int counter = 0;
    *RCGC2 |= (1 << 5);
    *GPIODIR |= (1 << 0);
    *GPIOF_DEN |= (1 << 0);
    *GPIODATA |= (1 << 0);
    uart_init();
    uart_putc('H');
    uart_putc('i');
    uart_putc('\n');
    while (1) {
        counter++;
    }
    return 0;
}
