#include <stdint.h>

#define UARTDR ((volatile uint32_t *) 0x4000C000)
#define UARTFR ((volatile uint32_t *) 0x4000C018)
#define UARTIBRD ((volatile uint32_t *) 0x4000C024)
#define UARTFBRD ((volatile uint32_t *) 0x4000C028)
#define UARTLCRH ((volatile uint32_t *) 0x4000C02C) 
#define UARTCTL ((volatile uint32_t *) 0x4000C030)

void uart_init(void);
void uart_putc(char c);
