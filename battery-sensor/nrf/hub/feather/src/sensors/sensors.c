/**
 * @file sensors.c
 * @brief Sensor data management implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "sensors/sensors.h"
#include "utils/utils.h" /* Added for timestamp */

/* Data storage for sensor readings */
static battery_reading_t battery_readings[MAX_BATTERY_SAMPLES];
static int battery_count = 0;

static temp_reading_t temp_readings[MAX_TEMP_SAMPLES];
static int temp_count = 0;

static gyro_reading_t gyro_readings[MAX_GYRO_SAMPLES];
static int gyro_count = 0;

/* Mutex for protecting the reading arrays */
static K_MUTEX_DEFINE(sensor_mutex);

int sensors_init(void)
{
    /* Initialize the arrays with empty data */
    battery_count = 0;
    temp_count = 0;
    gyro_count = 0;

    /* Seed the random number generator */
    sys_rand_get(NULL, 0);

    return 0;
}

int sensors_generate_battery_reading(int battery_id)
{
    if (battery_id < 1 || battery_id > 4)
    {
        return -EINVAL; /* Invalid battery ID */
    }

    /* Generate random voltage between 7000mV and 13000mV */
    int voltage = 7000 + (sys_rand32_get() % 6000);

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
    battery_readings[battery_count].battery_id = battery_id;
    battery_readings[battery_count].voltage = voltage;
    battery_readings[battery_count].timestamp = utils_get_timestamp();
    battery_count++;

    k_mutex_unlock(&sensor_mutex);

    return 0;
}

int sensors_generate_temp_reading(void)
{
    /* Generate random temperature between 5°C and 35°C */
    int temperature = 5 + (sys_rand32_get() % 30);

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
    temp_readings[temp_count].temperature = temperature;
    temp_readings[temp_count].timestamp = utils_get_timestamp();
    temp_count++;

    k_mutex_unlock(&sensor_mutex);

    return 0;
}

int sensors_generate_gyro_reading(void)
{
    /* Generate random accelerometer readings */
    int accel_x = -5000000 + (sys_rand32_get() % 10000000);
    int accel_y = -5000000 + (sys_rand32_get() % 10000000);
    int accel_z = -5000000 + (sys_rand32_get() % 10000000);

    /* Generate random gyroscope readings */
    int gyro_x = -250000 + (sys_rand32_get() % 500000);
    int gyro_y = -250000 + (sys_rand32_get() % 500000);
    int gyro_z = -250000 + (sys_rand32_get() % 500000);

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
    gyro_readings[gyro_count].accel_x = accel_x;
    gyro_readings[gyro_count].accel_y = accel_y;
    gyro_readings[gyro_count].accel_z = accel_z;
    gyro_readings[gyro_count].gyro_x = gyro_x;
    gyro_readings[gyro_count].gyro_y = gyro_y;
    gyro_readings[gyro_count].gyro_z = gyro_z;
    gyro_readings[gyro_count].timestamp = utils_get_timestamp();
    gyro_count++;

    k_mutex_unlock(&sensor_mutex);

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