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

    gpio_output(PB3);   // green user LED on Nucleo board

    // leave PA12 open or connect it to the nearby
    // GND pin with the little black jumper
    gpio_input(PA12);   // digital input
    gpio_pullup(PA12);  // ensure high level when open

    // use terminal prog with baud rate 115200 8N1 for serial comm
    printf("hello, world\n");

    // initial color for the digital APA106 RGB LED attached to pin PA6
    uint8_t r = 128;
    uint8_t g = 0;
    uint8_t b = 0;

    // for testing readline
    // for(;;) {
    //     char line[80]; 
    //     readline(line, sizeof(line));
    //     printf("you said: '%s'\n", line);
    // }

    /* Loop forever */
    for (;;)
    {
        setRGB(0, r, g, b);     // set RGB LED color 
        gpio_set_1(PB3);        // green LED on

        if (gpio_get(PA12) == 0)
        {
            delay(1000);
        }
        else
        {
            delay(200);
        }

        setRGB(0, 0, 0, 0);     // RGB LED off
        gpio_set_0(PB3);        // green LED off
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
