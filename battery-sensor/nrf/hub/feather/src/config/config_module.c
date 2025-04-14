/**
 * @file config_module.c
 * @brief Configuration management implementation
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "config/config_module.h"

/* Mutex for protecting configuration access */
static K_MUTEX_DEFINE(config_mutex);

/* Configuration parameters */
static int battery_interval = DEFAULT_BATTERY_INTERVAL;
static int temp_interval = DEFAULT_TEMP_INTERVAL;
static int gyro_interval = DEFAULT_GYRO_INTERVAL;

/* Last configuration command for confirmation */
static char last_command[256]; /* Increased buffer size from 128 to 256 */
static bool has_new_command = false;

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
        printf("Error: Empty command received\n");
        return -EINVAL;
    }

    /* Check command length for buffer safety */
    size_t cmd_len = strlen(command);
    if (cmd_len >= sizeof(cmd_copy))
    {
        printf("Error: Command too long (%u bytes, max %u)\n",
               (unsigned int)cmd_len, (unsigned int)(sizeof(cmd_copy) - 1));
        return -EINVAL;
    }

    /* Make a copy of the command to tokenize */
    strcpy(cmd_copy, command);

    /* Parse command type and value */
    type = strtok(cmd_copy, " ");
    if (!type)
    {
        printf("Error: Invalid command format\n");
        return -EINVAL;
    }

    /* Get optional value parameter */
    value_str = strtok(NULL, " ");

    /* Process simple commands without value (meaning to send all data) */
    if (value_str == NULL)
    {
        printf("Command without value: %s\n", type);

        /* Commands without values trigger immediate data sending */
        if (strcmp(type, "battery") == 0)
        {
            printf("Request to send all battery data\n");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "battery send");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);

            return 0;
        }
        else if (strcmp(type, "temp") == 0)
        {
            printf("Request to send all temperature data\n");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "temperature send");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);

            return 0;
        }
        else if (strcmp(type, "gyro") == 0)
        {
            printf("Request to send all gyroscope data\n");

            /* Store confirmation that will be sent back */
            k_mutex_lock(&config_mutex, K_FOREVER);
            snprintf(last_command, sizeof(last_command), "gyro send");
            has_new_command = true;
            k_mutex_unlock(&config_mutex);

            return 0;
        }

        printf("Unknown command type: %s\n", type);
        return -EINVAL;
    }

    /* Parse the integer value */
    value = atoi(value_str);
    printf("Command: %s with value: %d\n", type, value);

    /* Value must be non-negative */
    if (value < 0)
    {
        printf("Error: Negative values not allowed\n");
        return -EINVAL;
    }

    /* Process command based on type */
    if (strcmp(type, "battery") == 0)
    {
        ret = config_set_battery_interval(value);
    }
    else if (strcmp(type, "temp") == 0 || strcmp(type, "temperature") == 0)
    {
        ret = config_set_temp_interval(value);
    }
    else if (strcmp(type, "gyro") == 0)
    {
        ret = config_set_gyro_interval(value);
    }
    else
    {
        printf("Unknown command type: %s\n", type);
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
        printf("Confirmation prepared: %s\n", buffer);
    }

    k_mutex_unlock(&config_mutex);

    return len;
}