#include "lm3s6965.h"
#include "systick.h"
#include "uart.h"

int main(void) {
    *RCGC2 |= (1 << 5);
    *GPIODIR |= (1 << 0);
    *GPIOF_DEN |= (1 << 0);
    *GPIODATA |= (1 << 0);
    uart_init();
    systick_init();
    while (1) {
        if (systick_count >= 1000) {
            uart_putc('a');
            systick_count = 0;
        }
    }
    return 0;
}
