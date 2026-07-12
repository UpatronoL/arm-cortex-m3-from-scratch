#include "mini_printf.h"
#include "uart.h"
#include <stdarg.h>

int mini_printf(char *format, ...) {
    va_list args;
    va_start (args, format);
    int count = 0;
    int i, digit;
    unsigned int b;
    int index = 0;
    char a;
    char *s;
    char buffer[12];
    for(const char *c = format; *c != '\0'; c++) {
        if (*c != '%') {
            uart_putc(*c);
            count++;
            continue;
        }
        
        c++;

        switch (*c) {
            case 'd':
                i = va_arg(args, int);

                if (i < 0) {
                    i = ~i + 1;
                    uart_putc('-');
                    count++;
                }

                do {
                    digit = i % 10;
                    buffer[index++] = digit + '0';
                    i /= 10;
                } while (i > 0);

                for (int i = index - 1; i >= 0; i--) {
                    uart_putc(buffer[i]);
                    count++;
                }
                index = 0;
                break;
            case 'u':
                b = va_arg(args, unsigned int);
                
                do {
                    digit = b % 10;
                    buffer[index++] = digit + '0';
                    b /= 10;
                } while (b > 0);

                for (int i = index - 1; i >= 0; i--) {
                    uart_putc(buffer[i]);
                    count++;
                }
                index = 0;
                break;
            case 'x':
                b = va_arg(args, unsigned int);
                do {
                    digit = b % 16;
                    if (digit < 10)
                        buffer[index++] = digit + '0';
                    else 
                        buffer[index++] = (digit - 10) + 'A';
                    b /= 16;
                } while (b > 0);

                for (int i = index - 1; i >= 0; i--) {
                    uart_putc(buffer[i]);
                    count++;
                }
                index = 0;
                break;
            case 'c':
                a = (char) va_arg(args, int);
                uart_putc(a);
                count++;
                break;
            case 's':
                s =  va_arg(args, char *);
                while(*s) {
                    uart_putc(*s);
                    count++;
                    s++;
                }
                break;
        }
    }
    va_end(args);
    return count;
}
