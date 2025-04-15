/**
 * @file mqtt_publishers.h
 * @brief MQTT sensor data publishing interface
 */

#ifndef MQTT_PUBLISHERS_H
#define MQTT_PUBLISHERS_H

#include "sensors/sensors.h"

/**
 * Publish battery readings to the MQTT broker
 *
 * @param readings Array of battery readings
 * @param count    Number of readings in the array
 * @return         0 on success, negative error code on failure
 */
int mqtt_client_publish_battery(battery_reading_t *readings, int count);

/**
 * Publish temperature readings to the MQTT broker
 *
 * @param readings Array of temperature readings
 * @param count    Number of readings in the array
 * @return         0 on success, negative error code on failure
 */
int mqtt_client_publish_temp(temp_reading_t *readings, int count);

/**
 * Publish gyroscope readings to the MQTT broker
 *
 * @param readings Array of gyroscope readings
 * @param count    Number of readings in the array
 * @return         0 on success, negative error code on failure
 */
int mqtt_client_publish_gyro(gyro_reading_t *readings, int count);

/**
 * Publish configuration confirmation to the MQTT broker
 *
 * @param confirmation Configuration confirmation message
 * @return             0 on success, negative error code on failure
 */
int mqtt_client_publish_config_confirm(const char *confirmation);

#endif /* MQTT_PUBLISHERS_H */