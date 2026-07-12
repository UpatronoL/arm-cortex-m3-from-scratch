#include "lm3s6965.h"
#include "mini_printf.h"
#include "systick.h"
#include "uart.h"

int main(void) {
    *RCGC2 |= (1 << 5);
    *GPIODIR |= (1 << 0);
    *GPIOF_DEN |= (1 << 0);
    *GPIODATA |= (1 << 0);
    uart_init();
    systick_init();
    mini_printf("please start typing: \n");
    while (1) {
        int c = uart_getc();
        if(c != UART_NO_DATA) { 
            uart_putc(c);
        } 
    }
    return 0;
}
