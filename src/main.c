#include "lm3s6965.h"
#include "mini_printf.h"
#include "mini_strtok.h"
#include "mini_strcmp.h"
#include "systick.h"
#include "uart.h"
#include "shell.h"

#define MAXSIZE 1024

int main(void) {
    char buffer[MAXSIZE];
    int index = -1;
    int c, i;
    char *token = 0;
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
        uart_putc('\r');

        token = mini_strtok(buffer);
        if (token != 0) {
            for (i = 0; i < handler_size; i++) {
                if (!mini_strcmp(token, command_handler[i].name)) {
                    command_handler[i].function();
                    break;
                }
            }
            if(i >= handler_size) mini_printf("Unknown Command Entered!!\r");
        }
        index = -1;
    }
    return 0;
}
