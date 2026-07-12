#include <stdint.h>

#define RCGC2 ((volatile uint32_t *) 0x400FE108)
#define RCGC1 ((volatile uint32_t *) 0x400FE104)

#define GPIODIR ((volatile uint32_t *) 0x40025400)
#define GPIOA_DEN ((volatile uint32_t *) 0x4000451C)
#define GPIOF_DEN ((volatile uint32_t *) 0x4002551C)
#define GPIODATA ((volatile uint32_t *) 0x40025004)
#define GPIOAFSEL ((volatile uint32_t *) 0x40004420)
#define NVIC_EN0 ((volatile uint32_t *) 0xE000E100)
