/**
 * @file config_module.c
 * @brief Configuration management implementation
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
#include "config/config_module.h"

/* Register the module with a dedicated log level and prefix */
LOG_MODULE_REGISTER(config_module, LOG_LEVEL_INF);
#define LOG_PREFIX_CONFIG "[CONFIG] "

/* Mutex for protecting configuration access */
static K_MUTEX_DEFINE(config_mutex);

/* Configuration parameters */
static int battery_interval = DEFAULT_BATTERY_INTERVAL;
static int temp_interval = DEFAULT_TEMP_INTERVAL;
static int gyro_interval = DEFAULT_GYRO_INTERVAL;

/* Last configuration command for confirmation */
static char last_command[256]; /* Increased buffer size from 128 to 256 */
static bool has_new_command = false;

/* Declare external flags for immediate publishing */
extern struct k_mutex publish_flags_mutex;
extern bool battery_publish_request;
extern bool temp_publish_request;
extern bool gyro_publish_request;

int config_init(void)
{
    k_mutex_lock(&config_mutex, K_FOREVER);

    /* Initialize with default values */
    battery_interval = DEFAULT_BATTERY_INTERVAL;
    temp_interval = DEFAULT_TEMP_INTERVAL;
    gyro_interval = DEFAULT_GYRO_INTERVAL;
    has_new_command = false;
    memset(last_command, 0, sizeof(last_command));

    k_mutex_unlock(&config_mutex);

    return 0;
}

int config_get_battery_interval(void)
{
    int interval;

    k_mutex_lock(&config_mutex, K_FOREVER);
    interval = battery_interval;
    k_mutex_unlock(&config_mutex);

    return interval;
}

int config_get_temp_interval(void)
{
    int interval;

    k_mutex_lock(&config_mutex, K_FOREVER);
    interval = temp_interval;
    k_mutex_unlock(&config_mutex);

    return interval;
}

int config_get_gyro_interval(void)
{
    int interval;

    k_mutex_lock(&config_mutex, K_FOREVER);
    interval = gyro_interval;
    k_mutex_unlock(&config_mutex);

    return interval;
}

int config_set_battery_interval(int interval)
{
    if (interval < 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&config_mutex, K_FOREVER);
    battery_interval = interval;

    /* Store for confirmation */
    snprintf(last_command, sizeof(last_command), "battery %d", interval);
    has_new_command = true;

    k_mutex_unlock(&config_mutex);

    return 0;
}

int config_set_temp_interval(int interval)
{
    if (interval < 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&config_mutex, K_FOREVER);
    temp_interval = interval;

    /* Store for confirmation */
    snprintf(last_command, sizeof(last_command), "temperature %d", interval);
    has_new_command = true;

    k_mutex_unlock(&config_mutex);

    return 0;
}

int config_set_gyro_interval(int interval)
{
    if (interval < 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&config_mutex, K_FOREVER);
    gyro_interval = interval;

    /* Store for confirmation */
    snprintf(last_command, sizeof(last_command), "gyro %d", interval);
    has_new_command = true;

    k_mutex_unlock(&config_mutex);

    return 0;
}

int config_process_command(const char *command)
{
    char cmd_copy[256]; /* Increased buffer size from 128 to 256 */
    char *type, *value_str;
    int value = -1;
    int ret = -EINVAL;

    if (!command || command[0] == '\0')
    {
        LOG_ERR(LOG_PREFIX_CONFIG "Empty command received");
        return -EINVAL;
    }

    /* Check command length for buffer safety */
    size_t cmd_len = strlen(command);
    if (cmd_len >= sizeof(cmd_copy))
    {
        LOG_ERR(LOG_PREFIX_CONFIG "Command too long (%u bytes, max %u)",
               (unsigned int)cmd_len, (unsigned int)(sizeof(cmd_copy) - 1));
        return -EINVAL;
    }

    /* Make a copy of the command to tokenize */
    strcpy(cmd_copy, command);

    /* Parse command type and value */
    type = strtok(cmd_copy, " ");
    if (!type)
    {
        LOG_ERR(LOG_PREFIX_CONFIG "Invalid command format");
        return -EINVAL;
    }

    /* Get optional value parameter */
    value_str = strtok(NULL, " ");

    /* Process simple commands without value (meaning to send all data) */
    if (value_str == NULL)
    {
        LOG_INF(LOG_PREFIX_CONFIG "Command without value: %s", type);

        /* Commands without values trigger immediate data sending */
        if (strcmp(type, "battery") == 0)
        {
            LOG_INF(LOG_PREFIX_CONFIG "Request to send all battery data");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "battery");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);
            
            /* Set the flag to publish battery data immediately */
            k_mutex_lock(&publish_flags_mutex, K_FOREVER);
            battery_publish_request = true;
            k_mutex_unlock(&publish_flags_mutex);

            return 0;
        }
        else if (strcmp(type, "temp") == 0)
        {
            LOG_INF(LOG_PREFIX_CONFIG "Request to send all temperature data");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "temp");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);
            
            /* Set the flag to publish temperature data immediately */
            k_mutex_lock(&publish_flags_mutex, K_FOREVER);
            temp_publish_request = true;
            k_mutex_unlock(&publish_flags_mutex);

            return 0;
        }
        else if (strcmp(type, "gyro") == 0)
        {
            LOG_INF(LOG_PREFIX_CONFIG "Request to send all gyroscope data");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "gyro");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);
            
            /* Set the flag to publish gyro data immediately */
            k_mutex_lock(&publish_flags_mutex, K_FOREVER);
            gyro_publish_request = true;
            k_mutex_unlock(&publish_flags_mutex);

            return 0;
        }

        LOG_ERR(LOG_PREFIX_CONFIG "Unknown command type: %s", type);
        return -EINVAL;
    }

    /* Parse the integer value */
    value = atoi(value_str);
    LOG_INF(LOG_PREFIX_CONFIG "Command: %s with value: %d", type, value);

    /* Value must be non-negative */
    if (value < 0)
    {
        LOG_ERR(LOG_PREFIX_CONFIG "Negative values not allowed");
        return -EINVAL;
    }

    /* Process command based on type */
    if (strcmp(type, "battery") == 0)
    {
        ret = config_set_battery_interval(value);
    }
    else if (strcmp(type, "temp") == 0)
    {
        ret = config_set_temp_interval(value);
    }
    else if (strcmp(type, "gyro") == 0)
    {
        ret = config_set_gyro_interval(value);
    }
    else
    {
        LOG_ERR(LOG_PREFIX_CONFIG "Unknown command type: %s", type);
    }

    return ret;
}

int config_get_confirmation(char *buffer, size_t size)
{
    int len = 0;

    if (!buffer || size == 0)
    {
        return -EINVAL;
    }

    k_mutex_lock(&config_mutex, K_FOREVER);

    if (has_new_command)
    {
        len = snprintf(buffer, size, "%s", last_command);
        has_new_command = false;
        LOG_INF(LOG_PREFIX_CONFIG "Confirmation prepared: %s", buffer);
    }

    k_mutex_unlock(&config_mutex);

    return len;
}