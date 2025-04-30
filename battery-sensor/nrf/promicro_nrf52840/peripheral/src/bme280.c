#include "bme280.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

const struct device *bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);
static struct sensor_value temperature;

int baut_bme_init(void) {
    if (bme280_dev == NULL) {
        printk("BME is missing!\n");
        return -1;
    }

    if (!device_is_ready(bme280_dev)) {
        printk("BME is not ready\n");
        return -1;
    }

    return 0;
}

int baut_bme_read(int32_t *celcius) {
    int succeeded = false;

    for (int i = 0; i < 10; i++) {
        int err = sensor_sample_fetch(bme280_dev);
        if (err) {
            if (err != -EAGAIN) {
                printk("BME sample fail: %d\n", err);
            }
            continue;
        }

        err = sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
        if (err) {
            printk("BME channel fail: %d\n", err);
            continue;
        }

        succeeded = true;
        break;
    }

    if (succeeded) {
        *celcius = temperature.val1;
    }

    return succeeded ? 0 : -1;
}