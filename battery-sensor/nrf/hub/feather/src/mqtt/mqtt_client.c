/**
 * @file mqtt_client.c
 * @brief MQTT client module implementation
 */

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/random/rand32.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zephyr/logging/log.h>

#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <modem/modem_key_mgmt.h>

#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_publishers.h"
#include "config/config_module.h"
#include "certificates.h"

LOG_MODULE_REGISTER(mqtt_client, LOG_LEVEL_INF);
#define LOG_PREFIX_MQTT "[MQTT] "
#define LOG_PREFIX_LTE "[LTE] "
#define LOG_PREFIX_TLS "[TLS] "
#define LOG_PREFIX_NET "[NET] "

/* Buffers for MQTT client */
static uint8_t rx_buffer[APP_MQTT_BUFFER_SIZE];
static uint8_t tx_buffer[APP_MQTT_BUFFER_SIZE];
static uint8_t payload_buffer[APP_MQTT_BUFFER_SIZE];

/* The mqtt client struct */
static struct mqtt_client client_ctx;

/* MQTT Broker details */
static struct sockaddr_storage broker;

static struct zsock_pollfd fds[1];
static int nfds;

static bool mqtt_connected;

/* Calculate the length of the CA certificate */
static const size_t ca_certificate_len = sizeof(ca_certificate) - 1;

/* Mutex for protecting the MQTT client */
static K_MUTEX_DEFINE(mqtt_mutex);

/* LTE connection semaphore */
static K_SEM_DEFINE(lte_connected, 0, 1);

/* Forward declarations */
static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt);
static int get_mqtt_broker_addrinfo(void);
static int setup_certificates(void);

/* Flag for DNS resolution success */
static bool dns_resolved = false;
static char resolved_ip[INET_ADDRSTRLEN];

/* Handle LTE connection state */
static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
            (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            LOG_INF(LOG_PREFIX_LTE "Network registration status: %d", evt->nw_reg_status);
            k_sem_give(&lte_connected);
        }
        break;
    case LTE_LC_EVT_PSM_UPDATE:
        LOG_INF(LOG_PREFIX_LTE "PSM parameter update: TAU: %d, Active time: %d",
                evt->psm_cfg.tau, evt->psm_cfg.active_time);
        break;
    case LTE_LC_EVT_EDRX_UPDATE:
        LOG_INF(LOG_PREFIX_LTE "eDRX parameter update: eDRX: %f, PTW: %f",
                evt->edrx_cfg.edrx, evt->edrx_cfg.ptw);
        break;
    case LTE_LC_EVT_RRC_UPDATE:
        LOG_INF(LOG_PREFIX_LTE "RRC mode: %s", evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" : "Idle");
        break;
    case LTE_LC_EVT_CELL_UPDATE:
        LOG_INF(LOG_PREFIX_LTE "LTE cell changed: Cell ID: %d, Tracking area: %d",
                evt->cell.id, evt->cell.tac);
        break;
    case LTE_LC_EVT_LTE_MODE_UPDATE:
        LOG_INF(LOG_PREFIX_LTE "Active LTE mode changed: %s",
                evt->lte_mode == LTE_LC_LTE_MODE_NONE ? "None" : evt->lte_mode == LTE_LC_LTE_MODE_LTEM ? "LTE-M"
                                                             : evt->lte_mode == LTE_LC_LTE_MODE_NBIOT  ? "NB-IoT"
                                                                                                       : "Unknown");
        break;
    default:
        break;
    }
}

static int setup_lte(void)
{
    int err;

    LOG_INF(LOG_PREFIX_LTE "Setting up LTE connection...");

    /* Initialize LTE link controller */
    err = lte_lc_init();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to initialize LTE link controller, error: %d", err);
        return err;
    }

    /* Configure modem for LTE-M/NB-IoT */
    err = lte_lc_system_mode_set(LTE_LC_SYSTEM_MODE_LTEM_GPS,
                                 LTE_LC_SYSTEM_MODE_PREFER_LTEM);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to set system mode, error: %d", err);
        return err;
    }

    /* Set LTE offline for certificate provisioning */
    err = lte_lc_offline();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to set modem offline: %d", err);
        return err;
    }

    /* Set up certificates while offline */
    err = setup_certificates();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to set up certificates, error: %d", err);
        return err;
    }

    /* Register event handler */
    lte_lc_register_handler(lte_handler);

    LOG_INF(LOG_PREFIX_LTE "Waiting for LTE connection...");

    /* Connect to network asynchronously */
    err = lte_lc_connect_async(lte_handler);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to connect to LTE network, error: %d", err);
        return err;
    }

    /* Wait for network registration with a timeout */
    err = k_sem_take(&lte_connected, K_SECONDS(120));
    if (err == -EAGAIN)
    {
        LOG_ERR(LOG_PREFIX_LTE "Timeout waiting for LTE connection");
        return -ETIMEDOUT;
    }

    LOG_INF(LOG_PREFIX_LTE "LTE connected!");

    /* Additional check of registration status */
    enum lte_lc_nw_reg_status reg_status;
    err = lte_lc_nw_reg_status_get(&reg_status);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_LTE "Failed to get network registration status: %d", err);
    }
    else
    {
        LOG_INF(LOG_PREFIX_LTE "Network registration status: %d", reg_status);
    }

    return 0;
}

static int setup_certificates(void)
{
    int err;
    bool exists;

    LOG_INF(LOG_PREFIX_TLS "Checking for existing certificate...");

    /* Check if certificates already exist */
    err = modem_key_mgmt_exists(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, &exists);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_TLS "Failed to check for certificates, error: %d", err);
        return err;
    }

    if (exists)
    {
        /* Compare with existing certificate to see if it needs updating */
        err = modem_key_mgmt_cmp(SEC_TAG,
                                 MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                                 ca_certificate,
                                 ca_certificate_len);
        if (err == 0)
        {
            LOG_INF(LOG_PREFIX_TLS "Certificate already provisioned and matches");
            return 0;
        }

        LOG_INF(LOG_PREFIX_TLS "Certificate exists but doesn't match, updating...");
        /* Delete existing certificate */
        err = modem_key_mgmt_delete(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_TLS "Failed to delete existing certificate: %d", err);
            /* Continue anyway, as write might still succeed */
        }
    }
    else
    {
        LOG_INF(LOG_PREFIX_TLS "No existing certificate found, provisioning new one");
    }

    /* Provision certificates */
    LOG_INF(LOG_PREFIX_TLS "Provisioning certificate...");

    /* CA certificate */
    err = modem_key_mgmt_write(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                               ca_certificate, ca_certificate_len);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_TLS "Failed to provision CA certificate, error: %d", err);
        return err;
    }

    LOG_INF(LOG_PREFIX_TLS "Certificate provisioned successfully");
    return 0;
}

static int get_mqtt_broker_addrinfo(void)
{
    int err;
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM};
    char port[6];

    snprintf(port, sizeof(port), "%d", SERVER_PORT);

    LOG_INF(LOG_PREFIX_NET "Resolving hostname: %s", SERVER_HOST);

    err = getaddrinfo(SERVER_HOST, port, &hints, &result);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_NET "getaddrinfo() failed, error: %d", err);
        return -ECHILD;
    }

    if (result == NULL)
    {
        LOG_ERR(LOG_PREFIX_NET "Error: Address not found");
        return -ENOENT;
    }

    /* IPv4 Address. */
    struct sockaddr_in *server4 = ((struct sockaddr_in *)&broker);

    server4->sin_addr.s_addr =
        ((struct sockaddr_in *)result->ai_addr)->sin_addr.s_addr;
    server4->sin_family = AF_INET;
    server4->sin_port = htons(SERVER_PORT);

    /* Store the resolved IP address */
    inet_ntop(AF_INET, &server4->sin_addr, resolved_ip, sizeof(resolved_ip));
    LOG_INF(LOG_PREFIX_NET "Hostname %s resolved to %s", SERVER_HOST, resolved_ip);

    dns_resolved = true;
    freeaddrinfo(result);

    return 0;
}

static void prepare_fds(struct mqtt_client *client)
{
    if (client->transport.type == MQTT_TRANSPORT_SECURE)
    {
        fds[0].fd = client->transport.tls.sock;
    }
    else
    {
        fds[0].fd = client->transport.tcp.sock;
    }
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

    if (nfds > 0)
    {
        ret = zsock_poll(fds, nfds, timeout);
        if (ret < 0)
        {
            LOG_ERR(LOG_PREFIX_NET "poll error: %d", errno);
        }
    }

    return ret;
}

void mqtt_evt_handler(struct mqtt_client *const client,
                      const struct mqtt_evt *evt)
{
    int err;

    LOG_INF(LOG_PREFIX_MQTT "MQTT event type: %d, result: %d", evt->type, evt->result);

    switch (evt->type)
    {
    case MQTT_EVT_CONNACK:
        if (evt->result != 0)
        {
            LOG_ERR(LOG_PREFIX_MQTT "MQTT connection failed %d", evt->result);
            break;
        }

        mqtt_connected = true;
        LOG_INF(LOG_PREFIX_MQTT "MQTT client connected!");

        /* Subscribe to configuration topic - USING QoS 1 INSTEAD OF QoS 2 */
        struct mqtt_topic subscribe_topic = {
            .topic = {
                .utf8 = (uint8_t *)MQTT_TOPIC_CONFIG_SEND,
                .size = strlen(MQTT_TOPIC_CONFIG_SEND)},
            .qos = MQTT_QOS_1_AT_LEAST_ONCE}; /* Changed from QoS 2 to QoS 1 */

        const struct mqtt_subscription_list subscription_list = {
            .list = &subscribe_topic,
            .list_count = 1,
            .message_id = 1};

        err = mqtt_subscribe(client, &subscription_list);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Failed to subscribe to %s topic (err %d)",
                    MQTT_TOPIC_CONFIG_SEND, err);
        }
        else
        {
            LOG_INF(LOG_PREFIX_MQTT "Subscribed to topic: %s", MQTT_TOPIC_CONFIG_SEND);
        }

        break;

    case MQTT_EVT_DISCONNECT:
        LOG_INF(LOG_PREFIX_MQTT "MQTT client disconnected %d", evt->result);
        mqtt_connected = false;
        clear_fds();
        break;

    case MQTT_EVT_PUBLISH:
    {
        LOG_INF(LOG_PREFIX_MQTT "MQTT_EVT_PUBLISH received");

        const struct mqtt_publish_param *pub = &evt->param.publish;

        /* Print topic and payload information */
        if (pub->message.topic.topic.utf8 && pub->message.topic.topic.size > 0)
        {
            LOG_INF(LOG_PREFIX_MQTT "Topic: %.*s", pub->message.topic.topic.size, pub->message.topic.topic.utf8);
        }

        LOG_INF(LOG_PREFIX_MQTT "Payload length: %d", pub->message.payload.len);

        /* Check if this is a configuration message */
        bool is_config_message = false;
        if (pub->message.topic.topic.size == strlen(MQTT_TOPIC_CONFIG_SEND) &&
            strncmp((char *)pub->message.topic.topic.utf8, MQTT_TOPIC_CONFIG_SEND,
                    pub->message.topic.topic.size) == 0)
        {
            is_config_message = true;
        }

        /* Handle configuration messages */
        if (is_config_message && pub->message.payload.len > 0)
        {
            LOG_INF(LOG_PREFIX_MQTT "Config message detected, reading payload");

            /* Create buffer for payload */
            uint8_t payload_buf[256] = {0};
            int bytes_read;

            /* Use mqtt_read_publish_payload to read the payload data */
            bytes_read = mqtt_read_publish_payload(client, payload_buf,
                                                   pub->message.payload.len < sizeof(payload_buf) - 1 ? pub->message.payload.len : sizeof(payload_buf) - 1);

            if (bytes_read > 0)
            {
                /* Ensure NULL termination */
                payload_buf[bytes_read] = '\0';
                LOG_INF(LOG_PREFIX_MQTT "Read %d bytes of payload: '%s'", bytes_read, payload_buf);

                /* Process the command */
                err = config_process_command((char *)payload_buf);
                if (err == 0)
                {
                    LOG_INF(LOG_PREFIX_MQTT "Command processed successfully");

                    /* Get confirmation message */
                    char confirm_buf[256];
                    int confirm_len = config_get_confirmation(confirm_buf, sizeof(confirm_buf));

                    if (confirm_len > 0)
                    {
                        LOG_INF(LOG_PREFIX_MQTT "Sending confirmation: %s", confirm_buf);
                        err = mqtt_client_publish_config_confirm(confirm_buf);
                        if (err)
                        {
                            LOG_ERR(LOG_PREFIX_MQTT "Failed to publish confirmation: %d", err);
                        }
                        else
                        {
                            LOG_INF(LOG_PREFIX_MQTT "Confirmation published");
                        }
                    }
                    else
                    {
                        LOG_ERR(LOG_PREFIX_MQTT "No confirmation message available: %d", confirm_len);
                    }
                }
                else
                {
                    LOG_ERR(LOG_PREFIX_MQTT "Failed to process configuration command: %d", err);

                    /* Send error message - using a safer format with limited length */
                    char error_msg[128];
                    snprintf(error_msg, sizeof(error_msg), "Error processing command: %.80s", payload_buf);
                    err = mqtt_client_publish_config_confirm(error_msg);
                }
            }
            else
            {
                LOG_ERR(LOG_PREFIX_MQTT "Failed to read payload: %d", bytes_read);

                /* Send error message */
                const char *error_msg = "Error: Failed to read command payload";
                err = mqtt_client_publish_config_confirm(error_msg);
            }
        }

        /* Send acknowledgment for QoS 1 messages */
        if (pub->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE)
        {
            const struct mqtt_puback_param puback = {
                .message_id = pub->message_id};

            err = mqtt_publish_qos1_ack(client, &puback);
            if (err)
            {
                LOG_ERR(LOG_PREFIX_MQTT "Failed to send PUBACK: %d", err);
            }
        }
        else if (pub->message.topic.qos == MQTT_QOS_2_EXACTLY_ONCE)
        {
            const struct mqtt_pubrec_param pubrec = {
                .message_id = pub->message_id};

            err = mqtt_publish_qos2_receive(client, &pubrec);
            if (err)
            {
                LOG_ERR(LOG_PREFIX_MQTT "Failed to send PUBREC: %d", err);
            }
        }
    }
    break;

    case MQTT_EVT_PUBREC:
        LOG_INF(LOG_PREFIX_MQTT "PUBREC event received - id: %u, result: %d",
                evt->param.pubrec.message_id, evt->result);

        if (evt->result != 0)
        {
            LOG_ERR(LOG_PREFIX_MQTT "MQTT PUBREC error %d", evt->result);
            break;
        }

        /* For QoS 2, we need to send a PUBREL */
        const struct mqtt_pubrel_param rel_param = {
            .message_id = evt->param.pubrec.message_id};

        LOG_INF(LOG_PREFIX_MQTT "Sending PUBREL for message id: %u", rel_param.message_id);
        err = mqtt_publish_qos2_release(client, &rel_param);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Failed to send PUBREL: %d", err);
        }
        break;

    case MQTT_EVT_PUBREL:
        LOG_INF(LOG_PREFIX_MQTT "PUBREL event received - id: %u, result: %d",
                evt->param.pubrel.message_id, evt->result);

        if (evt->result != 0)
        {
            LOG_ERR(LOG_PREFIX_MQTT "MQTT PUBREL error %d", evt->result);
            break;
        }

        /* Send PUBCOMP in response to PUBREL */
        const struct mqtt_pubcomp_param comp_param = {
            .message_id = evt->param.pubrel.message_id};

        LOG_INF(LOG_PREFIX_MQTT "Sending PUBCOMP for message id: %u", comp_param.message_id);
        err = mqtt_publish_qos2_complete(client, &comp_param);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Failed to send PUBCOMP: %d", err);
        }
        break;

    case MQTT_EVT_PUBCOMP:
        LOG_INF(LOG_PREFIX_MQTT "PUBCOMP event received - id: %u, result: %d",
                evt->param.pubcomp.message_id, evt->result);

        if (evt->result != 0)
        {
            LOG_ERR(LOG_PREFIX_MQTT "MQTT PUBCOMP error %d", evt->result);
            break;
        }
        LOG_INF(LOG_PREFIX_MQTT "PUBCOMP packet id: %u", evt->param.pubcomp.message_id);
        break;

    case MQTT_EVT_SUBACK:
        LOG_INF(LOG_PREFIX_MQTT "SUBACK packet id: %u", evt->param.suback.message_id);
        break;

    case MQTT_EVT_PINGRESP:
        LOG_INF(LOG_PREFIX_MQTT "PINGRESP packet");
        break;

    default:
        LOG_INF(LOG_PREFIX_MQTT "Unhandled MQTT event: %d", evt->type);
        break;
    }
}

int elfryd_mqtt_client_init(void)
{
    int err;

    LOG_INF(LOG_PREFIX_MQTT "Initializing MQTT client...");

    /* Set up LTE connection */
    err = setup_lte();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to set up LTE connection, error: %d", err);
        return err;
    }

    /* Initialize broker address */
    err = get_mqtt_broker_addrinfo();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to get broker address, error: %d", err);
        return err;
    }

    /* Configure MQTT client */
    mqtt_client_init(&client_ctx);

    client_ctx.broker = &broker;
    client_ctx.evt_cb = mqtt_evt_handler;
    client_ctx.client_id.utf8 = (uint8_t *)MQTT_CLIENTID;
    client_ctx.client_id.size = strlen(MQTT_CLIENTID);
    client_ctx.password = NULL;
    client_ctx.user_name = NULL;
    client_ctx.protocol_version = MQTT_VERSION_3_1_1;
    client_ctx.transport.type = MQTT_TRANSPORT_SECURE;
    client_ctx.clean_session = 1;

    /* Set TLS configuration */
    struct mqtt_sec_config *tls_config = &client_ctx.transport.tls.config;
    tls_config->peer_verify = TLS_PEER_VERIFY_REQUIRED;
    tls_config->cipher_list = NULL;
    tls_config->cipher_count = 0;

    /* Create a static sec_tag_t array for the security tag */
    static sec_tag_t sec_tag_list[] = {SEC_TAG};
    tls_config->sec_tag_list = sec_tag_list;
    tls_config->sec_tag_count = 1;
    tls_config->hostname = SERVER_HOST;

    /* Setup MQTT buffers */
    client_ctx.rx_buf = rx_buffer;
    client_ctx.rx_buf_size = sizeof(rx_buffer);
    client_ctx.tx_buf = tx_buffer;
    client_ctx.tx_buf_size = sizeof(tx_buffer);

    LOG_INF(LOG_PREFIX_MQTT "MQTT client initialized");

    return 0;
}

int mqtt_client_connect(void)
{
    int err;
    int retry_count = 0;
    int max_retries = CONFIG_MQTT_CONNECT_ATTEMPTS;

    k_mutex_lock(&mqtt_mutex, K_FOREVER);

    if (mqtt_connected)
    {
        LOG_INF(LOG_PREFIX_MQTT "Already connected to MQTT broker");
        k_mutex_unlock(&mqtt_mutex);
        return 0;
    }

    /* Verify DNS resolution happened */
    if (!dns_resolved)
    {
        err = get_mqtt_broker_addrinfo();
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Failed to resolve MQTT broker hostname, error: %d", err);
            k_mutex_unlock(&mqtt_mutex);
            return err;
        }
    }

    /* Try connecting with retries */
    while (retry_count < max_retries && !mqtt_connected)
    {
        LOG_INF(LOG_PREFIX_MQTT "Attempting to connect to MQTT broker: %s:%d... (attempt %d/%d)",
                SERVER_HOST, SERVER_PORT, retry_count + 1, max_retries);

        err = mqtt_connect(&client_ctx);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Failed to connect to MQTT broker, error: %d", err);
            retry_count++;
            k_sleep(K_MSEC(CONFIG_MQTT_CONNECT_RETRY_DELAY_MS));
            continue;
        }

        prepare_fds(&client_ctx);

        /* Wait for the connection to complete */
        if (wait(APP_CONNECT_TIMEOUT_MS))
        {
            err = mqtt_input(&client_ctx);
            if (err)
            {
                LOG_ERR(LOG_PREFIX_MQTT "MQTT input error: %d", err);
            }
        }

        if (!mqtt_connected)
        {
            LOG_ERR(LOG_PREFIX_MQTT "MQTT connection failed or timed out");
            mqtt_abort(&client_ctx);
            retry_count++;
            k_sleep(K_MSEC(CONFIG_MQTT_CONNECT_RETRY_DELAY_MS));
        }
    }

    if (!mqtt_connected)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to connect to MQTT broker after %d attempts", max_retries);
        k_mutex_unlock(&mqtt_mutex);
        return -ETIMEDOUT;
    }

    /* Send a ping to verify the connection */
    err = mqtt_ping(&client_ctx);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to ping MQTT broker: %d", err);
        mqtt_abort(&client_ctx);
        mqtt_connected = false;
        k_mutex_unlock(&mqtt_mutex);
        return err;
    }

    /* Process the ping response */
    if (wait(APP_CONNECT_TIMEOUT_MS))
    {
        err = mqtt_input(&client_ctx);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Error processing ping response: %d", err);
            mqtt_abort(&client_ctx);
            mqtt_connected = false;
            k_mutex_unlock(&mqtt_mutex);
            return err;
        }
    }
    else
    {
        LOG_ERR(LOG_PREFIX_MQTT "Ping response timeout");
    }

    k_mutex_unlock(&mqtt_mutex);
    return 0;
}

int mqtt_client_disconnect(void)
{
    int err;

    k_mutex_lock(&mqtt_mutex, K_FOREVER);

    if (!mqtt_connected)
    {
        LOG_INF(LOG_PREFIX_MQTT "Not connected to MQTT broker");
        k_mutex_unlock(&mqtt_mutex);
        return 0;
    }

    err = mqtt_disconnect(&client_ctx);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to disconnect from MQTT broker, error: %d", err);
    }
    else
    {
        LOG_INF(LOG_PREFIX_MQTT "Disconnected from MQTT broker");
    }

    mqtt_connected = false;

    k_mutex_unlock(&mqtt_mutex);
    return err;
}

bool mqtt_client_is_connected(void)
{
    bool connected;

    k_mutex_lock(&mqtt_mutex, K_FOREVER);
    connected = mqtt_connected;
    k_mutex_unlock(&mqtt_mutex);

    return connected;
}

int mqtt_client_process(int timeout)
{
    int err;

    k_mutex_lock(&mqtt_mutex, K_FOREVER);

    if (!mqtt_connected)
    {
        k_mutex_unlock(&mqtt_mutex);
        return -ENOTCONN;
    }

    /* Process MQTT events */
    err = wait(timeout);
    if (err < 0)
    {
        k_mutex_unlock(&mqtt_mutex);
        return err;
    }

    err = mqtt_input(&client_ctx);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Error in MQTT input: %d", err);
        mqtt_abort(&client_ctx);
        mqtt_connected = false;
        clear_fds();
    }

    k_mutex_unlock(&mqtt_mutex);
    return err;
}

int mqtt_client_publish(const char *topic, const char *message, enum mqtt_qos qos)
{
    int err;

    k_mutex_lock(&mqtt_mutex, K_FOREVER);

    if (!mqtt_connected)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Not connected to MQTT broker");
        k_mutex_unlock(&mqtt_mutex);
        return -ENOTCONN;
    }

    /* Copy message to payload buffer */
    size_t message_len = strlen(message);
    if (message_len > sizeof(payload_buffer))
    {
        LOG_ERR(LOG_PREFIX_MQTT "Message too long for payload buffer");
        k_mutex_unlock(&mqtt_mutex);
        return -ENOMEM;
    }

    memcpy(payload_buffer, message, message_len);

    /* Prepare MQTT publish structure */
    struct mqtt_publish_param param = {
        .message.topic.qos = qos,
        .message.topic.topic.utf8 = (uint8_t *)topic,
        .message.topic.topic.size = strlen(topic),
        .message.payload.data = payload_buffer,
        .message.payload.len = message_len,
        .message_id = sys_rand32_get(),
        .dup_flag = 0,
        .retain_flag = 0};

    /* Publish the message */
    err = mqtt_publish(&client_ctx, &param);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to publish to %s, error: %d", topic, err);
    }

    k_mutex_unlock(&mqtt_mutex);
    return err;
}