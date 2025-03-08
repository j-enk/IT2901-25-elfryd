#include "nfc_helpers.h"
#include "mqtt_helpers.h"
#include "certificates.h"
#include "gpio_utils.h"

#if defined(CONFIG_MODEM_KEY_MGMT)
#include <modem/modem_key_mgmt.h>
#endif

LOG_MODULE_DECLARE(mqtt_simple, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

/* COMMANDS */
#define CMD_OPEN "OPEN"
#define CMD_CLOSE "CLOSE"
#define CMD_KEY_STATUS "KEY_STATUS"
#define CMD_PING "PING"
#define CMD_LOCK_STATUS "LOCK_STATUS"

struct mqtt_client client;

#if defined CONFIG_MQTT_LIB_TLS
sec_tag_t sec_tag_list[] = {CONFIG_MQTT_TLS_SEC_TAG};
#endif /* CONFIG_MQTT_LIB_TLS */

uint8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
uint8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
uint8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

struct sockaddr_storage broker;

struct pollfd fds;

struct mqtt_utf8 user_name;
struct mqtt_utf8 password;

void data_print(uint8_t *prefix, uint8_t *data, size_t len)
{
    char buf[len + 1];

    memcpy(buf, data, len);
    buf[len] = 0;
    LOG_INF("%s%s", prefix, buf);
}

int data_publish(struct mqtt_client *c, enum mqtt_qos qos,
                 uint8_t *data, size_t len)
{
    struct mqtt_publish_param param;

    param.message.topic.qos = qos;
    param.message.topic.topic.utf8 = CONFIG_MQTT_PUB_TOPIC;
    param.message.topic.topic.size = strlen(CONFIG_MQTT_PUB_TOPIC);
    param.message.payload.data = data;
    param.message.payload.len = len;
    param.message_id = sys_rand32_get();
    param.dup_flag = 0;
    param.retain_flag = 0;

    // printk("Publishing:%s ", data);

    LOG_INF("to topic: %s len: %u",
            CONFIG_MQTT_PUB_TOPIC,
            (unsigned int)strlen(CONFIG_MQTT_PUB_TOPIC));

    return mqtt_publish(c, &param);
}

int subscribe(void)
{
    struct mqtt_topic subscribe_topic = {
        .topic = {
            .utf8 = CONFIG_MQTT_SUB_TOPIC,
            .size = strlen(CONFIG_MQTT_SUB_TOPIC)},
        .qos = MQTT_QOS_1_AT_LEAST_ONCE};

    const struct mqtt_subscription_list subscription_list = {
        .list = &subscribe_topic,
        .list_count = 1,
        .message_id = 1234};

    LOG_INF("Subscribing to: %s len %u", CONFIG_MQTT_SUB_TOPIC,
            (unsigned int)strlen(CONFIG_MQTT_SUB_TOPIC));

    return mqtt_subscribe(&client, &subscription_list);
}

int publish_get_payload(struct mqtt_client *c, size_t length)
{
    int ret;
    int err = 0;

    /* Return an error if the payload is larger than the payload buffer.
     * Note: To allow new messages, we have to read the payload before returning.
     */
    if (length > sizeof(payload_buf))
    {
        err = -EMSGSIZE;
    }

    /* Truncate payload until it fits in the payload buffer. */
    while (length > sizeof(payload_buf))
    {
        ret = mqtt_read_publish_payload_blocking(
            c, payload_buf, (length - sizeof(payload_buf)));
        if (ret == 0)
        {
            return -EIO;
        }
        else if (ret < 0)
        {
            return ret;
        }

        length -= ret;
    }

    ret = mqtt_readall_publish_payload(c, payload_buf, length);
    if (ret)
    {
        return ret;
    }

    return err;
}

#if defined(CONFIG_MQTT_LIB_TLS)
int certificates_provision(void)
{
    int err = 0;
    /* Delete Certificates */
    nrf_sec_tag_t sec_tag = (nrf_sec_tag_t)sec_tag_list[0];

    for (int type = 0; type < 5; type++)
    {
        LOG_INF("Deleting certs sec_tag: %d", sec_tag);
        err = modem_key_mgmt_delete(sec_tag, type);
        LOG_INF("nrf_inbuilt_key_delete(%u, %d) => result%d",
                sec_tag, type, err);
    }

#if defined(CONFIG_NRF_MODEM_LIB) && defined(CONFIG_MODEM_KEY_MGMT)

    LOG_INF("Provisioning CA certificate");
    err = modem_key_mgmt_write(CONFIG_MQTT_TLS_SEC_TAG,
                               MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                               CA_CERTIFICATE,
                               strlen(CA_CERTIFICATE));
    if (err)
    {
        LOG_ERR("Failed to provision CA certificate: %d", err);
        return err;
    }

#endif

#if defined(CLIENT_CERTIFICATE)

    LOG_INF("Provisioning client certificate");
    err = modem_key_mgmt_write(CONFIG_MQTT_TLS_SEC_TAG,
                               MODEM_KEY_MGMT_CRED_TYPE_PUBLIC_CERT,
                               CLIENT_CERTIFICATE,
                               strlen(CLIENT_CERTIFICATE));

    if (err)
    {
        LOG_ERR("Failed to provision client certificate: %d", err);
        return err;
    }
#endif

#if defined(CLIENT_PRIVATE_KEY)

    LOG_INF("Provisioning client private key");
    err = modem_key_mgmt_write(CONFIG_MQTT_TLS_SEC_TAG,
                               MODEM_KEY_MGMT_CRED_TYPE_PRIVATE_CERT,
                               CLIENT_PRIVATE_KEY,
                               strlen(CLIENT_PRIVATE_KEY));

    if (err)
    {
        LOG_ERR("Failed to provision client private key: %d", err);
        return err;
    }
#endif
    return err;
}
#endif /* defined(CONFIG_MQTT_LIB_TLS) */

#if defined(CONFIG_NRF_MODEM_LIB)
void nrf_modem_recoverable_error_handler(uint32_t err)
{
    LOG_ERR("Modem library recoverable error: %u", (unsigned int)err);
}
#endif /* defined(CONFIG_NRF_MODEM_LIB) */

int publish_error(const char *error, int error_code)
{
    LOG_INF("Publishing error: %s", error);

    char error_msg[100];
    sprintf(error_msg, "%d?ERROR=%s, %d", CONFIG_LOCKBOX_ID, error, error_code);

    int ret = data_publish(&client,
                           MQTT_QOS_1_AT_LEAST_ONCE,
                           error_msg,
                           strlen(error_msg));
    if (ret)
    {
        LOG_ERR("Failed to publish error: %d", ret);
    }
    else
    {
        LOG_INF("Error published");
    }

    return ret;
}
#define OPEN_THREAD_STACK_SIZE 1024
#define OPEN_THREAD_PRIORITY 4

bool thread_running = false;

K_THREAD_STACK_DEFINE(open_stack_area, OPEN_THREAD_STACK_SIZE);
struct k_thread open_thread_data;

void open_event_thread(void *arg1, void *arg2, void *arg3)
{
    LOG_INF("Opening lockbox");
    thread_running = true;

    int ret = 0;
    ret = gpio_pin_set_dt(&led_3, 1);
    k_sleep(K_SECONDS(5));
    ret = gpio_pin_set_dt(&led_3, 0);

    if (ret < 0)
    {
        ret = publish_error("Error opening lockbox", ret);
    }
    thread_running = false;
}

int open_lockbox()
{
    if (thread_running)
    {
        LOG_INF("Open thread already running");
        return 0;
    }

    k_tid_t tid = k_thread_create(
        &open_thread_data,
        open_stack_area,
        K_THREAD_STACK_SIZEOF(open_stack_area),
        open_event_thread,
        NULL, NULL, NULL,
        OPEN_THREAD_PRIORITY, 0, K_NO_WAIT);

    if (tid == 0)
    {
        LOG_ERR("Failed to create open thread");
    }

    return tid;
}

int close_lockbox()
{
    LOG_INF("Closing lockbox");

    int ret = 0;
    ret = gpio_pin_set_dt(&led_3, 0);
    if (ret < 0)
    {
        ret = publish_error("Error closing lockbox", ret);
    }
    return ret;
}

int publish_key_status()
{
    LOG_INF("Finding key status...");
    bool key_status = key_present_state;
    LOG_INF("Key status: %d", key_status);

    char data[15];

    sprintf(data, "%d?KEY_STATUS=%s\0", CONFIG_LOCKBOX_ID, key_status ? "1" : "0");

    int ret = data_publish(&client,
                           MQTT_QOS_1_AT_LEAST_ONCE,
                           data,
                           strlen(data));
    if (ret)
    {
        LOG_ERR("Failed to report key status: %d", ret);
    }
    else
    {
        LOG_INF("Key status reported");
    }

    return ret;
}

int pong()
{
    LOG_INF("Ponging...");

    char data[15];
    sprintf(data, "%d?PONG\0", CONFIG_LOCKBOX_ID);

    int ret = data_publish(&client,
                           MQTT_QOS_1_AT_LEAST_ONCE,
                           data,
                           strlen(data));
    if (ret)
    {
        LOG_ERR("Failed to pong: %d", ret);
    }
    else
    {
        LOG_INF("Pong sent");
    }

    return ret;
}

int publish_lock_status()
{
    LOG_INF("Finding lock status...");
    bool lock_status = lock_pressed_state;
    LOG_INF("lock status: %d", lock_status);

    char data[25];

    sprintf(data, "%d?LOCK_STATUS=%s\0", CONFIG_LOCKBOX_ID, lock_status ? "1" : "0");

    int ret = data_publish(&client,
                           MQTT_QOS_1_AT_LEAST_ONCE,
                           data,
                           strlen(data));
    if (ret)
    {
        LOG_ERR("Failed to report lock status: %d", ret);
    }
    else
    {
        LOG_INF("Lock status reported");
    }

    return ret;
}

void mqtt_evt_handler(struct mqtt_client *const c,
                      const struct mqtt_evt *evt)
{
    int err;

    switch (evt->type)
    {
    case MQTT_EVT_CONNACK:
        if (evt->result != 0)
        {
            LOG_ERR("MQTT connect failed: %d", evt->result);
            break;
        }

        LOG_INF("MQTT client connected");
        subscribe();
        break;

    case MQTT_EVT_DISCONNECT:
        LOG_INF("MQTT client disconnected: %d", evt->result);
        break;

    case MQTT_EVT_PUBLISH:
    {
        LOG_INF("Received MQTT PUBLISH event");
        const struct mqtt_publish_param *p = &evt->param.publish;

        LOG_INF("MQTT PUBLISH result=%d len=%d",
                evt->result, p->message.payload.len);
        err = publish_get_payload(c, p->message.payload.len);

        if (p->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE)
        {
            const struct mqtt_puback_param ack = {
                .message_id = p->message_id};

            /* Send acknowledgment. */
            mqtt_publish_qos1_ack(&client, &ack);
        }

        if (err >= 0)
        {
            size_t len = p->message.payload.len;
            data_print("Received: ", payload_buf, len);

            /* Null-terminate the payload msg */
            char buf[len + 1];
            memcpy(buf, payload_buf, len);
            buf[len] = 0;

            // Split into ID and command
            char *token = strtok(buf, "?");

            if (token == NULL)
            {
                LOG_INF("Error splitting payload");
                break;
            }

            char *id = token;
            token = strtok(NULL, "?");
            char *command = token;
            LOG_INF("Target ID: %s", id);
            LOG_INF("Command: %s", command);

            int id_number = strtol(id, NULL, 10);
            if (id_number != CONFIG_LOCKBOX_ID)
            {
                LOG_INF("ID does not match, breaking");
                break;
            }

            int ret = 0;
            /* Handle commands */
            if (strcmp((const char *)command, CMD_OPEN) == 0)
            {
                ret = open_lockbox();
            }
            else if (strcmp((const char *)command, CMD_CLOSE) == 0)
            {
                ret = close_lockbox();
            }
            else if (strcmp((const char *)command, CMD_KEY_STATUS) == 0)
            {
                ret = publish_key_status();
            }
            else if (strcmp((const char *)command, CMD_PING) == 0)
            {
                ret = pong();
            }
            else if (strcmp((const char *)command, CMD_LOCK_STATUS) == 0)
            {
                ret = publish_lock_status();
            }
            else
            {
                LOG_INF("Unknown command: %s", command);
            }
        }
        else if (err == -EMSGSIZE)
        {
            LOG_ERR("Received payload (%d bytes) is larger than the payload buffer "
                    "size (%d bytes).",
                    p->message.payload.len, sizeof(payload_buf));
        }
        else
        {
            LOG_ERR("publish_get_payload failed: %d", err);
            LOG_INF("Disconnecting MQTT client...");

            err = mqtt_disconnect(c);
            if (err)
            {
                LOG_ERR("Could not disconnect: %d", err);
            }
        }
    }
    break;

    case MQTT_EVT_PUBACK:
        if (evt->result != 0)
        {
            LOG_ERR("MQTT PUBACK error: %d", evt->result);
            break;
        }

        LOG_INF("PUBACK packet id: %u", evt->param.puback.message_id);
        break;

    case MQTT_EVT_SUBACK:
        if (evt->result != 0)
        {
            LOG_ERR("MQTT SUBACK error: %d", evt->result);
            break;
        }

        LOG_INF("SUBACK packet id: %u", evt->param.suback.message_id);
        break;

    case MQTT_EVT_PINGRESP:
        if (evt->result != 0)
        {
            LOG_ERR("MQTT PINGRESP error: %d", evt->result);
        }
        break;

    default:
        LOG_INF("Unhandled MQTT event type: %d", evt->type);
        break;
    }
}

int broker_init(void)
{
    int err;
    struct addrinfo *result;
    struct addrinfo *addr;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM};

    err = getaddrinfo(CONFIG_MQTT_BROKER_HOSTNAME, NULL, &hints, &result);
    if (err)
    {
        LOG_ERR("getaddrinfo failed: %d", err);
        return -ECHILD;
    }

    addr = result;

    /* Look for address of the broker. */
    while (addr != NULL)
    {
        /* IPv4 Address. */
        if (addr->ai_addrlen == sizeof(struct sockaddr_in))
        {
            struct sockaddr_in *broker4 =
                ((struct sockaddr_in *)&broker);
            char ipv4_addr[NET_IPV4_ADDR_LEN];

            broker4->sin_addr.s_addr =
                ((struct sockaddr_in *)addr->ai_addr)
                    ->sin_addr.s_addr;
            broker4->sin_family = AF_INET;
            broker4->sin_port = htons(CONFIG_MQTT_BROKER_PORT);

            inet_ntop(AF_INET, &broker4->sin_addr.s_addr,
                      ipv4_addr, sizeof(ipv4_addr));
            LOG_INF("IPv4 Address found %s", ipv4_addr);

            break;
        }
        else
        {
            LOG_ERR("ai_addrlen = %u should be %u or %u",
                    (unsigned int)addr->ai_addrlen,
                    (unsigned int)sizeof(struct sockaddr_in),
                    (unsigned int)sizeof(struct sockaddr_in6));
        }

        addr = addr->ai_next;
    }

    /* Free the address. */
    freeaddrinfo(result);

    return err;
}

#if defined(CONFIG_NRF_MODEM_LIB)
#define IMEI_LEN 15
#define CGSN_RESPONSE_LENGTH (IMEI_LEN + 6 + 1) /* Add 6 for \r\nOK\r\n and 1 for \0 */
#define CLIENT_ID_LEN sizeof("nrf-") + IMEI_LEN
#else
#define RANDOM_LEN 10
#define CLIENT_ID_LEN sizeof(CONFIG_BOARD) + 1 + RANDOM_LEN
#endif /* defined(CONFIG_NRF_MODEM_LIB) */

/* Function to get the client id */
const uint8_t *client_id_get(void)
{
    static uint8_t client_id[MAX(sizeof(CONFIG_MQTT_CLIENT_ID),
                                 CLIENT_ID_LEN)];

    if (strlen(CONFIG_MQTT_CLIENT_ID) > 0)
    {
        snprintf(client_id, sizeof(client_id), "%s",
                 CONFIG_MQTT_CLIENT_ID);
        goto exit;
    }
exit:
    LOG_DBG("client_id = %s", client_id);

    return client_id;
}

int client_init(struct mqtt_client *client)
{
    int err;

    mqtt_client_init(client);

    err = broker_init();
    if (err)
    {
        LOG_ERR("Failed to initialize broker connection");
        return err;
    }

    /* MQTT client configuration */
    client->broker = &broker;
    client->evt_cb = mqtt_evt_handler;
    client->client_id.utf8 = client_id_get();
    client->client_id.size = strlen(client->client_id.utf8);

    password.utf8 = (uint8_t *)CONFIG_MQTT_BROKER_PASSWORD;
    password.size = strlen(CONFIG_MQTT_BROKER_PASSWORD);

    user_name.utf8 = (uint8_t *)CONFIG_MQTT_BROKER_USERNAME;
    user_name.size = strlen(CONFIG_MQTT_BROKER_USERNAME);

    client->password = &password;
    client->user_name = &user_name;
    client->protocol_version = MQTT_VERSION_3_1_1;

    /* MQTT buffers configuration */
    client->rx_buf = rx_buffer;
    client->rx_buf_size = sizeof(rx_buffer);
    client->tx_buf = tx_buffer;
    client->tx_buf_size = sizeof(tx_buffer);

    /* MQTT transport configuration */
#if defined(CONFIG_MQTT_LIB_TLS)
    struct mqtt_sec_config *tls_cfg = &(client->transport).tls.config;

    LOG_INF("TLS enabled");
    client->transport.type = MQTT_TRANSPORT_SECURE;

    tls_cfg->peer_verify = CONFIG_MQTT_TLS_PEER_VERIFY;
    tls_cfg->cipher_count = 0;
    tls_cfg->cipher_list = NULL;
    tls_cfg->sec_tag_count = ARRAY_SIZE(sec_tag_list);
    tls_cfg->sec_tag_list = sec_tag_list;
    tls_cfg->hostname = CONFIG_MQTT_BROKER_HOSTNAME;

#if defined(CONFIG_NRF_MODEM_LIB)
    tls_cfg->session_cache = IS_ENABLED(CONFIG_MQTT_TLS_SESSION_CACHING) ? TLS_SESSION_CACHE_ENABLED : TLS_SESSION_CACHE_DISABLED;
#else
    /* TLS session caching is not supported by the Zephyr network stack */
    tls_cfg->session_cache = TLS_SESSION_CACHE_DISABLED;

#endif

#else
    client->transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif

    return err;
}

int fds_init(struct mqtt_client *c)
{
    if (c->transport.type == MQTT_TRANSPORT_NON_SECURE)
    {
        fds.fd = c->transport.tcp.sock;
    }
    else
    {
#if defined(CONFIG_MQTT_LIB_TLS)
        fds.fd = c->transport.tls.sock;
#else
        return -ENOTSUP;
#endif
    }

    fds.events = POLLIN;

    return 0;
}

int modem_configure(void)
{
#if defined(CONFIG_LTE_LINK_CONTROL)
    /* Turn off LTE power saving features for a more responsive demo. Also,
     * request power saving features before network registration. Some
     * networks rejects timer updates after the device has registered to the
     * LTE network.
     */
    LOG_INF("Disabling PSM and eDRX");
    lte_lc_psm_req(false);
    lte_lc_edrx_req(false);

    if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT))
    {
        /* Do nothing, modem is already turned on
         * and connected.
         */
    }
    else
    {
        int err;

        LOG_INF("LTE Link Connecting...");
        err = lte_lc_init_and_connect();
        if (err)
        {
            LOG_INF("Failed to establish LTE connection: %d", err);
            return err;
        }
        LOG_INF("LTE Link Connected!");
    }
#endif /* defined(CONFIG_LTE_LINK_CONTROL) */

    return 0;
}