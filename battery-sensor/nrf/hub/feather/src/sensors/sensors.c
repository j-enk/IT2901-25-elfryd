/**
 * @file sensors.c
 * @brief Sensor data management implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <zephyr/logging/log.h>

#include "sensors/sensors.h"
#include "utils/utils.h"
#include "i2c/i2c_master.h"

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_INF);
#define LOG_PREFIX_SENSOR "[SENSOR] "
#define LOG_PREFIX_I2C "[I2C] "

/* Data storage for sensor readings */
static battery_reading_t battery_readings[MAX_BATTERY_SAMPLES];
static int battery_count = 0;

static temp_reading_t temp_readings[MAX_TEMP_SAMPLES];
static int temp_count = 0;

static gyro_reading_t gyro_readings[MAX_GYRO_SAMPLES];
static int gyro_count = 0;

/* Mutex for protecting the reading arrays */
static K_MUTEX_DEFINE(sensor_mutex);

/* Flag to track if using I2C sensors */
static bool using_i2c = false;

int sensors_init(void)
{
#ifdef CONFIG_ELFRYD_USE_I2C_SENSORS
    int err;
#endif

    /* Initialize the arrays with empty data */
    battery_count = 0;
    temp_count = 0;
    gyro_count = 0;

    /* Seed the random number generator for sample data generation */
    sys_rand_get(NULL, 0);

    /* Check if we should use I2C sensors */
#ifdef CONFIG_ELFRYD_USE_I2C_SENSORS
    LOG_INF(LOG_PREFIX_I2C "Initializing I2C sensor interface");

    err = i2c_master_init();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_I2C "Failed to initialize I2C: %d", err);
        LOG_WRN(LOG_PREFIX_I2C "Falling back to sample data generation");
        using_i2c = false;
    }
    else
    {
        LOG_INF(LOG_PREFIX_I2C "Using I2C for sensor data collection");
        using_i2c = true;
    }
#else
    LOG_INF(LOG_PREFIX_SENSOR "Using sample data generation (I2C disabled in config)");
    using_i2c = false;
#endif

    return 0;
}

int sensors_generate_battery_reading(int battery_id)
{
    int err;
    battery_reading_t reading;

    if (battery_id < 1 || battery_id > NUM_BATTERIES)
    {
        return -EINVAL; /* Invalid battery ID */
    }

    /* Check if time is synchronized before collecting data */
    if (!utils_is_time_synchronized())
    {
        LOG_WRN(LOG_PREFIX_SENSOR "Time not synchronized, skipping battery reading");
        return 0; /* Skip data collection but don't report as error */
    }

    /* Generate battery reading from I2C or sample data */
    if (using_i2c)
    {
        err = i2c_read_battery_data(battery_id, &reading);
        if (err)
        {
            if (err == -EAGAIN)
            {
                /* No new data available, just skip without error */
                LOG_DBG(LOG_PREFIX_SENSOR "No new battery data for ID %d", battery_id);
                return 0;
            }
            LOG_ERR(LOG_PREFIX_I2C "Failed to read battery data from I2C: %d", err);
            return err; /* Return the error from I2C reading */
        }
        
        /* Successfully read new data, store it */
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (battery_count >= MAX_BATTERY_SAMPLES)
        {
            for (int i = 0; i < MAX_BATTERY_SAMPLES - 1; i++)
            {
                battery_readings[i] = battery_readings[i + 1];
            }
            battery_count = MAX_BATTERY_SAMPLES - 1;
        }

        /* Store the new reading */
        battery_readings[battery_count] = reading;
        battery_count++;

        k_mutex_unlock(&sensor_mutex);
        
        LOG_INF(LOG_PREFIX_SENSOR "New battery reading for ID %d: %d mV", 
                reading.battery_id, reading.voltage);
    }
    else
    {
        /* Generate sample battery data - only in non-I2C mode */
        reading.battery_id = battery_id;
        reading.voltage = 12000 + (sys_rand32_get() % 1501);
        reading.timestamp = utils_get_timestamp();
        
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (battery_count >= MAX_BATTERY_SAMPLES)
        {
            for (int i = 0; i < MAX_BATTERY_SAMPLES - 1; i++)
            {
                battery_readings[i] = battery_readings[i + 1];
            }
            battery_count = MAX_BATTERY_SAMPLES - 1;
        }

        /* Store the new reading */
        battery_readings[battery_count] = reading;
        battery_count++;

        k_mutex_unlock(&sensor_mutex);
    }

    return 0;
}

int sensors_generate_temp_reading(void)
{
    int err;
    temp_reading_t reading;

    /* Check if time is synchronized before collecting data */
    if (!utils_is_time_synchronized())
    {
        LOG_WRN(LOG_PREFIX_SENSOR "Time not synchronized, skipping temperature reading");
        return 0; /* Skip data collection but don't report as error */
    }

    /* Generate temperature reading from I2C or sample data */
    if (using_i2c)
    {
        err = i2c_read_temp_data(&reading);
        if (err)
        {
            if (err == -EAGAIN)
            {
                /* No new data available, just skip without error */
                LOG_DBG(LOG_PREFIX_SENSOR "No new temperature data");
                return 0;
            }
            LOG_ERR(LOG_PREFIX_I2C "Failed to read temperature data from I2C: %d", err);
            return err; /* Return the error from I2C reading */
        }
        
        /* Successfully read new data, store it */
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (temp_count >= MAX_TEMP_SAMPLES)
        {
            for (int i = 0; i < MAX_TEMP_SAMPLES - 1; i++)
            {
                temp_readings[i] = temp_readings[i + 1];
            }
            temp_count = MAX_TEMP_SAMPLES - 1;
        }

        /* Store the new reading */
        temp_readings[temp_count] = reading;
        temp_count++;

        k_mutex_unlock(&sensor_mutex);
        
        LOG_INF(LOG_PREFIX_SENSOR "New temperature reading: %d °C", reading.temperature);
    }
    else
    {
        /* Generate sample temperature data - only in non-I2C mode */
        reading.temperature = 5 + (sys_rand32_get() % 30);
        reading.timestamp = utils_get_timestamp();
        
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (temp_count >= MAX_TEMP_SAMPLES)
        {
            for (int i = 0; i < MAX_TEMP_SAMPLES - 1; i++)
            {
                temp_readings[i] = temp_readings[i + 1];
            }
            temp_count = MAX_TEMP_SAMPLES - 1;
        }

        /* Store the new reading */
        temp_readings[temp_count] = reading;
        temp_count++;

        k_mutex_unlock(&sensor_mutex);
    }

    return 0;
}

int sensors_generate_gyro_reading(void)
{
    int err;
    gyro_reading_t reading;

    /* Check if time is synchronized before collecting data */
    if (!utils_is_time_synchronized())
    {
        LOG_WRN(LOG_PREFIX_SENSOR "Time not synchronized, skipping gyroscope reading");
        return 0; /* Skip data collection but don't report as error */
    }

    /* Generate gyroscope reading from I2C or sample data */
    if (using_i2c)
    {
        err = i2c_read_gyro_data(&reading);
        if (err)
        {
            if (err == -EAGAIN)
            {
                /* No new data available, just skip without error */
                LOG_DBG(LOG_PREFIX_SENSOR "No new gyroscope data");
                return 0;
            }
            LOG_ERR(LOG_PREFIX_I2C "Failed to read gyroscope data from I2C: %d", err);
            return err; /* Return the error from I2C reading */
        }
        
        /* Successfully read new data, store it */
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (gyro_count >= MAX_GYRO_SAMPLES)
        {
            for (int i = 0; i < MAX_GYRO_SAMPLES - 1; i++)
            {
                gyro_readings[i] = gyro_readings[i + 1];
            }
            gyro_count = MAX_GYRO_SAMPLES - 1;
        }

        /* Store the new reading */
        gyro_readings[gyro_count] = reading;
        gyro_count++;

        k_mutex_unlock(&sensor_mutex);
        
        LOG_INF(LOG_PREFIX_SENSOR "New gyroscope reading received");
    }
    else
    {
        /* Generate sample gyroscope data - only in non-I2C mode */
        reading.accel_x = -5000000 + (sys_rand32_get() % 10000000);
        reading.accel_y = -5000000 + (sys_rand32_get() % 10000000);
        reading.accel_z = -5000000 + (sys_rand32_get() % 10000000);
        reading.gyro_x = -250000 + (sys_rand32_get() % 500000);
        reading.gyro_y = -250000 + (sys_rand32_get() % 500000);
        reading.gyro_z = -250000 + (sys_rand32_get() % 500000);
        reading.timestamp = utils_get_timestamp();
        
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        /* If buffer is full, make room by shifting */
        if (gyro_count >= MAX_GYRO_SAMPLES)
        {
            for (int i = 0; i < MAX_GYRO_SAMPLES - 1; i++)
            {
                gyro_readings[i] = gyro_readings[i + 1];
            }
            gyro_count = MAX_GYRO_SAMPLES - 1;
        }

        /* Store the new reading */
        gyro_readings[gyro_count] = reading;
        gyro_count++;

        k_mutex_unlock(&sensor_mutex);
    }

    return 0;
}

int sensors_get_battery_readings(battery_reading_t *readings, int max_count)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    /* Copy the available readings, up to max_count */
    count = battery_count < max_count ? battery_count : max_count;
    for (int i = 0; i < count; i++)
    {
        readings[i] = battery_readings[i];
    }

    k_mutex_unlock(&sensor_mutex);

    return count;
}

int sensors_get_temp_readings(temp_reading_t *readings, int max_count)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    /* Copy the available readings, up to max_count */
    count = temp_count < max_count ? temp_count : max_count;
    for (int i = 0; i < count; i++)
    {
        readings[i] = temp_readings[i];
    }

    k_mutex_unlock(&sensor_mutex);

    return count;
}

int sensors_get_gyro_readings(gyro_reading_t *readings, int max_count)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    /* Copy the available readings, up to max_count */
    count = gyro_count < max_count ? gyro_count : max_count;
    for (int i = 0; i < count; i++)
    {
        readings[i] = gyro_readings[i];
    }

    k_mutex_unlock(&sensor_mutex);

    return count;
}

void sensors_clear_battery_readings(void)
{
    k_mutex_lock(&sensor_mutex, K_FOREVER);
    battery_count = 0;
    k_mutex_unlock(&sensor_mutex);
}

void sensors_clear_temp_readings(void)
{
    k_mutex_lock(&sensor_mutex, K_FOREVER);
    temp_count = 0;
    k_mutex_unlock(&sensor_mutex);
}

void sensors_clear_gyro_readings(void)
{
    k_mutex_lock(&sensor_mutex, K_FOREVER);
    gyro_count = 0;
    k_mutex_unlock(&sensor_mutex);
}

/**
 * Implementation of new monitoring functions
 */

int sensors_get_latest_battery_reading(battery_reading_t *reading)
{
    if (reading == NULL)
    {
        return -EINVAL;
    }

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    if (battery_count <= 0)
    {
        k_mutex_unlock(&sensor_mutex);
        return -ENODATA;
    }

    /* Copy the most recent reading (last in the array) */
    *reading = battery_readings[battery_count - 1];

    k_mutex_unlock(&sensor_mutex);

    return 0;
}

int sensors_get_latest_temp_reading(temp_reading_t *reading)
{
    if (reading == NULL)
    {
        return -EINVAL;
    }

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    if (temp_count <= 0)
    {
        k_mutex_unlock(&sensor_mutex);
        return -ENODATA;
    }

    /* Copy the most recent reading (last in the array) */
    *reading = temp_readings[temp_count - 1];

    k_mutex_unlock(&sensor_mutex);

    return 0;
}

int sensors_get_latest_gyro_reading(gyro_reading_t *reading)
{
    if (reading == NULL)
    {
        return -EINVAL;
    }

    k_mutex_lock(&sensor_mutex, K_FOREVER);

    if (gyro_count <= 0)
    {
        k_mutex_unlock(&sensor_mutex);
        return -ENODATA;
    }

    /* Copy the most recent reading (last in the array) */
    *reading = gyro_readings[gyro_count - 1];

    k_mutex_unlock(&sensor_mutex);

    return 0;
}

int sensors_get_battery_reading_count(void)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);
    count = battery_count;
    k_mutex_unlock(&sensor_mutex);

    return count;
}

int sensors_get_temp_reading_count(void)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);
    count = temp_count;
    k_mutex_unlock(&sensor_mutex);

    return count;
}

int sensors_get_gyro_reading_count(void)
{
    int count;

    k_mutex_lock(&sensor_mutex, K_FOREVER);
    count = gyro_count;
    k_mutex_unlock(&sensor_mutex);

    return count;
}

bool sensors_using_i2c(void)
{
    return using_i2c;
}

int sensors_generate_all_battery_readings(void)
{
    int err;
    int valid_readings = 0;
    battery_reading_t temp_readings[NUM_BATTERIES];

    /* Check if time is synchronized before collecting data */
    if (!utils_is_time_synchronized())
    {
        LOG_WRN(LOG_PREFIX_SENSOR "Time not synchronized, skipping battery readings");
        return 0; /* Skip data collection but don't report as error */
    }

    /* Generate battery readings from I2C or sample data */
    if (using_i2c)
    {
        /* Use the new bulk read function for I2C mode */
        err = i2c_read_all_battery_data(temp_readings, NUM_BATTERIES);
        if (err < 0)
        {
            if (err == -EAGAIN)
            {
                /* No new data available, just skip without error */
                LOG_DBG(LOG_PREFIX_SENSOR "No new battery data available");
                return 0;
            }
            LOG_ERR(LOG_PREFIX_I2C "Failed to read battery data from I2C: %d", err);
            return err; /* Return the error from I2C reading */
        }
        
        valid_readings = err; /* err contains the number of valid readings */
        
        if (valid_readings == 0)
        {
            LOG_DBG(LOG_PREFIX_SENSOR "No new battery data to process");
            return 0;
        }

        /* Store all valid readings */
        k_mutex_lock(&sensor_mutex, K_FOREVER);

        for (int i = 0; i < valid_readings; i++)
        {
            /* If buffer is full, make room by shifting */
            if (battery_count >= MAX_BATTERY_SAMPLES)
            {
                for (int j = 0; j < MAX_BATTERY_SAMPLES - 1; j++)
                {
                    battery_readings[j] = battery_readings[j + 1];
                }
                battery_count = MAX_BATTERY_SAMPLES - 1;
            }

            /* Store the new reading */
            battery_readings[battery_count] = temp_readings[i];
            battery_count++;
            
            LOG_INF(LOG_PREFIX_SENSOR "New battery reading for ID %d: %d mV", 
                    temp_readings[i].battery_id, temp_readings[i].voltage);
        }

        k_mutex_unlock(&sensor_mutex);
    }
    else
    {
        /* Generate sample battery data for all batteries - only in non-I2C mode */
        k_mutex_lock(&sensor_mutex, K_FOREVER);
        
        for (int battery_id = 1; battery_id <= NUM_BATTERIES; battery_id++)
        {
            /* If buffer is full, make room by shifting */
            if (battery_count >= MAX_BATTERY_SAMPLES)
            {
                for (int j = 0; j < MAX_BATTERY_SAMPLES - 1; j++)
                {
                    battery_readings[j] = battery_readings[j + 1];
                }
                battery_count = MAX_BATTERY_SAMPLES - 1;
            }
            
            /* Create a new sample reading */
            battery_readings[battery_count].battery_id = battery_id;
            battery_readings[battery_count].voltage = 12000 + (sys_rand32_get() % 1501);
            battery_readings[battery_count].timestamp = utils_get_timestamp();
            battery_count++;
            valid_readings++;
        }
        
        k_mutex_unlock(&sensor_mutex);
    }

    return valid_readings;
}