#ifndef ADC_H
#define ADC_H

#include <stdint.h>

int baut_adc_init(void);
int baut_adc_read(int32_t *mv);

#endif // ADC_H