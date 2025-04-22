#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>

#define SLEEP_TIME_MS 1000

const struct device *bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);
static struct sensor_value temperature;

void main(void) {

    int err;

    if (!device_is_ready(bme280_dev)) {
        printk("Device is not ready\n");
        return;
    }
    while(true) {

        do {

            err = sensor_sample_fetch(bme280_dev);
            err = sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
            if (err < 0) {
                printk("Failed to get temperature: %d", err);
                break;
            }

            printk("Temperature: %d Celsius\n", temperature.val1);

        } while (false);

        k_msleep(SLEEP_TIME_MS);
    }
}