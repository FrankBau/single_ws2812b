#include <stm32l432xx.h>

uint32_t SystemCoreClock = 4000000UL;   // reset default using MSI clock @ 4 MHz

void SystemInit(void) {
    // SystemInit is automagically called from the startup code before main(). 
    // enable FPU, needed for printf and your own floating point stuff
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2)); // Set CP10 and CP11 Full Access
}
