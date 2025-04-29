/**
 * @file i2c_master.h
 * @brief I2C master interface for the Elfryd hub
 */

#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <zephyr/kernel.h>
#include <stdbool.h>
#include "sensors/sensors.h"

/**
 * @brief Initialize the I2C master interface
 *
 * @return 0 on success, negative errno otherwise
 */
int i2c_master_init(void);

/**
 * @brief Read battery data from a slave device
 *
 * @param battery_id ID of the battery to read (1-4)
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_battery_data(int battery_id, battery_reading_t *reading);

/**
 * @brief Read all battery data from the slave device in a single transaction
 *
 * @param readings Array to store the readings
 * @param max_readings Maximum number of readings to process
 * @return Number of valid readings on success, negative errno otherwise
 */
int i2c_read_all_battery_data(battery_reading_t *readings, int max_readings);

/**
 * @brief Read temperature data from a slave device
 *
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_temp_data(temp_reading_t *reading);

/**
 * @brief Read gyroscope data from a slave device
 *
 * @param reading Pointer to store the reading
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_gyro_data(gyro_reading_t *reading);

/**
 * @brief Check if the I2C system is ready to read data
 *
 * @return true if the I2C system is ready, false otherwise
 */
bool i2c_is_ready(void);

#endif /* I2C_MASTER_H */