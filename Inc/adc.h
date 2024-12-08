#ifndef __ADC_H
#define __ADC_H

void adc_init(void);

int adc_pin(uint8_t Pxy);

int adc_vdda(void);

int adc_temperature(void);

#endif
