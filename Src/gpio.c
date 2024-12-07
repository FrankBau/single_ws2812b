#include <stm32l432xx.h>

#include <gpio.h>

// currently only GPIOA and GPIOB are supported

// to keep the code small, it is assumed that the microcontroller registers
// still have their reset default values and were not changed by other code.


// internal "constructor", do not call.
void gpio_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;
}

// configure pin Pxy as a digital input
void gpio_input(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->PUPDR &= ~(3 << (2 * pin)); // set no pull-up/down (reset state)
    port->MODER &= ~(3 << (2 * pin)); // clear pin mode bits
    // port->MODER |=  (0 << (2*pin));  // set mode 0 (input mode)
    //  the previous statement does nothing, but looks cool
}

// configure pin as a digital output (push-pull)
void gpio_output(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->OTYPER &= ~(1 << pin);      // set type 0: push-pull (reset state)
    port->PUPDR &= ~(3 << (2 * pin)); // set no pull-up/down (reset state)
    port->MODER &= ~(3 << (2 * pin)); // clear pin mode bits
    port->MODER |= (1 << (2 * pin));  // set mode 1 (output mode)
}

// configure pin as a digital open-drain output
void gpio_opendrain(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->OTYPER |= (1 << pin);       // set type 1: open-drain
    port->MODER &= ~(3 << (2 * pin)); // clear pin mode bits
    port->MODER |= (1 << (2 * pin));  // set mode 1 (output mode)
}

// configure pin for alternate functions (see data sheet for AF)
void gpio_alternate(uint8_t Pxy, uint8_t function)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->AFR[pin / 8] &= ~(0xF << (4 * (pin % 8)));
    port->AFR[pin / 8] |= (function << (4 * (pin % 8)));
    port->MODER &= ~(3 << (2 * pin)); // clear pin mode bits
    port->MODER |= (2 << (2 * pin));  // set mode 2 (AF mode)
}

// configure pin for additional functions (reset state, see data sheet)
void gpio_additional(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    // port->MODER &= ~(3 << (2*pin));   // clear pin mode bits
    port->MODER |= (3 << (2 * pin)); // set mode 3 (additional mode)
}

// enable internal pull-up resistor
void gpio_pullup(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->PUPDR &= ~(3 << (2 * pin)); // clear bitmask for pin
    port->PUPDR |= (1 << (2 * pin));  // 1: internal pull-up resistor
}

// enable internal pull-down resistor
void gpio_pulldown(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->PUPDR &= ~(3 << (2 * pin)); // clear bitmask for pin
    port->PUPDR |= (2 << (2 * pin));  // 2: internal pull-down resistor
}

// get level of Pin Pxy
int gpio_get(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    return 1 & (port->IDR >> pin);
}

// set the level of a digital output pin Pxy to 0 (low)
void gpio_set_0(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->ODR &= ~(1 << pin);
}

// set the level of a digital output pin Pxy to 1 (high)
void gpio_set_1(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->ODR |= (1 << pin);
}

// toggle the level of a digital output pin Pxy
void gpio_toggle(uint8_t Pxy)
{
    GPIO_TypeDef *port = GPIO_PORT(Pxy);
    int pin = GPIO_PIN(Pxy);
    port->ODR ^= (1 << pin);
}
