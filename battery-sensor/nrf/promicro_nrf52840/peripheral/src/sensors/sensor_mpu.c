#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "../sensor.h"


/*
-----------------------------------------------------------------------------
                        MPU Gyro sensor
-----------------------------------------------------------------------------
*/

const struct device *const mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);

int sensor_init() {
    if(mpu6050 == NULL) {
        printk("MPU is missing!\n");
        return -1;
    }

    if (!device_is_ready(mpu6050)) {
        printk("MPU is not ready\n");
        return -1;
    }

    return 0;
}

static int sensor_mpu_read(double mpu_values[6]) {
    struct sensor_value accel[3];
    struct sensor_value gyro[3];

    int err = sensor_sample_fetch(mpu6050);
    if(err) {
        printf("MPU6050: sample fetch failed: %d\n", err);
        return err;
    }

    err = sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ, accel);
    if(err) {
        printf("MPU6050: accel failed: %d\n", err);
        return err;
    }

    err = sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ, gyro);
    if(err) {
        printf("MPU6050: gyro failed: %d\n", err);
        return err;
    }

    mpu_values[0] = sensor_value_to_double(&accel[0]);
    mpu_values[1] = sensor_value_to_double(&accel[1]);
    mpu_values[2] = sensor_value_to_double(&accel[2]);
    mpu_values[3] = sensor_value_to_double(&gyro[0]);
    mpu_values[4] = sensor_value_to_double(&gyro[1]);
    mpu_values[5] = sensor_value_to_double(&gyro[2]);
    return 0;
}

/*
-----------------------------------------------------------------------------
                        MPU Gyro GATT Service 
-----------------------------------------------------------------------------
*/
#define BT_UUID_GYRO BT_UUID_DECLARE_16(0x2c08)

static ssize_t mpu_GATT_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    static uint8_t packed[18]; // Static to handle GATT offset correctly
    double values[6];
    const size_t total_len = sizeof(packed);

    if (offset >= total_len) {
        return 0; // Nothing more to read
    }

    // Only fetch and convert data on first call (offset == 0)
    if (offset == 0) {
        if (baut_mpu_read(values) != 0) {
            printk("MPU: read failed\n");
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }

        printk("MPU: read successfully\n");

        for (int i = 0; i < 6; ++i) {
            int32_t fixed_val = (int32_t)(values[i] * 10000.0); // 5 decimal places

            // Pack as big-endian 24-bit signed int
            packed[i * 3 + 0] = (fixed_val >> 16) & 0xFF;
            packed[i * 3 + 1] = (fixed_val >> 8) & 0xFF;
            packed[i * 3 + 2] = fixed_val & 0xFF;
        }
    }

    return bt_gatt_attr_read(conn, attr, buf, len, offset, packed, total_len);
}

BT_GATT_SERVICE_DEFINE(mpu_srvc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_GYRO),
    BT_GATT_CHARACTERISTIC(BT_UUID_GYRO, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
        mpu_GATT_read, NULL, NULL),
);