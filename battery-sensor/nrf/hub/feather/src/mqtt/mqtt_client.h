/**
 * @file mqtt_client.h
 * @brief MQTT client interface for the Elfryd system
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include <zephyr/net/mqtt.h>

/* MQTT topic definitions from Kconfig */
#define MQTT_TOPIC_BATTERY CONFIG_MQTT_TOPIC_BATTERY
#define MQTT_TOPIC_TEMP CONFIG_MQTT_TOPIC_TEMP
#define MQTT_TOPIC_GYRO CONFIG_MQTT_TOPIC_GYRO
#define MQTT_TOPIC_CONFIG_SEND CONFIG_MQTT_TOPIC_CONFIG_SEND
#define MQTT_TOPIC_CONFIG_CONFIRM CONFIG_MQTT_TOPIC_CONFIG_CONFIRM

/* MQTT broker details (from Kconfig) */
#define SERVER_HOST CONFIG_MQTT_BROKER_HOSTNAME
#define SERVER_PORT CONFIG_MQTT_BROKER_PORT
#define MQTT_CLIENTID CONFIG_MQTT_CLIENT_ID
#define SEC_TAG CONFIG_MQTT_TLS_SEC_TAG

#define APP_MQTT_BUFFER_SIZE CONFIG_MQTT_BUFFER_SIZE
#define APP_CONNECT_TIMEOUT_MS CONFIG_MQTT_CONNECT_TIMEOUT_MS

/**
 * Initialize the MQTT client and connect to the broker
 *
 * @return 0 on success, negative error code on failure
 */
int elfryd_mqtt_client_init(void);

/**
 * Connect to the MQTT broker
 *
 * @return 0 on success, negative error code on failure
 */
int mqtt_client_connect(void);

/**
 * Disconnect from the MQTT broker
 *
 * @return 0 on success, negative error code on failure
 */
int mqtt_client_disconnect(void);

/**
 * Check if connected to the MQTT broker
 *
 * @return True if connected, false otherwise
 */
bool mqtt_client_is_connected(void);

/**
 * Process MQTT events
 *
 * @param timeout Timeout in milliseconds for event processing
 * @return 0 on success, negative error code on failure
 */
int mqtt_client_process(int timeout);

/**
 * Publish a message to the MQTT broker (internal function)
 *
 * @param topic   Topic to publish the message to
 * @param message Message to publish
 * @param qos     MQTT QoS level
 * @return        0 on success, negative error code on failure
 */
int mqtt_client_publish(const char *topic, const char *message, enum mqtt_qos qos);

#endif /* MQTT_CLIENT_H */