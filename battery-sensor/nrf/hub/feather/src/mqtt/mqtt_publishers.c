/**
 * @file mqtt_publishers.c
 * @brief MQTT sensor data publishing implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/random/rand32.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "mqtt/mqtt_publishers.h"
#include "mqtt/mqtt_client.h"
#include "config/config_module.h"
#include "utils/utils.h"

/* Sensor data publishing */
int mqtt_client_publish_battery(battery_reading_t *readings, int count)
{
    int err;
    char message[512]; /* Buffer for message */
    size_t offset = 0;
    bool first = true;

    /* Format multiple readings with pipe separator */
    for (int i = 0; i < count && offset < sizeof(message) - 32; i++)
    {
        size_t len;
        char timestamp_str[24]; /* Dedicated buffer for timestamp */

        /* Format timestamp using the utility function */
        int ts_len = format_timestamp(readings[i].timestamp, timestamp_str, sizeof(timestamp_str));
        if (ts_len < 0)
        {
            printk("Error formatting timestamp\n");
            continue;
        }

        if (!first)
        {
            /* Add separator between readings */
            message[offset++] = '|';
        }

        /* Format: "{battery_id}/{voltage}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                       "%d/%d/%s",
                       readings[i].battery_id,
                       readings[i].voltage,
                       timestamp_str);

        if (len >= sizeof(message) - offset - 16)
        { /* Extra margin */
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }

        offset += len;
        first = false;
    }

    /* Debug output to verify format */
    printk("Battery payload: %s\n", message);

    if (offset > 0)
    {
        /* Publish the message with QoS 2 */
        err = mqtt_client_publish(MQTT_TOPIC_BATTERY, message, MQTT_QOS_2_EXACTLY_ONCE);
    }
    else
    {
        err = -EINVAL;
    }

    return err;
}

int mqtt_client_publish_temp(temp_reading_t *readings, int count)
{
    int err;
    char message[256];
    size_t offset = 0;
    bool first = true;

    /* Format multiple readings with pipe separator */
    for (int i = 0; i < count && offset < sizeof(message) - 32; i++)
    {
        size_t len;
        char timestamp_str[24]; /* Dedicated buffer for timestamp */

        /* Format timestamp using the utility function */
        int ts_len = format_timestamp(readings[i].timestamp, timestamp_str, sizeof(timestamp_str));
        if (ts_len < 0)
        {
            printk("Error formatting timestamp\n");
            continue;
        }

        if (!first)
        {
            /* Add separator between readings */
            message[offset++] = '|';
        }

        /* Format: "{temperature}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                       "%d/%s",
                       readings[i].temperature,
                       timestamp_str);

        if (len >= sizeof(message) - offset - 16)
        {
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }

        offset += len;
        first = false;
    }

    /* Debug output to verify format */
    printk("Temperature payload: %s\n", message);

    if (offset > 0)
    {
        /* Publish the message with QoS 2 */
        err = mqtt_client_publish(MQTT_TOPIC_TEMP, message, MQTT_QOS_2_EXACTLY_ONCE);
    }
    else
    {
        err = -EINVAL;
    }

    return err;
}

int mqtt_client_publish_gyro(gyro_reading_t *readings, int count)
{
    int err;
    char message[512]; /* Gyro messages are longer */
    size_t offset = 0;
    bool first = true;

    /* Format multiple readings with pipe separator */
    for (int i = 0; i < count && offset < sizeof(message) - 32; i++)
    {
        size_t len;
        char timestamp_str[24]; /* Dedicated buffer for timestamp */

        /* Format timestamp using the utility function */
        int ts_len = format_timestamp(readings[i].timestamp, timestamp_str, sizeof(timestamp_str));
        if (ts_len < 0)
        {
            printk("Error formatting timestamp\n");
            continue;
        }

        if (!first)
        {
            /* Add separator between readings */
            message[offset++] = '|';
        }

        /* Format: "{accel_x},{accel_y},{accel_z}/{gyro_x},{gyro_y},{gyro_z}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                       "%d,%d,%d/%d,%d,%d/%s",
                       readings[i].accel_x,
                       readings[i].accel_y,
                       readings[i].accel_z,
                       readings[i].gyro_x,
                       readings[i].gyro_y,
                       readings[i].gyro_z,
                       timestamp_str);

        if (len >= sizeof(message) - offset - 16)
        {
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }

        offset += len;
        first = false;
    }

    /* Debug output to verify format */
    printk("Gyro payload: %s\n", message);

    if (offset > 0)
    {
        /* Publish the message with QoS 2 */
        err = mqtt_client_publish(MQTT_TOPIC_GYRO, message, MQTT_QOS_2_EXACTLY_ONCE);
    }
    else
    {
        err = -EINVAL;
    }

    return err;
}

int mqtt_client_publish_config_confirm(const char *confirmation)
{
        return mqtt_client_publish(MQTT_TOPIC_CONFIG_CONFIRM, confirmation, MQTT_QOS_2_EXACTLY_ONCE);
}