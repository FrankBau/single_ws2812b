#include <stdlib.h>

#include <stm32l432xx.h>

// https://www.led-stuebchen.de/de/50x-apa106-f5-p9823-yf923-5mm-rgb-led-integriertem-controller-ws2812b


// implementation with a 16 MHz clock
#define T0H     350 * 16 / 1000
#define T1H    1360 * 16 / 1000
#define PERIOD  (T0H + T1H)

#define LEDS    256

// use HSI16 16 MHz clock which is the most precise internal clock, factory trimmed to +/- 1%
void SystemClock_Config_HSI16(void) {
    // Enable HSI16 oscillator
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    // Set the system clock source to HSI16
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
}


void init_TIM16() {
    // set PA6 to AF14 == TIM16 CH1  
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // enable clock for peripheral component 
    (void)RCC->AHB2ENR; // ensure that the last write command finished and the clock is on
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE6_Msk) | (2 << GPIO_MODER_MODE6_Pos);   // AF mode
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL6_Msk) | (14 << GPIO_AFRL_AFSEL6_Pos); // AF14

    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; // enable clock for peripheral component 
    (void)RCC->APB2ENR; // read-back ensures delay before the clock be active

    TIM16->PSC = 1-1;       // pre-scaler set to k-1, timer running/counting at SYSCLK/k MHz
    TIM16->ARR = PERIOD-1;  // timer/counter period

    TIM16->CCMR1 = (TIM16->CCMR1 &~TIM_CCMR1_CC1S_Msk) | (0 << TIM_CCMR1_CC1S_Pos);   // CH1 output compare (OC) mode
    TIM16->CCMR1 = (TIM16->CCMR1 &~TIM_CCMR1_OC1M_Msk) | (6 << TIM_CCMR1_OC1M_Pos);   // OC mode 6: PWM1
    TIM16->CCMR1 |= TIM_CCMR1_OC1PE;    // pre-load enable
    TIM16->DIER |= TIM_DIER_UDE;        // DMA request on timer update event enable
    TIM16->CCER |= TIM_CCER_CC1E;       // CH1 enable
    TIM16->BDTR |= TIM_BDTR_MOE;        // master output enable
    TIM16->CR1 |= TIM_CR1_CEN;          // timer enable
}


uint16_t bits[24*LEDS+200];    // array of PWM encoded 'bits' which will be sent to the LED by DMA transfer


void init_DMA() {
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; // enable clock for peripheral component 
    (void)RCC->AHB1ENR; // ensure that the last write command finished and the clock is on

    // see ref.man. Table 41. DMA1 requests for each channel
    // Channel 6 with C6S = 4 is triggered by TIM16 update event
    DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR &~DMA_CSELR_C6S_Msk) | (4 << DMA_CSELR_C6S_Pos);

    DMA1_Channel6->CCR = 0;                                 // disable DMA channel for setup
    DMA1_Channel6->CPAR = (uint32_t)&TIM16->CCR1;           // DMA target (peripheral) address
    DMA1_Channel6->CMAR = (uint32_t)bits;                   // DMA source (memory) address
    DMA1_Channel6->CNDTR = sizeof(bits)/sizeof(bits[0]);    // DMA transfer size
    DMA1_Channel6->CCR |= 1 << DMA_CCR_MSIZE_Pos;           // read 16-bit data
    DMA1_Channel6->CCR |= 1 << DMA_CCR_PSIZE_Pos;           // write 16-bit data
    DMA1_Channel6->CCR |= DMA_CCR_DIR;                      // transfer direction memory -> peripheral
    DMA1_Channel6->CCR |= DMA_CCR_MINC;                     // increment memory address after each transfer
    DMA1_Channel6->CCR |= DMA_CCR_CIRC;                     // circular (endless) DMA 
    DMA1_Channel6->CCR |= DMA_CCR_EN;                       // DMA channel enable
}


// gamma[i] = max_value * (i/max_value)^2.8
const uint8_t gamma_table[256] = {
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,     1,   2,   2,   2,   2,   2,   2,   2,
      2,   3,   3,   3,   3,   3,   3,   3,     4,   4,   4,   4,   4,   5,   5,   5,
  
      5,   6,   6,   6,   6,   7,   7,   7,     7,   8,   8,   8,   9,   9,   9,  10,
     10,  10,  11,  11,  11,  12,  12,  13,    13,  13,  14,  14,  15,  15,  16,  16,
     17,  17,  18,  18,  19,  19,  20,  20,    21,  21,  22,  22,  23,  24,  24,  25,
     25,  26,  27,  27,  28,  29,  29,  30,    31,  32,  32,  33,  34,  35,  35,  36,
  
     37,  38,  39,  39,  40,  41,  42,  43,    44,  45,  46,  47,  48,  49,  50,  50,
     51,  52,  54,  55,  56,  57,  58,  59,    60,  61,  62,  63,  64,  66,  67,  68,
     69,  70,  72,  73,  74,  75,  77,  78,    79,  81,  82,  83,  85,  86,  87,  89,
     90,  92,  93,  95,  96,  98,  99, 101,   102, 104, 105, 107, 109, 110, 112, 114,
  
    115, 117, 119, 120, 122, 124, 126, 127,   129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158,   160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193,   196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233,   236, 239, 241, 244, 247, 249, 252, 255,
};


void setRGBraw(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    for(int i=7; i>=0; --i) {
        bits[24*led+i]    = (r & (1<<(7-i))) ? T1H : T0H;
        bits[24*led+8+i]  = (g & (1<<(7-i))) ? T1H : T0H;
        bits[24*led+16+i] = (b & (1<<(7-i))) ? T1H : T0H;
    }
}


void setRGB(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t r_ = gamma_table[r];
    uint8_t g_ = gamma_table[g];
    uint8_t b_ = gamma_table[b];
    setRGBraw(led, r_, g_, b_);
}


// hue is 0..360 degree, s and v are 0..255
void setHSV(uint8_t led, uint16_t h, uint8_t s, uint8_t v) {

    uint8_t r, g, b;

    if (s == 0) {
        // If saturation is 0, the color is grayscale (r = g = b = value)
        r = g = b = v;
    } else {
        int region = h / 60;            // region of the color circle
        int m = (h % 60) * 255 / 60;    // remainder in the region
        int p = (v * (255 - s)) / 255;
        int q = (v * (255 - ((s *        m ) / 255))) / 255;
        int t = (v * (255 - ((s * (255 - m)) / 255))) / 255;
        switch (region) {
            case 0:     r = v;  g = t;  b = p;  break;
            case 1:     r = q;  g = v;  b = p;  break;
            case 2:     r = p;  g = v;  b = t;  break;
            case 3:     r = p;  g = q;  b = v;  break;
            case 4:     r = t;  g = p;  b = v;  break;
            default:    r = v;  g = p;  b = q;  break;
        }
    }
    setRGB(led, r, g, b);
}


void delay(void) {
    // some visible delay, exact value not critical
    for(volatile int i=0; i<500000; ++i);
}


void bit_test(void) {
    setRGBraw(0, 0, 0, 0);
    delay();
    for(uint8_t i=1; i; i<<=1) {
        setRGBraw(0, i, 0, 0);
        delay();
    }
    for(uint8_t i=1; i; i<<=1) {
        setRGBraw(0, 0, i, 0);
        delay();
    }
    for(uint8_t i=1; i; i<<=1) {
        setRGBraw(0, 0, 0, i);
        delay();
    }
}


int main(void)
 {
    SystemClock_Config_HSI16();
    init_TIM16();
    init_DMA();

    bit_test();

    /* Loop forever */
    for(;;) {
        uint16_t hue = rand() % 360;
        for(int led=0; led<LEDS; ++led) {
            setHSV(led, hue, 255, 255);
        }
        delay();
    }
}
