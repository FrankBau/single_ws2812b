#include <stdio.h>
#include <stdlib.h>

#include <stm32l432xx.h>

#include <sysclk.h>
#include <gpio.h>
#include <delay.h>
#include <uart.h>
#include <apa106.h>
#include <adc.h>

int main(void)
{
    sysclk_init_hsi_16();
    gpio_init();
    delay_init();
    uart_init();
    apa106_init();
    adc_init();

    // leave PA12 open or connect it to the nearby
    // GND pin with the little black jumper
    gpio_input(PA12);
    gpio_pullup(PA12); // ensure high level when open

    // use terminal prog with baud rate 115200 8N1
    printf("hello, world\n");

    uint8_t r = 128;
    uint8_t g = 0;
    uint8_t b = 0;

    // for(;;) {
    //     char line[80]; 
    //     readline(line, sizeof(line));
    //     printf("you said: '%s'\n", line);
    // }

    /* Loop forever */
    for (;;)
    {
        setRGB(0, r, g, b);

        if (gpio_get(PA12) == 0)
        {
            delay(1000);
        }
        else
        {
            delay(200);
        }

        setRGB(0, 0, 0, 0);
        delay(200);

        char ch = getch();
        if (ch == 'R')          // increase red level
        {
            r += 16;
            printf("r=%3d\n", r);
        }
        if (ch == 'r')          // decrease red level
        {
            r -= 16;
            printf("r=%3d\n", r);
        }

        if (ch == 'm')          // do some measurements
        {
            int vpin = adc_pin(PA4);
            int vdda = adc_vdda();
            int temp = adc_temperature();
            printf("vpin = %4d [mV], vdda = %4d [mV], temp = %4d degC\n", vpin, vdda, temp);
        }
    }
}
