#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "../sensor.h"


/*
-----------------------------------------------------------------------------
                        BME temperature sensor
-----------------------------------------------------------------------------
*/

const struct device *bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);
const struct sensor_value temperature;

int sensor_init() {
    if(bme280_dev == NULL) {
        printk("BME is missing!\n");
        return -1;
    }

    if (!device_is_ready(bme280_dev)) {
        printk("BME is not ready\n");
        return -1;
    }

    return 0;
}

static int sensor_bme_read(int16_t * temperature_celcius) {
    int err;

    err = sensor_sample_fetch(bme280_dev);
    if(err) {
        printk("BME fetch failed: %d\n", err);
        return -1;
    }

    err = sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
    if(err) {
        printk("BME channel get failed: %d\n", err);
        return -1;
    }

    *temperature_celcius = temperature.val1;

    return 0;
}

/*
-----------------------------------------------------------------------------
                        BME Temperature GATT Service 
-----------------------------------------------------------------------------
*/
#define BT_UUID_TEMPERATURE BT_UUID_DECLARE_16(0x2c07)

static ssize_t bme_GATT_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    int16_t temperature_celius = 0xBEEF;
    if(sensor_bme_read(&mV) != 0){
        printk("Temperature fetching failed\n");
        temperature_celius = 0xBEEF;
    }
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temperature_celius, sizeof(temperature_celius));
}

BT_GATT_SERVICE_DEFINE(bme_srvc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_TEMPERATURE),
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        bme_GATT_read, NULL, NULL),
);