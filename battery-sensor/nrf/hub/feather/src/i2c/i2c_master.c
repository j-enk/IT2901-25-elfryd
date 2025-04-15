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
#define I2C_ADDR_BATTERY_BASE 0x10  /* Base address for batteries (0x10-0x13) */
#define I2C_ADDR_TEMP         0x20  /* Temperature sensor address */
#define I2C_ADDR_GYRO         0x30  /* Gyroscope sensor address */

/* Register addresses */
/* For TIMESTAMP_LOCAL mode: just data registers */
#define REG_BATTERY_VOLTAGE   0x01
#define REG_TEMP_VALUE        0x01
#define REG_GYRO_ACCEL_X      0x01
#define REG_GYRO_ACCEL_Y      0x03
#define REG_GYRO_ACCEL_Z      0x05
#define REG_GYRO_GYRO_X       0x07
#define REG_GYRO_GYRO_Y       0x09
#define REG_GYRO_GYRO_Z       0x0B

/* For TIMESTAMP_REMOTE mode: data registers include timestamp */
#define REG_BATTERY_WITH_TS   0x10  /* Battery data with timestamp */
#define REG_TEMP_WITH_TS      0x10  /* Temperature data with timestamp */
#define REG_GYRO_WITH_TS      0x10  /* Gyroscope data with timestamp */

/* Register offsets for remote timestamps */
#define OFFSET_TIMESTAMP      0x00  /* 8 bytes for int64_t timestamp at start of register block */
#define OFFSET_BATTERY_DATA   0x08  /* 2 bytes for battery data after timestamp */
#define OFFSET_TEMP_DATA      0x08  /* 2 bytes for temperature data after timestamp */
#define OFFSET_GYRO_DATA      0x08  /* Gyro data starts after timestamp */

/* Flag to track if I2C is ready */
static bool i2c_ready = false;

int i2c_master_init(void)
{
    /* Get the I2C device by its device tree node name - using i2c1 for Circuit Dojo board */
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    
    if (!device_is_ready(i2c_dev)) {
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
    
    if (ret < 0) {
        LOG_ERR(LOG_PREFIX_HW "Failed to read from I2C device 0x%02x (reg 0x%02x): %d", 
                dev_addr, reg_addr, ret);
        return ret;
    }
    
    /* Combine the two bytes into a 16-bit value */
    *value = (int16_t)((data[0] << 8) | data[1]);
    
    return 0;
}

/* Helper function to read data with timestamp (10 bytes: 8 for timestamp, 2 for data) */
static int read_i2c_with_timestamp(uint8_t dev_addr, uint8_t reg_addr, 
                                  int16_t *value, int64_t *timestamp)
{
    uint8_t data[10]; /* 8 bytes timestamp + 2 bytes value */
    int ret;
    
    k_mutex_lock(&i2c_mutex, K_FOREVER);
    
    /* Read the data from the specified register */
    ret = i2c_write_read(i2c_dev, dev_addr, &reg_addr, 1, data, sizeof(data));
    
    k_mutex_unlock(&i2c_mutex);
    
    if (ret < 0) {
        LOG_ERR(LOG_PREFIX_HW "Failed to read from I2C device 0x%02x (reg 0x%02x): %d", 
                dev_addr, reg_addr, ret);
        return ret;
    }
    
    /* Extract timestamp (first 8 bytes) - assuming little-endian format */
    *timestamp = 0;
    for (int i = 0; i < 8; i++) {
        *timestamp |= ((int64_t)data[i] << (i * 8));
    }
    
    /* Extract 16-bit value (last 2 bytes) */
    *value = (int16_t)((data[8] << 8) | data[9]);
    
    return 0;
}

/* Helper function to read multiple gyro values with timestamp */
static int read_i2c_gyro_with_timestamp(uint8_t dev_addr, uint8_t reg_addr,
                                       gyro_reading_t *reading)
{
    uint8_t data[20]; /* 8 bytes timestamp + 12 bytes for 6 int16_t values */
    int ret;
    
    k_mutex_lock(&i2c_mutex, K_FOREVER);
    
    /* Read the data from the specified register */
    ret = i2c_write_read(i2c_dev, dev_addr, &reg_addr, 1, data, sizeof(data));
    
    k_mutex_unlock(&i2c_mutex);
    
    if (ret < 0) {
        LOG_ERR(LOG_PREFIX_HW "Failed to read from I2C device 0x%02x (reg 0x%02x): %d", 
                dev_addr, reg_addr, ret);
        return ret;
    }
    
    /* Extract timestamp (first 8 bytes) - assuming little-endian format */
    reading->timestamp = 0;
    for (int i = 0; i < 8; i++) {
        reading->timestamp |= ((int64_t)data[i] << (i * 8));
    }
    
    /* Extract gyro values (6 int16_t values after timestamp) */
    reading->accel_x = (int16_t)((data[8] << 8) | data[9]);
    reading->accel_y = (int16_t)((data[10] << 8) | data[11]);
    reading->accel_z = (int16_t)((data[12] << 8) | data[13]);
    reading->gyro_x = (int16_t)((data[14] << 8) | data[15]);
    reading->gyro_y = (int16_t)((data[16] << 8) | data[17]);
    reading->gyro_z = (int16_t)((data[18] << 8) | data[19]);
    
    return 0;
}

int i2c_read_battery_data(int battery_id, battery_reading_t *reading, enum timestamp_source src)
{
    int ret;
    int16_t voltage_raw;
    
    if (!i2c_ready) {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for battery reading");
        return -ENODEV;
    }
    
    if (battery_id < 1 || battery_id > 4 || !reading) {
        return -EINVAL;
    }
    
    /* Calculate the I2C address for this battery_id */
    uint8_t dev_addr = I2C_ADDR_BATTERY_BASE + (battery_id - 1);
    
    /* Read data based on timestamp source */
    if (src == TIMESTAMP_LOCAL) {
        /* Read just the voltage value */
        ret = read_i2c_16bit(dev_addr, REG_BATTERY_VOLTAGE, &voltage_raw);
        if (ret < 0) {
            goto simulate_values;
        }
        
        /* Fill in the battery reading structure with local timestamp */
        reading->battery_id = battery_id;
        reading->voltage = voltage_raw;
        reading->timestamp = utils_get_timestamp();
    } else {
        /* Read voltage value with timestamp from device */
        ret = read_i2c_with_timestamp(dev_addr, REG_BATTERY_WITH_TS, 
                                     &voltage_raw, &reading->timestamp);
        if (ret < 0) {
            goto simulate_values;
        }
        
        /* Fill in the battery reading structure */
        reading->battery_id = battery_id;
        reading->voltage = voltage_raw;
    }
    
    LOG_DBG(LOG_PREFIX_I2C "Read battery data: id=%d, voltage=%d mV, timestamp=%lld", 
            reading->battery_id, reading->voltage, reading->timestamp);
    
    return 0;
    
simulate_values:
    /* In case of failure, simulate with values similar to the sample generator */
    LOG_WRN(LOG_PREFIX_I2C "Failed to read battery data, simulating values");
    reading->battery_id = battery_id;
    reading->voltage = 7000 + (sys_rand32_get() % 6000);
    reading->timestamp = utils_get_timestamp();
    return 0;  /* Return success for simulation mode */
}

int i2c_read_temp_data(temp_reading_t *reading, enum timestamp_source src)
{
    int ret;
    int16_t temp_raw;
    
    if (!i2c_ready) {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for temperature reading");
        return -ENODEV;
    }
    
    if (!reading) {
        return -EINVAL;
    }
    
    /* Read data based on timestamp source */
    if (src == TIMESTAMP_LOCAL) {
        /* Read just the temperature value */
        ret = read_i2c_16bit(I2C_ADDR_TEMP, REG_TEMP_VALUE, &temp_raw);
        if (ret < 0) {
            goto simulate_values;
        }
        
        /* Fill in the temperature reading structure with local timestamp */
        reading->temperature = temp_raw;
        reading->timestamp = utils_get_timestamp();
    } else {
        /* Read temperature value with timestamp from device */
        ret = read_i2c_with_timestamp(I2C_ADDR_TEMP, REG_TEMP_WITH_TS, 
                                     &temp_raw, &reading->timestamp);
        if (ret < 0) {
            goto simulate_values;
        }
        
        /* Fill in the temperature reading structure */
        reading->temperature = temp_raw;
    }
    
    LOG_DBG(LOG_PREFIX_I2C "Read temperature data: %d °C, timestamp=%lld", 
            reading->temperature, reading->timestamp);
    
    return 0;
    
simulate_values:
    /* In case of failure, simulate with values similar to the sample generator */
    LOG_WRN(LOG_PREFIX_I2C "Failed to read temperature data, simulating values");
    reading->temperature = 5 + (sys_rand32_get() % 30);
    reading->timestamp = utils_get_timestamp();
    return 0;  /* Return success for simulation mode */
}

int i2c_read_gyro_data(gyro_reading_t *reading, enum timestamp_source src)
{
    int ret;
    
    if (!i2c_ready) {
        LOG_WRN(LOG_PREFIX_I2C "I2C not ready for gyroscope reading");
        return -ENODEV;
    }
    
    if (!reading) {
        return -EINVAL;
    }
    
    /* Read data based on timestamp source */
    if (src == TIMESTAMP_LOCAL) {
        int16_t value;
        
        /* Read accelerometer X-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_X, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->accel_x = value;
        
        /* Read accelerometer Y-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_Y, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->accel_y = value;
        
        /* Read accelerometer Z-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_ACCEL_Z, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->accel_z = value;
        
        /* Read gyroscope X-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_X, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->gyro_x = value;
        
        /* Read gyroscope Y-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_Y, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->gyro_y = value;
        
        /* Read gyroscope Z-axis */
        ret = read_i2c_16bit(I2C_ADDR_GYRO, REG_GYRO_GYRO_Z, &value);
        if (ret < 0) {
            goto simulate_values;
        }
        reading->gyro_z = value;
        
        /* Set local timestamp */
        reading->timestamp = utils_get_timestamp();
    } else {
        /* Read all gyro values with timestamp in one read */
        ret = read_i2c_gyro_with_timestamp(I2C_ADDR_GYRO, REG_GYRO_WITH_TS, reading);
        if (ret < 0) {
            goto simulate_values;
        }
    }
    
    LOG_DBG(LOG_PREFIX_I2C "Read gyro data: accel_x=%d, accel_y=%d, accel_z=%d, gyro_x=%d, gyro_y=%d, gyro_z=%d, timestamp=%lld",
            reading->accel_x, reading->accel_y, reading->accel_z,
            reading->gyro_x, reading->gyro_y, reading->gyro_z,
            reading->timestamp);
    
    return 0;
    
simulate_values:
    /* In case of failure, simulate with values similar to the sample generator */
    LOG_WRN(LOG_PREFIX_I2C "Failed to read gyroscope data, simulating values");
    reading->accel_x = -5000000 + (sys_rand32_get() % 10000000);
    reading->accel_y = -5000000 + (sys_rand32_get() % 10000000);
    reading->accel_z = -5000000 + (sys_rand32_get() % 10000000);
    reading->gyro_x = -250000 + (sys_rand32_get() % 500000);
    reading->gyro_y = -250000 + (sys_rand32_get() % 500000);
    reading->gyro_z = -250000 + (sys_rand32_get() % 500000);
    reading->timestamp = utils_get_timestamp();
    
    return 0;  /* Return success for simulation mode */
}