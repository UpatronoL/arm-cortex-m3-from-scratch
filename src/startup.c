#include "lm3s6965.h"

typedef void (*pFunc)(void);

extern int main(void);
extern char _etext;
extern char _data;
extern char _edata;
extern char _bstart;
extern char _bend;

void Reset(void) {
    char *src = &_etext;
    char *dst = &_data;

    while (dst < &_edata) {
        *dst++ = *src++;
    }

    for (dst = &_bstart; dst < &_bend; dst++) *dst = 0;

    main();

    while(1);
}

void Default_Handler(void) { while(1); }

extern void SysTick_Handler(void);
extern void UART0_Handler(void);

extern int _stack_top; 


pFunc const vector_table[] __attribute__((section(".vector_table")))= {
    (pFunc) &_stack_top,
    Reset,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    SysTick_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    UART0_Handler
};














