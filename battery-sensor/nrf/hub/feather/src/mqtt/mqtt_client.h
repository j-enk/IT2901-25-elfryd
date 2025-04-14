/**
 * @file mqtt_client.h
 * @brief MQTT client interface for the Elfryd system
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include <zephyr/net/mqtt.h>

/* MQTT topic definitions */
#define MQTT_TOPIC_BATTERY "elfryd/battery"
#define MQTT_TOPIC_TEMP "elfryd/temp"
#define MQTT_TOPIC_GYRO "elfryd/gyro"
#define MQTT_TOPIC_CONFIG_SEND "elfryd/config/send"
#define MQTT_TOPIC_CONFIG_CONFIRM "elfryd/config/confirm"

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