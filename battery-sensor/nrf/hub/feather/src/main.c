/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_mqtt_lte_publisher, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/random/rand32.h>

#include <string.h>
#include <stdio.h>  // Add this line for snprintf
#include <errno.h>

// LTE connectivity
#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <modem/modem_key_mgmt.h>

#include "config.h"
#include "certificates.h"

/* Buffers for MQTT client. */
static uint8_t rx_buffer[CONFIG_MQTT_BUFFER_SIZE];
static uint8_t tx_buffer[CONFIG_MQTT_BUFFER_SIZE];

/* The mqtt client struct */
static struct mqtt_client client_ctx;

/* MQTT Broker details. */
static struct sockaddr_storage broker;

static struct zsock_pollfd fds[1];
static int nfds;

static bool connected;

/* LTE connection ready semaphore */
static K_SEM_DEFINE(lte_connected, 0, 1);

/* Function forward declarations */
static void prepare_fds(struct mqtt_client *client);
static void clear_fds(void);
static int wait(int timeout);
static void broker_init(void);
static void client_init(struct mqtt_client *client);
static int try_to_connect(struct mqtt_client *client);
static int process_mqtt_and_sleep(struct mqtt_client *client, int timeout);
static int publish(struct mqtt_client *client, enum mqtt_qos qos);
static void mqtt_evt_handler(struct mqtt_client *const client, const struct mqtt_evt *evt);
static int provision_certificate(void);

static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type) {
        case LTE_LC_EVT_NW_REG_STATUS:
            if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
                (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING)) {
                LOG_INF("Network registration status: %d", evt->nw_reg_status);
                k_sem_give(&lte_connected);
            }
            break;
        default:
            break;
    }
}

static int provision_certificate(void)
{
    int err;
    bool exists = false;
    size_t ca_certificate_len = sizeof(ca_certificate) - 1;

    LOG_INF("Checking for existing certificate...");
    
    // Check if certificate already exists
    err = modem_key_mgmt_exists(CONFIG_MQTT_TLS_SEC_TAG, 
                              MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                              &exists);
    if (err) {
        LOG_ERR("Failed to check for certificate: %d", err);
        return err;
    }
    
    // If certificate exists and matches, no need to provision again
    if (exists) {
        // Compare with existing certificate (optional)
        err = modem_key_mgmt_cmp(CONFIG_MQTT_TLS_SEC_TAG,
                              MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                              ca_certificate,
                              ca_certificate_len);
        if (err == 0) {
            LOG_INF("Certificate already provisioned and matches");
            return 0;
        }
        
        LOG_INF("Certificate exists but doesn't match, updating...");
        // Delete existing certificate
        err = modem_key_mgmt_delete(CONFIG_MQTT_TLS_SEC_TAG, 
                                  MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
        if (err) {
            LOG_ERR("Failed to delete existing certificate: %d", err);
            // Continue anyway, as write might still succeed
        }
    } else {
        LOG_INF("No existing certificate found, provisioning new one");
    }
    
    // Provision the certificate
    err = modem_key_mgmt_write(CONFIG_MQTT_TLS_SEC_TAG, 
                              MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, 
                              ca_certificate, 
                              ca_certificate_len);
    if (err) {
        LOG_ERR("Failed to provision certificate: %d", err);
        return err;
    }

    LOG_INF("Certificate provisioned successfully");
    return 0;
}

static void prepare_fds(struct mqtt_client *client)
{
    // For the nRF91, we need to use the TLS socket
    fds[0].fd = client->transport.tls.sock;
    fds[0].events = ZSOCK_POLLIN;
    nfds = 1;
}

static void clear_fds(void)
{
    nfds = 0;
}

static int wait(int timeout)
{
    int ret = 0;

    if (nfds > 0) {
        ret = zsock_poll(fds, nfds, timeout);
        if (ret < 0) {
            LOG_ERR("poll error: %d", errno);
        }
    }

    return ret;
}

void mqtt_evt_handler(struct mqtt_client *const client,
              const struct mqtt_evt *evt)
{
    int err;

    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->result != 0) {
            LOG_ERR("MQTT connect failed %d", evt->result);
            break;
        }

        connected = true;
        LOG_INF("MQTT client connected!");

		

        break;

    case MQTT_EVT_DISCONNECT:
        LOG_INF("MQTT client disconnected %d", evt->result);

        connected = false;
        clear_fds();

        break;

    case MQTT_EVT_PUBACK:
        if (evt->result != 0) {
            LOG_ERR("MQTT PUBACK error %d", evt->result);
            break;
        }

        LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);

        break;

    case MQTT_EVT_PUBREC:
        if (evt->result != 0) {
            LOG_ERR("MQTT PUBREC error %d", evt->result);
            break;
        }

        LOG_INF("PUBREC packet id: %u", evt->param.pubrec.message_id);

        const struct mqtt_pubrel_param rel_param = {
            .message_id = evt->param.pubrec.message_id
        };

        err = mqtt_publish_qos2_release(client, &rel_param);
        if (err != 0) {
            LOG_ERR("Failed to send MQTT PUBREL: %d", err);
        }

        break;

    case MQTT_EVT_PUBCOMP:
        if (evt->result != 0) {
            LOG_ERR("MQTT PUBCOMP error %d", evt->result);
            break;
        }

        LOG_INF("PUBCOMP packet id: %u",
            evt->param.pubcomp.message_id);

        break;

    case MQTT_EVT_PINGRESP:
        LOG_INF("PINGRESP packet");
        break;

    default:
        break;
    }
}

static char *get_mqtt_payload(enum mqtt_qos qos)
{
    static char payload[100];
    snprintk(payload, sizeof(payload), "{\"message\":\"Hello LTE MQTT\",\"QoS\":\"%d\",\"uptime\":%lld}", 
             qos, k_uptime_get() / 1000);
    return payload;
}

static char *get_mqtt_topic(void)
{
    return CONFIG_MQTT_TOPIC;
}

#define RC_STR(rc) ((rc) == 0 ? "OK" : "ERROR")
#define PRINT_RESULT(func, rc) LOG_INF("%s: %d <%s>", (func), rc, RC_STR(rc))

static void broker_init(void)
{
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    char port_str[6];
    int err;
    
    snprintf(port_str, sizeof(port_str), "%d", CONFIG_MQTT_BROKER_PORT);
    
    // Resolve the broker address
    err = zsock_getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, port_str, &hints, &result);
    if (err) {
        LOG_ERR("getaddrinfo() failed: %d", err);
        return;
    }
    
    // Copy the resolved address to our broker address
    memcpy(&broker, result->ai_addr, result->ai_addrlen);
    
    zsock_freeaddrinfo(result);
}

static void client_init(struct mqtt_client *client)
{
    mqtt_client_init(client);

    broker_init();

    /* MQTT client configuration */
    client->broker = &broker;
    client->evt_cb = mqtt_evt_handler;
    client->client_id.utf8 = (uint8_t *)CONFIG_MQTT_CLIENT_ID;
    client->client_id.size = strlen(CONFIG_MQTT_CLIENT_ID);
    client->password = NULL;
    client->user_name = NULL;
    client->protocol_version = MQTT_VERSION_3_1_1;

    // Set clean session to true for public brokers
    client->clean_session = 1;

    /* MQTT buffers configuration */
    client->rx_buf = rx_buffer;
    client->rx_buf_size = sizeof(rx_buffer);
    client->tx_buf = tx_buffer;
    client->tx_buf_size = sizeof(tx_buffer);

    /* MQTT transport configuration */
    client->transport.type = MQTT_TRANSPORT_SECURE;
    
    struct mqtt_sec_config *tls_config = &client->transport.tls.config;
    
    // Fix: Create a static variable for the sec_tag
    static int sec_tag = CONFIG_MQTT_TLS_SEC_TAG;
    
    tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
    tls_config->cipher_list = NULL;
    tls_config->cipher_count = 0;
    tls_config->sec_tag_count = 1;
    tls_config->sec_tag_list = &sec_tag; // Point to the static variable
    tls_config->hostname = CONFIG_MQTT_BROKER_HOSTNAME;
}

static int publish(struct mqtt_client *client, enum mqtt_qos qos)
{
    struct mqtt_publish_param param;

    param.message.topic.qos = qos;
    param.message.topic.topic.utf8 = (uint8_t *)get_mqtt_topic();
    param.message.topic.topic.size = strlen(param.message.topic.topic.utf8);
    param.message.payload.data = get_mqtt_payload(qos);
    param.message.payload.len = strlen(param.message.payload.data);
    param.message_id = sys_rand32_get();
    param.dup_flag = 0U;
    param.retain_flag = 0U;

    return mqtt_publish(client, &param);
}

static int try_to_connect(struct mqtt_client *client)
{
    int rc, i = 0;

    while (i++ < CONFIG_MQTT_CONNECT_ATTEMPTS && !connected) {
        client_init(client);

        LOG_INF("Attempting to connect to MQTT broker: %s:%d...", 
                CONFIG_MQTT_BROKER_HOSTNAME, CONFIG_MQTT_BROKER_PORT);
        
        rc = mqtt_connect(client);
        if (rc != 0) {
            PRINT_RESULT("mqtt_connect", rc);
            k_sleep(K_MSEC(CONFIG_MQTT_CONNECT_RETRY_DELAY_MS));
            continue;
        }

        prepare_fds(client);

        if (wait(CONFIG_MQTT_CONNECT_TIMEOUT_MS)) {
            mqtt_input(client);
        }

        if (!connected) {
            mqtt_abort(client);
        }
    }

    if (connected) {
        return 0;
    }

    return -EINVAL;
}

static int process_mqtt_and_sleep(struct mqtt_client *client, int timeout)
{
    int64_t remaining = timeout;
    int64_t start_time = k_uptime_get();
    int rc;

    while (remaining > 0 && connected) {
        if (wait(remaining)) {
            rc = mqtt_input(client);
            if (rc != 0) {
                PRINT_RESULT("mqtt_input", rc);
                return rc;
            }
        }

        rc = mqtt_live(client);
        if (rc != 0 && rc != -EAGAIN) {
            PRINT_RESULT("mqtt_live", rc);
            return rc;
        } else if (rc == 0) {
            rc = mqtt_input(client);
            if (rc != 0) {
                PRINT_RESULT("mqtt_input", rc);
                return rc;
            }
        }

        remaining = timeout + start_time - k_uptime_get();
    }

    return 0;
}

#define SUCCESS_OR_EXIT(rc) { if (rc != 0) { return 1; } }
#define SUCCESS_OR_BREAK(rc) { if (rc != 0) { break; } }

static int publisher(void)
{
    int i, rc, r = 0;

    LOG_INF("Attempting to connect to MQTT broker");
    rc = try_to_connect(&client_ctx);
    PRINT_RESULT("try_to_connect", rc);
    SUCCESS_OR_EXIT(rc);

    i = 0;
    while (i++ < CONFIG_MQTT_MESSAGE_COUNT && connected) {
        r = -1;

        rc = mqtt_ping(&client_ctx);
        PRINT_RESULT("mqtt_ping", rc);
        SUCCESS_OR_BREAK(rc);

        rc = process_mqtt_and_sleep(&client_ctx, CONFIG_MQTT_SLEEP_MS);
        SUCCESS_OR_BREAK(rc);

        rc = publish(&client_ctx, MQTT_QOS_0_AT_MOST_ONCE);
        PRINT_RESULT("mqtt_publish QoS 0", rc);
        SUCCESS_OR_BREAK(rc);

        rc = process_mqtt_and_sleep(&client_ctx, CONFIG_MQTT_SLEEP_MS);
        SUCCESS_OR_BREAK(rc);

        rc = publish(&client_ctx, MQTT_QOS_1_AT_LEAST_ONCE);
        PRINT_RESULT("mqtt_publish QoS 1", rc);
        SUCCESS_OR_BREAK(rc);

        rc = process_mqtt_and_sleep(&client_ctx, CONFIG_MQTT_SLEEP_MS);
        SUCCESS_OR_BREAK(rc);

        rc = publish(&client_ctx, MQTT_QOS_2_EXACTLY_ONCE);
        PRINT_RESULT("mqtt_publish QoS 2", rc);
        SUCCESS_OR_BREAK(rc);

        rc = process_mqtt_and_sleep(&client_ctx, CONFIG_MQTT_SLEEP_MS);
        SUCCESS_OR_BREAK(rc);

        r = 0;
    }

    rc = mqtt_disconnect(&client_ctx);
    PRINT_RESULT("mqtt_disconnect", rc);

    LOG_INF("MQTT session finished");

    return r;
}

// Add to main.c after LTE connection is established
static int verify_connection_and_dns(void) 
{
    int err;
    struct sockaddr_in addr;
    char ip_addr[INET_ADDRSTRLEN];
    struct zsock_addrinfo *ai; // Declare the variable here
    struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    // Check if we can resolve the MQTT broker hostname
    LOG_INF("Resolving hostname: %s", CONFIG_MQTT_BROKER_HOSTNAME);
    
    err = zsock_getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, NULL, &hints, &ai);
    if (err) {
        LOG_ERR("Failed to resolve hostname, err: %d", err);
        LOG_ERR("Please check DNS server configuration");
        return -ENOENT;
    }
    
    // Print the resolved IP address
    addr = *((struct sockaddr_in *)ai->ai_addr);
    inet_ntop(AF_INET, &addr.sin_addr, ip_addr, sizeof(ip_addr));
    LOG_INF("Hostname %s resolved to %s", CONFIG_MQTT_BROKER_HOSTNAME, ip_addr);
    
    zsock_freeaddrinfo(ai);
    return 0;
}

int main(void)
{
    int err;
    
    // Initialize the modem library first
    err = nrf_modem_lib_init();
    if (err) {
        LOG_ERR("Failed to initialize modem library, err: %d", err);
        return err;
    }

    // Initialize LTE link control without connecting
    err = lte_lc_init();
    if (err) {
        LOG_ERR("Failed to initialize LTE link control, err: %d", err);
        return err;
    }
    
    // Make sure modem is in offline mode before provisioning
    err = lte_lc_offline();
    if (err) {
        LOG_ERR("Failed to set modem offline: %d", err);
        return err;
    }
    
    // Provision the TLS certificate while offline
    err = provision_certificate();
    if (err) {
        LOG_ERR("Failed to provision certificate");
        return err;
    }
    
    // Now connect to the LTE network
    err = lte_lc_connect_async(lte_handler);
    if (err) {
        LOG_ERR("Failed to connect to LTE network, err: %d", err);
        return err;
    }
    
    // Wait for network registration
    LOG_INF("Waiting for LTE connection...");
    err = k_sem_take(&lte_connected, K_SECONDS(120));
    if (err == -EAGAIN) {
        LOG_ERR("Timeout waiting for LTE connection");
        return err;
    }
    
    // Print network information
    LOG_INF("LTE connected!");
    
    // Run some network diagnostics
    enum lte_lc_nw_reg_status reg_status;
    err = lte_lc_nw_reg_status_get(&reg_status);
    if (err) {
        LOG_ERR("Failed to get network registration status: %d", err);
    } else {
        LOG_INF("Network registration status: %d", reg_status);
    }

	// After LTE connection and network diagnostics
	err = verify_connection_and_dns();
	if (err) {
		LOG_ERR("DNS resolution failed, check network and DNS configuration");
		return err;
	}
    
    // Start MQTT publisher
    err = publisher();
    if (err) {
        LOG_ERR("MQTT publishing failed");
        return err;
    }
    
    LOG_INF("MQTT LTE sample completed successfully");
    return 0;
}