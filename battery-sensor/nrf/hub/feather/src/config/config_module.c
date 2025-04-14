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
static char last_command[64];
static bool has_new_command = false;

int config_init(void)
{
    k_mutex_lock(&config_mutex, K_FOREVER);
    
    /* Initialize with default values */
    battery_interval = DEFAULT_BATTERY_INTERVAL;
    temp_interval = DEFAULT_TEMP_INTERVAL;
    gyro_interval = DEFAULT_GYRO_INTERVAL;
    has_new_command = false;
    
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
    if (interval < 0) {
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
    if (interval < 0) {
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
    if (interval < 0) {
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
    char cmd_copy[64];
    char *type, *value_str;
    int value = -1;
    int ret = -EINVAL;
    
    if (!command || strlen(command) >= sizeof(cmd_copy)) {
        return -EINVAL;
    }
    
    /* Make a copy of the command to tokenize */
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    /* Parse command type and value */
    type = strtok(cmd_copy, " ");
    if (!type) {
        return -EINVAL;
    }
    
    value_str = strtok(NULL, " ");
    if (value_str) {
        value = atoi(value_str);
    }
    
    /* Process command based on type */
    if (strcmp(type, "battery") == 0) {
        if (value_str) {
            ret = config_set_battery_interval(value);
        } else {
            /* No value means send all battery data */
            ret = config_set_battery_interval(battery_interval);  /* No change, just trigger confirmation */
        }
    } else if (strcmp(type, "temperature") == 0) {
        if (value_str) {
            ret = config_set_temp_interval(value);
        } else {
            /* No value means send all temperature data */
            ret = config_set_temp_interval(temp_interval);  /* No change, just trigger confirmation */
        }
    } else if (strcmp(type, "gyro") == 0) {
        if (value_str) {
            ret = config_set_gyro_interval(value);
        } else {
            /* No value means send all gyro data */
            ret = config_set_gyro_interval(gyro_interval);  /* No change, just trigger confirmation */
        }
    }
    
    return ret;
}

int config_get_confirmation(char *buffer, size_t size)
{
    int len = 0;
    
    k_mutex_lock(&config_mutex, K_FOREVER);
    
    if (has_new_command) {
        len = snprintf(buffer, size, "%s", last_command);
        has_new_command = false;
    }
    
    k_mutex_unlock(&config_mutex);
    
    return len;
}