#include <stm32l432xx.h>

#include <delay.h>

extern uint32_t SystemCoreClock;


// init 32-bit timer TIM2 in up-counting mode
void delay_init(void)
{
    // assume that all relevant clocks (SYSCLK, HCLK, PCLK, TIM2CLK are 4 MHz)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; // enable peripheral clock
    (void)RCC->APB1ENR1;                  // ensure that the last write command finished and the clock is on

    TIM2->CR1 |= TIM_CR1_ARPE;                     // register pre-load enable
    TIM2->PSC = (SystemCoreClock / 1000000UL) - 1; // prescaler divider to 1 MHz timer ticks (increments)
    TIM2->ARR = 0xFFFFFFFFUL;                      // count full 32-bit period
    TIM2->EGR = TIM_EGR_UG;                        // generate update event to set internal PSC and ARR registers

    TIM2->CR1 |= TIM_CR1_CEN; // enable the timer, start counting
}

// delay execution for us microseconds (at least)
void delay_us(uint32_t us)
{
    uint32_t start = TIM2->CNT;
    // 32-bit overflow is okay in the loop below
    while (TIM2->CNT - start < us)
        ;
}

// delay execution for ms milliseconds (at least)
void delay(uint32_t ms)
{
    delay_us(1000UL * ms); // handle overflow?
}
