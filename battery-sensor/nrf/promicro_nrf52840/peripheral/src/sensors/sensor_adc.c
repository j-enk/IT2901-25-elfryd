#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include "../sensor.h"


/*
-----------------------------------------------------------------------------
                        ADC sensor
-----------------------------------------------------------------------------
*/

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
int16_t buf;
struct adc_sequence sequence = {
    .buffer = &buf,
    .buffer_size = sizeof(buf),
};

int sensor_init() {
    if (!adc_is_ready_dt(&adc_channel)) {
        printk("ADC controller devivce %s not ready\n", adc_channel.dev->name);
        return -1;
    }

    int err = adc_channel_setup_dt(&adc_channel);
    if (err) {
        printk("Could not setup channel #%d (%d)\n", 0, err);
        return err;
    }

    int err = adc_sequence_init_dt(&adc_channel, &sequence);
    if (err) {
        printk("Could not initalize sequnce\n");
        return -1;
    }

    return 0;
}

static int sensor_adc_read(int16_t * mV) {

    err = adc_read(adc_channel.dev, &sequence);
    if (err) {
        printk("Could not read (%d)\n", err);
        return -1;
    }

    int16_t val_mV = buf;
    err = adc_raw_to_millivolts_dt(&adc_channel, &val_mV);
    if (err) {
        printk("adc_raw_to_millivolts_dt failed: %d\n", err);
        return -1;
    }
    val_mV *= 6.23;

    *mV = val_mV;
    return 0;
}

/*
-----------------------------------------------------------------------------
                        ADC GATT Service 
-----------------------------------------------------------------------------
*/

static ssize_t adc_GATT_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    int16_t mV = -1;
    if(sensor_adc_read(&mV) != 0){
        printk("mV failed\n");
        mV = -1;
    }
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &mV, sizeof(mV));
}

BT_GATT_SERVICE_DEFINE(adc_srvc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ADC),
    BT_GATT_CHARACTERISTIC(BT_UUID_ADC, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        adc_GATT_read, NULL, NULL),
);