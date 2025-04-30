#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

int baut_mpu_init(void);
int baut_mpu_read(double values[6]);

#endif // MPU6050_H