#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <zephyr/random/rand32.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>
#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* The mqtt client struct */
extern struct mqtt_client client;

/* TLS configuration */
extern sec_tag_t sec_tag_list[1];

/* Buffers for MQTT client. */
extern uint8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
extern uint8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
extern uint8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

/* MQTT Broker details. */
extern struct sockaddr_storage broker;

/* File descriptor */
extern struct pollfd fds;

/* Struct for username & password */
extern struct mqtt_utf8 user_name;
extern struct mqtt_utf8 password;

/**@brief Function to print strings without null-termination
 */
void data_print(uint8_t *prefix, uint8_t *data, size_t len);

/**@brief Function to publish data on the configured topic*/
int data_publish(struct mqtt_client *c, enum mqtt_qos qos, uint8_t *data, size_t len);

/**@brief Function to subscribe to the configured topic*/
int subscribe(void);

/**@brief Function to read the published payload.*/
int publish_get_payload(struct mqtt_client *c, size_t length);

/**@brief Deletes and provisions certificates*/
int certificates_provision(void);

/**@brief Recoverable modem library error. */
void nrf_modem_recoverable_error_handler(uint32_t err);

/**@brief Function to publish errors to mqtt broker*/
int publish_error(const char *error, int error_code);

/**@brief MQTT client event handler
 */
void mqtt_evt_handler(struct mqtt_client *const c, const struct mqtt_evt *evt);

/**@brief Resolves the configured hostname and
 * initializes the MQTT broker structure
 */
int broker_init(void);

/**@brief Initialize the MQTT client structure
 */
int client_init(struct mqtt_client *client);

/**@brief Initialize the file descriptor structure used by poll.
 */
int fds_init(struct mqtt_client *c);

/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
int modem_configure(void);