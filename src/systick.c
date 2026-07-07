#include "systick.h"
#include "lm3s6965.h"
#include <stdint.h>

void systick_init (void) {
    *STRELOAD = 11999;
    *STCURRENT = 0;
    *STCTRL |= ((1 << 0) | (1 << 1) | (1 << 2));
}

volatile uint32_t systick_count = 0;

void SysTick_Handler() {
    systick_count++;
    *GPIODATA ^= (1 << 0);
}
