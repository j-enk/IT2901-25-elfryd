#include "mpu6050.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

const struct device *const mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);

int baut_mpu_init(void) {
    if (mpu6050 == NULL) {
        printk("MPU is missing!\n");
        return -1;
    }

    if (!device_is_ready(mpu6050)) {
        printk("MPU is not ready\n");
        return -1;
    }

    return 0;
}

int baut_mpu_read(double values[6]) {
    struct sensor_value accel[3];
    struct sensor_value gyro[3];

    int err = sensor_sample_fetch(mpu6050);
    if (err) {
        printk("MPU6050: sample fetch failed: %d\n", err);
        return err;
    }

    err = sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ, accel);
    if (err) {
        printk("MPU6050: accel failed: %d\n", err);
        return err;
    }

    err = sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ, gyro);
    if (err) {
        printk("MPU6050: gyro failed: %d\n", err);
        return err;
    }

    values[0] = sensor_value_to_double(&accel[0]);
    values[1] = sensor_value_to_double(&accel[1]);
    values[2] = sensor_value_to_double(&accel[2]);
    values[3] = sensor_value_to_double(&gyro[0]);
    values[4] = sensor_value_to_double(&gyro[1]);
    values[5] = sensor_value_to_double(&gyro[2]);

    return 0;
}