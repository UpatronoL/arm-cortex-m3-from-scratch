#include <stdint.h>

#define SYSTICK 0xE000E000
#define STCTRL ((volatile uint32_t *) (SYSTICK + 0x010))
#define STRELOAD ((volatile uint32_t *) (SYSTICK + 0x014))
#define STCURRENT ((volatile uint32_t *) (SYSTICK + 0x018))

extern volatile uint32_t systick_count;

void systick_init (void);
void SysTick_Handler(void);
