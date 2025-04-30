#ifndef BME280_H
#define BME280_H

#include <stdint.h>

int baut_bme_init(void);
int baut_bme_read(int32_t *celcius);

#endif // BME280_H