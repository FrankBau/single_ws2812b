#include <stm32l432xx.h>

#include <uart.h>

#include <gpio.h>

// setup of UART (USART2) peripheral block in the microcontroller

// see STM32L43xxx reference manual (RM0394 Rev 4) for USART2 registers, bits, and function
// see STM32L432KC data sheet (DS11451 Rev 4) for alternate function (AF) and pin assignment
// see Nucleo-32 board user guide (UM1956 Rev 5) for NUCLEO-L432KC board schematics (wiring)

extern uint32_t SystemCoreClock;

// init USART2 for USB VCP virtual com port connection
// PA2    ------> USART2_TX     pin alternate function: AF7 (see data sheet)
// PA15   ------> USART2_RX     pin alternate function: AF3 (see data sheet)
void uart_init(void)
{
    // setup USART2 pins
    gpio_alternate(PA2, 7);
    gpio_alternate(PA15, 3);

    // setup USART2
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; // enable clock for peripheral component
    (void)RCC->APB1ENR1;                    // ensure that the last instruction finished and the clock is now on

    uint32_t baud_rate = 115200UL;
    // USART1SEL  is assumed to be at reset default 00: PCLK selected as USART1 clock
    USART2->BRR = SystemCoreClock / baud_rate;                // USARTDIV in ref.man. 38.5.4 USART baud rate generation
    USART2->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE; // enable UART, RX, TX
}

// redirect stdout to UART for using printf and friends
int __io_putchar(int ch)
{
    while (!(USART2->ISR & USART_ISR_TXE))
        ;                      // loop while the TX register is not empty (last transmission not completed)
    USART2->TDR = (uint8_t)ch; // write the char to the transmit data register
    return ch;                 // indicate success to the caller
}

// get input char from serial port if available, don't wait.
// returns the char read or -1 if no input was available 
int getch(void)
{
    USART2->ICR = USART_ICR_ORECF; // clear overrun error flag (if set)
    if (USART2->ISR & USART_ISR_RXNE)
    {
        return (char)(USART2->RDR);
    }
    else
    {
        return -1;
    }
}


// wait for next input char from serial port.
// returns the char read 
int getch_wait(void)
{
    // clear overrun error flag (if set)
    USART2->ICR = USART_ICR_ORECF;
    while (!(USART2->ISR & USART_ISR_RXNE))
        ; // wait for input
    return (char)(USART2->RDR);
}


// read a line from input until newline char '\n' detected.
// line is filled with the input and always terminated by a trailing '\0'.
// overflowing input chars are discarded.
// returns the number of chars read.
int readline(char *line, int size)
{
    int i = 0;
    for (;;)
    {
        char ch = getch_wait();
        line[i] = ch;

        if (ch == '\n')
        {
            line[i] = '\0'; // set trailing zero char
            return i;
        }
        else if (ch == '\r')
        {
            // suppress '\r' (carriage return)
        }
        else
        {
            if (i + 1 < size)
            {
                i++;
            }
        }
    }
}
