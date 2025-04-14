/**
 * @file config.h
 * @brief Configuration management for the Elfryd hub
 */

#ifndef CONFIG_MODULE_H
#define CONFIG_MODULE_H

#include <stdbool.h>

/** Default sensor sampling intervals in seconds */
#define DEFAULT_BATTERY_INTERVAL 100
#define DEFAULT_TEMP_INTERVAL 300
#define DEFAULT_GYRO_INTERVAL 10

/** Configuration parameter types */
typedef enum
{
    CONFIG_PARAM_BATTERY,
    CONFIG_PARAM_TEMP,
    CONFIG_PARAM_GYRO
} config_param_t;

/**
 * @brief Initialize the configuration module
 *
 * @return 0 on success, negative errno code on failure
 */
int config_init(void);

/**
 * @brief Get the current battery sampling interval
 *
 * @return Interval in seconds (0 = disabled)
 */
int config_get_battery_interval(void);

/**
 * @brief Get the current temperature sampling interval
 *
 * @return Interval in seconds (0 = disabled)
 */
int config_get_temp_interval(void);

/**
 * @brief Get the current gyroscope sampling interval
 *
 * @return Interval in seconds (0 = disabled)
 */
int config_get_gyro_interval(void);

/**
 * @brief Set the battery sampling interval
 *
 * @param interval Interval in seconds (0 = disabled)
 * @return 0 on success, negative errno code on failure
 */
int config_set_battery_interval(int interval);

/**
 * @brief Set the temperature sampling interval
 *
 * @param interval Interval in seconds (0 = disabled)
 * @return 0 on success, negative errno code on failure
 */
int config_set_temp_interval(int interval);

/**
 * @brief Set the gyroscope sampling interval
 *
 * @param interval Interval in seconds (0 = disabled)
 * @return 0 on success, negative errno code on failure
 */
int config_set_gyro_interval(int interval);

/**
 * @brief Process a configuration command
 *
 * @param command Command string to process (e.g., "battery 10")
 * @return 0 on success, negative errno code on failure
 */
int config_process_command(const char *command);

/**
 * @brief Get the latest configuration change to confirm
 *
 * @param buffer Buffer to store the confirmation message
 * @param size Size of the buffer
 * @return Length of the confirmation message or negative errno code on failure
 */
int config_get_confirmation(char *buffer, size_t size);

#endif /* CONFIG_MODULE_H */