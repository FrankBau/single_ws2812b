#ifndef __APA106_H
#define __APA106_H

void apa106_init(void);

void setHSV(uint8_t led, uint16_t h, uint8_t s, uint8_t v);

void setRGB(uint8_t led, uint8_t r, uint8_t g, uint8_t b);

void setRGBraw(uint8_t led, uint8_t r, uint8_t g, uint8_t b);

#endif
