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
 * @brief Timestamp source for I2C data
 */
enum timestamp_source {
    TIMESTAMP_LOCAL,  /**< Timestamp generated locally when data is received */
    TIMESTAMP_REMOTE  /**< Timestamp included in the I2C data from remote device */
};

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
 * @param src Source of the timestamp (local or remote)
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_battery_data(int battery_id, battery_reading_t *reading, enum timestamp_source src);

/**
 * @brief Read temperature data from a slave device
 *
 * @param reading Pointer to store the reading
 * @param src Source of the timestamp (local or remote)
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_temp_data(temp_reading_t *reading, enum timestamp_source src);

/**
 * @brief Read gyroscope data from a slave device
 *
 * @param reading Pointer to store the reading
 * @param src Source of the timestamp (local or remote)
 * @return 0 on success, negative errno otherwise
 */
int i2c_read_gyro_data(gyro_reading_t *reading, enum timestamp_source src);

/**
 * @brief Check if the I2C system is ready to read data
 *
 * @return true if the I2C system is ready, false otherwise
 */
bool i2c_is_ready(void);

#endif /* I2C_MASTER_H */