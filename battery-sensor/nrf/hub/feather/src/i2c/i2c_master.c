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

/* I2C addresses for sensors - all battery data comes from the same address */
#define I2C_ADDR_BATTERY 0x10      /* Battery sensor address */
#define I2C_ADDR_TEMP 0x20         /* Temperature sensor address */
#define I2C_ADDR_GYRO 0x30         /* Gyroscope sensor address */

/* Register addresses for sensors */
#define REG_BATTERY_DATA 0x01      /* Register containing all battery data */
#define REG_TEMP_DATA 0x01         /* Register containing temperature data */
#define REG_GYRO_DATA 0x01         /* Register containing gyroscope data */

/* Data formats */
#define BATTERY_BYTES_PER_READING 4    /* 1 byte new flag + 1 byte ID + 2 bytes voltage */
#define TEMP_DATA_SIZE 3               /* 1 byte new flag + 2 bytes temperature */
#define GYRO_DATA_SIZE 19              /* 1 byte new flag + 6 x 3 bytes for accel/gyro values */

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

/* Helper function to read all battery data from central device */
static int read_battery_data_block(uint8_t *data, size_t size)
{
    uint8_t reg = REG_BATTERY_DATA;
    int ret;

    if (!data || size == 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&i2c_mutex, K_FOREVER);

    /* Read the data from the battery register */
    ret = i2c_write_read(i2c_dev, I2C_ADDR_BATTERY, &reg, 1, data, size);

    k_mutex_unlock(&i2c_mutex);

    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_HW "Failed to read battery data from I2C: %d", ret);
        return ret;
    }

    return 0;
}

/* Helper function to read temperature data from central device */
static int read_temp_data_block(uint8_t *data, size_t size)
{
    uint8_t reg = REG_TEMP_DATA;
    int ret;

    if (!data || size == 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&i2c_mutex, K_FOREVER);

    /* Read the data from the temperature register */
    ret = i2c_write_read(i2c_dev, I2C_ADDR_TEMP, &reg, 1, data, size);

    k_mutex_unlock(&i2c_mutex);

    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_HW "Failed to read temperature data from I2C: %d", ret);
        return ret;
    }

    return 0;
}

/* Helper function to read gyroscope data from central device */
static int read_gyro_data_block(uint8_t *data, size_t size)
{
    uint8_t reg = REG_GYRO_DATA;
    int ret;

    if (!data || size == 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&i2c_mutex, K_FOREVER);

    /* Read the data from the gyroscope register */
    ret = i2c_write_read(i2c_dev, I2C_ADDR_GYRO, &reg, 1, data, size);

    k_mutex_unlock(&i2c_mutex);

    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_HW "Failed to read gyroscope data from I2C: %d", ret);
        return ret;
    }

    return 0;
}

int i2c_read_battery_data(int battery_id, battery_reading_t *reading)
{
    int ret;
    /* Buffer for all batteries - max 4 batteries with 4 bytes each */
    uint8_t data[NUM_BATTERIES * BATTERY_BYTES_PER_READING];

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for battery reading");
        return -ENODEV;
    }

    if (battery_id < 1 || battery_id > NUM_BATTERIES || !reading)
    {
        return -EINVAL;
    }

    /* Read all battery data from the register */
    ret = read_battery_data_block(data, sizeof(data));
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read battery data from I2C: %d", ret);
        return ret;
    }

    /* Calculate offset for this battery ID (0-based index in the array) */
    int offset = (battery_id - 1) * BATTERY_BYTES_PER_READING;
    
    /* Check if we have new data (first byte is the 'new' flag) */
    bool is_new = (data[offset] != 0);
    
    if (!is_new)
    {
        /* Skip this reading if there's no new data */
        LOG_DBG(LOG_PREFIX_I2C "No new data for battery %d", battery_id);
        return -EAGAIN;
    }

    /* Extract battery ID from data (second byte) */
    uint8_t id_from_data = data[offset + 1];
    
    /* Sanity check that IDs match */
    if (id_from_data != battery_id)
    {
        LOG_WRN(LOG_PREFIX_I2C "Battery ID mismatch: expected %d, got %d", 
                battery_id, id_from_data);
    }
    
    /* Extract voltage from data (third and fourth bytes - int16_t) */
    int16_t voltage;
    memcpy(&voltage, &data[offset + 2], sizeof(int16_t));

    /* Fill in the battery reading structure with local timestamp */
    reading->battery_id = battery_id;
    reading->voltage = voltage;
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read battery data: id=%d, voltage=%d mV, timestamp=%lld",
            reading->battery_id, reading->voltage, reading->timestamp);

    return 0;
}

int i2c_read_all_battery_data(battery_reading_t *readings, int max_readings)
{
    int ret;
    /* Buffer for all batteries - max NUM_BATTERIES with BATTERY_BYTES_PER_READING bytes each */
    uint8_t data[NUM_BATTERIES * BATTERY_BYTES_PER_READING];
    int valid_readings = 0;

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for battery reading");
        return -ENODEV;
    }

    if (!readings || max_readings <= 0)
    {
        return -EINVAL;
    }

    /* Read all battery data from the register */
    ret = read_battery_data_block(data, sizeof(data));
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read battery data from I2C: %d", ret);
        return ret;
    }

    /* Process each battery entry in the data block */
    for (int i = 0; i < NUM_BATTERIES && i < max_readings; i++)
    {
        /* Calculate offset for this battery */
        int offset = i * BATTERY_BYTES_PER_READING;
        
        /* Check if we have new data (first byte is the 'new' flag) */
        bool is_new = (data[offset] != 0);
        
        if (!is_new)
        {
            /* Skip this reading if there's no new data */
            LOG_DBG(LOG_PREFIX_I2C "No new data for battery %d", i + 1);
            continue;
        }

        /* Extract battery ID from data (second byte) */
        uint8_t id_from_data = data[offset + 1];
        
        /* Sanity check that IDs match expected pattern */
        if (id_from_data < 1 || id_from_data > NUM_BATTERIES)
        {
            LOG_WRN(LOG_PREFIX_I2C "Invalid battery ID in data: %d", id_from_data);
            continue;
        }
        
        /* Extract voltage from data (third and fourth bytes - int16_t) */
        int16_t voltage;
        memcpy(&voltage, &data[offset + 2], sizeof(int16_t));

        /* Fill in the battery reading structure with local timestamp */
        readings[valid_readings].battery_id = id_from_data;
        readings[valid_readings].voltage = voltage;
        readings[valid_readings].timestamp = utils_get_timestamp();

        LOG_DBG(LOG_PREFIX_I2C "Read battery data: id=%d, voltage=%d mV, timestamp=%lld",
                readings[valid_readings].battery_id, 
                readings[valid_readings].voltage, 
                readings[valid_readings].timestamp);

        valid_readings++;
    }

    LOG_INF(LOG_PREFIX_I2C "Read %d valid battery readings from I2C", valid_readings);
    return valid_readings;
}

int i2c_read_temp_data(temp_reading_t *reading)
{
    int ret;
    uint8_t data[TEMP_DATA_SIZE];

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for temperature reading");
        return -ENODEV;
    }

    if (!reading)
    {
        return -EINVAL;
    }

    /* Read temperature data block from the register */
    ret = read_temp_data_block(data, sizeof(data));
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read temperature data from I2C: %d", ret);
        return ret;
    }

    /* Check if we have new data (first byte is the 'new' flag) */
    bool is_new = (data[0] != 0);
    
    if (!is_new)
    {
        /* Skip this reading if there's no new data */
        LOG_DBG(LOG_PREFIX_I2C "No new temperature data");
        return -EAGAIN;
    }
    
    /* Extract temperature value from data (second and third bytes - int16_t) */
    int16_t temperature;
    memcpy(&temperature, &data[1], sizeof(int16_t));

    /* Fill in the temperature reading structure with local timestamp */
    reading->temperature = temperature;
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read temperature data: %d °C, timestamp=%lld",
            reading->temperature, reading->timestamp);

    return 0;
}

int i2c_read_gyro_data(gyro_reading_t *reading)
{
    int ret;
    uint8_t data[GYRO_DATA_SIZE];

    if (!i2c_ready)
    {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for gyroscope reading");
        return -ENODEV;
    }

    if (!reading)
    {
        return -EINVAL;
    }

    /* Read gyroscope data block from the register */
    ret = read_gyro_data_block(data, sizeof(data));
    if (ret < 0)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to read gyroscope data from I2C: %d", ret);
        return ret;
    }

    /* Check if we have new data (first byte is the 'new' flag) */
    bool is_new = (data[0] != 0);
    
    if (!is_new)
    {
        /* Skip this reading if there's no new data */
        LOG_DBG(LOG_PREFIX_I2C "No new gyroscope data");
        return -EAGAIN;
    }

    /* Extract accelerometer and gyroscope values (int24_t values after the new flag) */
    int32_t values[6];
    for (int i = 0; i < 6; i++) {
        /* Construct the 24-bit value from 3 bytes */
        int32_t val = ((uint32_t)data[1 + i * 3] << 16) | 
                      ((uint32_t)data[2 + i * 3] << 8) | 
                      (uint32_t)data[3 + i * 3];
        
        /* Check if this is a negative number (bit 23 is set) */
        if (val & 0x800000) {
            /* Sign extension - set bits 24-31 to 1 */
            val |= 0xFF000000;
        }
        
        values[i] = val;
    }
    
    /* Fill in the gyroscope reading structure with local timestamp */
    reading->accel_x = values[0];
    reading->accel_y = values[1];
    reading->accel_z = values[2];
    reading->gyro_x = values[3];
    reading->gyro_y = values[4];
    reading->gyro_z = values[5];
    reading->timestamp = utils_get_timestamp();

    LOG_DBG(LOG_PREFIX_I2C "Read gyro data: accel_x=%d, accel_y=%d, accel_z=%d, gyro_x=%d, gyro_y=%d, gyro_z=%d, timestamp=%lld",
            reading->accel_x, reading->accel_y, reading->accel_z,
            reading->gyro_x, reading->gyro_y, reading->gyro_z,
            reading->timestamp);

    return 0;
}
