/**
 * @file sensors.h
 * @brief Sensor data management for the Elfryd hub
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>

/** Maximum number of battery samples to store */
#define MAX_BATTERY_SAMPLES 20

/** Maximum number of temperature samples to store */
#define MAX_TEMP_SAMPLES 20

/** Maximum number of gyroscope samples to store */
#define MAX_GYRO_SAMPLES 20

/** Battery reading structure */
typedef struct
{
    int battery_id;    /**< Battery identifier */
    int voltage;       /**< Voltage in millivolts */
    int64_t timestamp; /**< Unix timestamp in seconds */
} battery_reading_t;

/** Temperature reading structure */
typedef struct
{
    int temperature;   /**< Temperature in degrees Celsius */
    int64_t timestamp; /**< Unix timestamp in seconds */
} temp_reading_t;

/** Gyroscope reading structure */
typedef struct
{
    int accel_x;       /**< Accelerometer X-axis reading */
    int accel_y;       /**< Accelerometer Y-axis reading */
    int accel_z;       /**< Accelerometer Z-axis reading */
    int gyro_x;        /**< Gyroscope X-axis reading */
    int gyro_y;        /**< Gyroscope Y-axis reading */
    int gyro_z;        /**< Gyroscope Z-axis reading */
    int64_t timestamp; /**< Unix timestamp in seconds */
} gyro_reading_t;

/**
 * @brief Initialize the sensor module
 *
 * @return 0 on success, negative errno otherwise
 */
int sensors_init(void);

/**
 * @brief Generate a new battery reading and store it
 *
 * @param battery_id The battery ID (1-4)
 * @return 0 on success, negative errno otherwise
 */
int sensors_generate_battery_reading(int battery_id);

/**
 * @brief Generate a new temperature reading and store it
 *
 * @return 0 on success, negative errno otherwise
 */
int sensors_generate_temp_reading(void);

/**
 * @brief Generate a new gyroscope reading and store it
 *
 * @return 0 on success, negative errno otherwise
 */
int sensors_generate_gyro_reading(void);

/**
 * @brief Get the newest battery readings for publishing
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings retrieved
 */
int sensors_get_battery_readings(battery_reading_t *readings, int max_count);

/**
 * @brief Get the newest temperature readings for publishing
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings retrieved
 */
int sensors_get_temp_readings(temp_reading_t *readings, int max_count);

/**
 * @brief Get the newest gyroscope readings for publishing
 *
 * @param readings Array to store the readings
 * @param max_count Maximum number of readings to retrieve
 * @return Number of readings retrieved
 */
int sensors_get_gyro_readings(gyro_reading_t *readings, int max_count);

/**
 * @brief Clear all battery readings
 */
void sensors_clear_battery_readings(void);

/**
 * @brief Clear all temperature readings
 */
void sensors_clear_temp_readings(void);

/**
 * @brief Clear all gyroscope readings
 */
void sensors_clear_gyro_readings(void);

/**
 * @brief Get the most recent battery reading
 *
 * @param reading Pointer to store the most recent reading
 * @return 0 if successful, -ENODATA if no readings available
 */
int sensors_get_latest_battery_reading(battery_reading_t *reading);

/**
 * @brief Get the most recent temperature reading
 *
 * @param reading Pointer to store the most recent reading
 * @return 0 if successful, -ENODATA if no readings available
 */
int sensors_get_latest_temp_reading(temp_reading_t *reading);

/**
 * @brief Get the most recent gyroscope reading
 *
 * @param reading Pointer to store the most recent reading
 * @return 0 if successful, -ENODATA if no readings available
 */
int sensors_get_latest_gyro_reading(gyro_reading_t *reading);

/**
 * @brief Get the current number of battery readings stored
 *
 * @return Number of battery readings currently stored
 */
int sensors_get_battery_reading_count(void);

/**
 * @brief Get the current number of temperature readings stored
 *
 * @return Number of temperature readings currently stored
 */
int sensors_get_temp_reading_count(void);

/**
 * @brief Get the current number of gyroscope readings stored
 *
 * @return Number of gyroscope readings currently stored
 */
int sensors_get_gyro_reading_count(void);

/**
 * @brief Check if the sensor module is using I2C for data collection
 * 
 * @return true if using I2C, false if using sample data generation
 */
bool sensors_using_i2c(void);

#endif /* SENSORS_H */