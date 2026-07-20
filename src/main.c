#include "lm3s6965.h"
#include "mini_printf.h"
#include "systick.h"
#include "uart.h"
#include "mini_strtok.h"

#define MAXSIZE 1024

int main(void) {
    char buffer[MAXSIZE];
    int index = -1;
    int c;
    char *token = 0;
    // char *error = {""};
    *RCGC2 |= (1 << 5);
    *GPIODIR |= (1 << 0);
    *GPIOF_DEN |= (1 << 0);
    *GPIODATA |= (1 << 0);
    uart_init();
    systick_init();
    while (1) {
        while ((c = uart_getc()) != '\r' || c == UART_NO_DATA) {
            if (index < (MAXSIZE - 2) && c != UART_NO_DATA) {
                uart_putc(c);
                index++;
                buffer[index] = c;
            }
        }
        // if (index >= MAXSIZE) {
        // }
        index++;
        buffer[index] = '\0';

        token = mini_strtok(buffer);
        while (token != 0) {
            mini_printf("%s\r\n", token);

            token = mini_strtok(0);
        }
        index = -1;
    }
    return 0;
}
