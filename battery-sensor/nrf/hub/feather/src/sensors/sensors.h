/**
 * @file sensors.h
 * @brief Sensor data management header
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>
#include <zephyr/kernel.h>

/** 
 * Configure number of battery packs to monitor 
 * Note: This is defined by Kconfig (CONFIG_ELFRYD_NUM_BATTERIES)
 */
#define NUM_BATTERIES CONFIG_ELFRYD_NUM_BATTERIES

/**
 * Maximum number of sensor readings to store
 * Note: These are defined by Kconfig
 */
#define MAX_BATTERY_SAMPLES CONFIG_ELFRYD_MAX_BATTERY_SAMPLES
#define MAX_TEMP_SAMPLES CONFIG_ELFRYD_MAX_TEMP_SAMPLES
#define MAX_GYRO_SAMPLES CONFIG_ELFRYD_MAX_GYRO_SAMPLES

/**
 * Battery voltage reading structure
 */
typedef struct
{
    int battery_id;
    int16_t voltage;
    int64_t timestamp;
} battery_reading_t;

/**
 * Temperature reading structure
 */
typedef struct
{
    int16_t temperature;
    int64_t timestamp;
} temp_reading_t;

/**
 * Gyroscope/accelerometer reading structure
 */
typedef struct
{
    int32_t accel_x;  /* Using 24 bits only */
    int32_t accel_y;  /* Using 24 bits only */
    int32_t accel_z;  /* Using 24 bits only */
    int32_t gyro_x;   /* Using 24 bits only */
    int32_t gyro_y;   /* Using 24 bits only */
    int32_t gyro_z;   /* Using 24 bits only */
    int64_t timestamp;
} gyro_reading_t;

/**
 * Initialize the sensor module
 *
 * @return 0 on success, negative errno code on failure
 */
int sensors_init(void);

/**
 * Generate a new battery reading for a specific battery ID
 *
 * @param battery_id ID of the battery to generate a reading for
 * @return 0 on success, negative errno code on failure
 */
int sensors_generate_battery_reading(int battery_id);

/**
 * Generate readings for all batteries in a single I2C transaction (optimized)
 *
 * @return Number of readings generated, or negative errno code on failure
 */
int sensors_generate_all_battery_readings(void);

/**
 * Generate a new temperature reading
 *
 * @return 0 on success, negative errno code on failure
 */
int sensors_generate_temp_reading(void);

/**
 * Generate a new gyroscope/accelerometer reading
 *
 * @return 0 on success, negative errno code on failure
 */
int sensors_generate_gyro_reading(void);

/**
 * Get battery readings
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings copied, or negative errno code on failure
 */
int sensors_get_battery_readings(battery_reading_t *readings, int max_count);

/**
 * Get temperature readings
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings copied, or negative errno code on failure
 */
int sensors_get_temp_readings(temp_reading_t *readings, int max_count);

/**
 * Get gyroscope/accelerometer readings
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings copied, or negative errno code on failure
 */
int sensors_get_gyro_readings(gyro_reading_t *readings, int max_count);

/**
 * Clear all stored battery readings
 */
void sensors_clear_battery_readings(void);

/**
 * Clear all stored temperature readings
 */
void sensors_clear_temp_readings(void);

/**
 * Clear all stored gyroscope/accelerometer readings
 */
void sensors_clear_gyro_readings(void);

/**
 * Get the latest battery reading
 *
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno code on failure
 */
int sensors_get_latest_battery_reading(battery_reading_t *reading);

/**
 * Get the latest temperature reading
 *
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno code on failure
 */
int sensors_get_latest_temp_reading(temp_reading_t *reading);

/**
 * Get the latest gyroscope/accelerometer reading
 *
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno code on failure
 */
int sensors_get_latest_gyro_reading(gyro_reading_t *reading);

/**
 * Get the number of stored battery readings
 *
 * @return Number of readings
 */
int sensors_get_battery_reading_count(void);

/**
 * Get the number of stored temperature readings
 *
 * @return Number of readings
 */
int sensors_get_temp_reading_count(void);

/**
 * Get the number of stored gyroscope/accelerometer readings
 *
 * @return Number of readings
 */
int sensors_get_gyro_reading_count(void);

/**
 * Check if we are using I2C sensors
 *
 * @return true if using I2C sensors, false if using sample data
 */
bool sensors_using_i2c(void);

#endif /* SENSORS_H */