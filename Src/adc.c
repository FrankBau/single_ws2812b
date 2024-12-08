#include <stm32l432xx.h>

#include <adc.h>

#include <gpio.h>
#include <delay.h>

// default until measured
int vdda_mV = 3300;

void adc_init(void)
{
    // the GPIO pins used for ADC are already in analog mode after reset
    // no need to re-config GPIO here (unless used before)

    // System clock selected as ADCs clock
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_ADCSEL_Msk) | (3 << RCC_CCIPR_ADCSEL_Pos);

    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // turn ADC peripheral clock on
    (void)RCC->AHB2ENR;                // read back to make sure that clock is on

    // HCLK/4 (Synchronous clock mode)
    ADC1_COMMON->CCR |= 3 << ADC_CCR_CKMODE_Pos;

    ADC1->CR &= ~ADC_CR_DEEPPWD; // disable deep power-down
    ADC1->CR |= ADC_CR_ADVREGEN; // power up ADC voltage regulator

    // wait  t_ADCVREG_STUP (ADC voltage regulator start-up time),
    delay_us(20);   //data sheet Table 63. ADC characteristics

    // config
    ADC1->CFGR &= ~ADC_CFGR_RES;   // Set resolution to 12 bits
    ADC1->CFGR &= ~ADC_CFGR_ALIGN; // Right alignment
    ADC1->CFGR &= ~ADC_CFGR_CONT;  // Single conversion mode

    // do self calibration
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
        ; // wait for calibration to finish
    // now, ADC1->CALFACT holds the calibration factor.

    delay_us(10);   // wait >4 ADC clock cycles

    // "enable the ADC" procedure from ref.man:
    ADC1->ISR |= ADC_ISR_ADRDY; //  Clear the ADRDY bit in ADC_ISR register
    ADC1->CR |= ADC_CR_ADEN;    //  Set ADEN = 1 in the ADC_CR register.
    while (!(ADC1->ISR & ADC_ISR_ADRDY))
        ; //  Wait until ADRDY = 1 in the ADC_ISR register

    vdda_mV = adc_vdda(); // initial measurement of vdda
}

// measure the analog voltage at ADC channel
// returns the raw digiatl 12-bit value
static int adc_channel_raw(uint8_t channel)
{
    ADC1->SQR1 &= ~ADC_SQR1_L_Msk;                                                  // set sequence length to 1 conversion
    ADC1->SQR1 = (ADC1->SQR1 & ~ADC_SQR1_SQ1_Msk) | (channel << ADC_SQR1_SQ1_Pos);  // set rank 1 channel number

    // set longest possibble sampling time of 640.5 ADC clock cycles
    if(channel < 10)
        ADC1->SMPR1 |= 7 << (3 * channel);
    else
        ADC1->SMPR1 |= 7 << (3 * (channel-10));

    ADC1->CR |= ADC_CR_ADSTART; // start ADC conversion
    while (!(ADC1->ISR & ADC_ISR_EOC))
        ;                             // wait for end of conversion
    uint32_t adc_data_raw = ADC1->DR; // conversion done. store result

    return adc_data_raw;
}

// measure the analog voltage at pin Pxy (PA0..PA7).
// returns the measured voltage in millivolts
int adc_pin(uint8_t Pxy)
{
    // ADC1_IN5 is PA0 and so on
    uint32_t channel = 5 + GPIO_PIN(Pxy);
    uint32_t adc_data_raw = adc_channel_raw(channel);
    int adc_data_mV = (adc_data_raw * vdda_mV) / 4095; // 12 bit digital reading
    return adc_data_mV;
}


// calibration values stored in the the engineering bytes
#define VREFINT_CAL     *((uint16_t*) (0x1FFF75AAUL))
#define TS_CAL1         *((uint16_t*) (0x1FFF75A8UL))
#define TS_CAL2         *((uint16_t*) (0x1FFF75CAUL))

// reference conditions which were used during calibration 
#define VREFINT_CAL_VREF           3000
#define TS_CAL1_TEMP                 30
#define TS_CAL2_TEMP                130
#define TS_CAL_VREFANALOG          3000

// sample reading:  temp_raw_12bit:  868; vref_raw_12bit: 1431; vref_cal_raw_12bit: 1654; ts_cal1_raw_12bit: 1034; ts_cal2_raw_12bit: 1381; vdda_mV: 3467; temp_degC:   21

// returns the actual chip supply voltage Vdda in mV
int adc_vdda(void)
{
    // see ref.man. 16.4.34 Monitoring the internal voltage reference
    ADC1_COMMON->CCR |= ADC_CCR_VREFEN;

    // tstart_vrefint
    delay_us(12);

    // the internal reference voltage (VREFINT) is connected to ADC1_INP0/INN0
    volatile int vrefint_data = adc_channel_raw(0);
    
    // Calculating the actual VDDA voltage using the internal reference voltage
    // factory calibration was done at a fixed vdda of VREFINT_CAL_VREF mV
    vdda_mV = (VREFINT_CAL_VREF * VREFINT_CAL) / vrefint_data;

    return vdda_mV;
}


// returns the actual chip temperature in °C
int adc_temperature(void)
{
    // see ref.man. 16.4.32 Temperature sensor
    ADC1_COMMON->CCR |= ADC_CCR_TSEN;

    // tstart
    delay_us(120);

    // the internal temperature sensor (VTS) is connected to ADC1_INP17/INN17
    int ts_raw = adc_channel_raw(17);
    
    int ts_cal1 = TS_CAL1 * TS_CAL_VREFANALOG / vdda_mV;
    int ts_cal2 = TS_CAL2 * TS_CAL_VREFANALOG / vdda_mV;

    // TODO: 
    int temp_C = TS_CAL1_TEMP + (TS_CAL2_TEMP - TS_CAL1_TEMP) * (ts_raw - ts_cal1) / (ts_cal2 - ts_cal1);

    return temp_C;
}
