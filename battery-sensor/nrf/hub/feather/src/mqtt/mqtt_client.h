/**
 * @file mqtt_client.h
 * @brief MQTT client module for Elfryd hub
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <zephyr/kernel.h>
#include <zephyr/net/mqtt.h>
#include <stdbool.h>
#include "sensors/sensors.h"

/** MQTT Topic definitions */
#define MQTT_TOPIC_BATTERY "elfryd/battery"
#define MQTT_TOPIC_TEMP "elfryd/temp"
#define MQTT_TOPIC_GYRO "elfryd/gyro"
#define MQTT_TOPIC_CONFIG_SEND "elfryd/config/send"
#define MQTT_TOPIC_CONFIG_CONFIRM "elfryd/config/confirm"

/**
 * @brief Initialize the MQTT client
 * 
 * @return 0 on success, negative errno code on failure
 */
int elfryd_mqtt_client_init(void);

/**
 * @brief Connect to the MQTT broker
 * 
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_connect(void);

/**
 * @brief Disconnect from the MQTT broker
 * 
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_disconnect(void);

/**
 * @brief Check if connected to the MQTT broker
 * 
 * @return true if connected, false otherwise
 */
bool mqtt_client_is_connected(void);

/**
 * @brief Publish battery readings to the MQTT broker
 * 
 * @param readings Array of battery readings to publish
 * @param count Number of readings in the array
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_publish_battery(battery_reading_t *readings, int count);

/**
 * @brief Publish temperature readings to the MQTT broker
 * 
 * @param readings Array of temperature readings to publish
 * @param count Number of readings in the array
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_publish_temp(temp_reading_t *readings, int count);

/**
 * @brief Publish gyroscope readings to the MQTT broker
 * 
 * @param readings Array of gyroscope readings to publish
 * @param count Number of readings in the array
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_publish_gyro(gyro_reading_t *readings, int count);

/**
 * @brief Publish a configuration confirmation to the MQTT broker
 * 
 * @param confirmation Configuration confirmation message
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_publish_config_confirm(const char *confirmation);

/**
 * @brief Process MQTT events
 * 
 * @param timeout Timeout in milliseconds to wait for events
 * @return 0 on success, negative errno code on failure
 */
int mqtt_client_process(int timeout);

#endif /* MQTT_CLIENT_H */