/**
 * @file i2c_master.c
 * @brief I2C master interface implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <zephyr/random/rand32.h>
#include "i2c/i2c_master.h"
#include "utils/utils.h"

LOG_MODULE_REGISTER(i2c_master, LOG_LEVEL_INF);
#define LOG_PREFIX_I2C "[I2C] "
#define LOG_PREFIX_HW "[HW] "

/* I2C device */
static const struct device *i2c_dev;

/* Mutex for protecting I2C operations */
static K_MUTEX_DEFINE(i2c_mutex);

/* Default I2C addresses for sensors */
#define I2C_ADDR_BATTERY_BASE 0x10 /* Base address for batteries (0x10-0x13) */
#define I2C_ADDR_TEMP 0x20         /* Temperature sensor address */
#define I2C_ADDR_GYRO 0x30         /* Gyroscope sensor address */

/* Register addresses - only need data registers for local timestamp mode */
#define REG_BATTERY_VOLTAGE 0x01
#define REG_TEMP_VALUE 0x01
#define REG_GYRO_ACCEL_X 0x01
#define REG_GYRO_ACCEL_Y 0x03
#define REG_GYRO_ACCEL_Z 0x05
#define REG_GYRO_GYRO_X 0x07
#define REG_GYRO_GYRO_Y 0x09
#define REG_GYRO_GYRO_Z 0x0B

/* Flag to track if I2C is ready */
static bool i2c_ready = false;

int i2c_master_init(void)
{
    /* Get the I2C device by its device tree node name - using i2c1 for Circuit Dojo board */
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

    if (!device_is_ready(i2c_dev))
    {
        LOG_ERR(LOG_PREFIX_I2C "I2C device not ready");
        return -ENODEV;
    }

    LOG_INF(LOG_PREFIX_I2C "I2C master initialized successfully");
    i2c_ready = true;
    return 0;
}

bool i2c_is_ready(void)
{
    return i2c_ready;
}

/* Helper function to read a 16-bit value from an I2C device */
static int read_i2c_16bit(uint8_t dev_addr, uint8_t reg_addr, int16_t *value)
{
    uint8_t data[2];
    int ret;

    k_mutex_lock(&i2c_mutex, K_FOREVER);

    /* Read the data from the specified register */
    ret = i2c_write_read(i2c_dev, dev_addr, &reg_addr, 1, data, 2);

    k_mutex_unlock(&i2c_mutex);

    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_HW "Failed to read from I2C device 0x%02x (reg 0x%02x): %d",
                dev_addr, reg_addr, ret);
        return ret;
    }

    /* Combine the two bytes into a 16-bit value */
    *value = (int16_t)((data[0] << 8) | data[1]);

    return 0;
}

int i2c_read_battery_data(int battery_id, battery_reading_t *reading)
{
    int ret;
    int32_t voltage_raw;
    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for battery reading");
        return -ENODEV;
    }

    if (battery_id < 1 || battery_id > 4 || !reading)
    {
        return -EINVAL;
    }

    /* Calculate the I2C address for this battery_id */
    uint8_t dev_addr = I2C_ADDR_BATTERY_BASE + (battery_id - 1);

    /* Read just the voltage value */
    // ret = read_i2c_16bit(dev_addr, REG_BATTERY_VOLTAGE, &voltage_raw);

    // [ID: 1byte, Payload: 4bytes]
    uint8_t data[5];
    ret = i2c_write_read(i2c_dev, dev_addr, &REG_BATTERY_VOLTAGE, 1, data, sizeof(data));
    if (ret < 0||data[0] != battery_id)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read battery data from I2C: %d", ret);
        return ret;
    }

    /* Fill in the battery reading structure with local timestamp */
    reading->battery_id = battery_id;
    reading->voltage = ((int32_t)data[1]) | ((int32_t)data[2] << 8) | ((int32_t)data[3] << 16) | ((int32_t)data[4] << 24);
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read battery data: id=%d, voltage=%d mV, timestamp=%lld",
            reading->battery_id, reading->voltage, reading->timestamp);

    return 0;
}

int i2c_read_temp_data(temp_reading_t *reading)
{
    int ret;
    int16_t temp_raw;

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for temperature reading");
        return -ENODEV;
    }

    if (!reading)
    {
        return -EINVAL;
    }

    /* Read just the temperature value */
    ret = read_i2c_16bit(I2C_ADDR_TEMP, REG_TEMP_VALUE, &temp_raw);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read temperature data from I2C: %d", ret);
        return ret;
    }

    /* Fill in the temperature reading structure with local timestamp */
    reading->temperature = temp_raw;
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read temperature data: %d °C, timestamp=%lld",
            reading->temperature, reading->timestamp);

    return 0;
}

int i2c_read_gyro_data(gyro_reading_t *reading)
{
    int ret;
    int16_t value;

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for gyroscope reading");
        return -ENODEV;
    }

    if (!reading)
    {
        return -EINVAL;
    }

    /* Read accelerometer X-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_X, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read accelerometer X data: %d", ret);
        return ret;
    }
    reading->accel_x = value;

    /* Read accelerometer Y-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_Y, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read accelerometer Y data: %d", ret);
        return ret;
    }
    reading->accel_y = value;

    /* Read accelerometer Z-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_Z, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read accelerometer Z data: %d", ret);
        return ret;
    }
    reading->accel_z = value;

    /* Read gyroscope X-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_X, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read gyroscope X data: %d", ret);
        return ret;
    }
    reading->gyro_x = value;

    /* Read gyroscope Y-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_Y, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read gyroscope Y data: %d", ret);
        return ret;
    }
    reading->gyro_y = value;

    /* Read gyroscope Z-axis */
    ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_Z, &value);
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read gyroscope Z data: %d", ret);
        return ret;
    }
    reading->gyro_z = value;

    /* Set local timestamp */
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read gyro data: accel_x=%d, accel_y=%d, accel_z=%d, gyro_x=%d, gyro_y=%d, gyro_z=%d, timestamp=%lld",
            reading->accel_x, reading->accel_y, reading->accel_z,
            reading->gyro_x, reading->gyro_y, reading->gyro_z,
            reading->timestamp);

    return 0;
}