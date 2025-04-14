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

#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <modem/modem_key_mgmt.h>

#include "mqtt/mqtt_client.h"
#include "config/config_module.h"
#include "certificates.h"

/* MQTT broker details (from prj.conf) */
#define SERVER_HOST CONFIG_MQTT_BROKER_HOSTNAME
#define SERVER_PORT CONFIG_MQTT_BROKER_PORT
#define MQTT_CLIENTID CONFIG_MQTT_CLIENT_ID
#define SEC_TAG CONFIG_MQTT_TLS_SEC_TAG

#define APP_MQTT_BUFFER_SIZE 1024
#define APP_CONNECT_TIMEOUT_MS CONFIG_MQTT_CONNECT_TIMEOUT_MS /* Connection timeout from config */

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
    switch (evt->type) {
        case LTE_LC_EVT_NW_REG_STATUS:
            if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
                (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING)) {
                printk("Network registration status: %d\n", evt->nw_reg_status);
                k_sem_give(&lte_connected);
            }
            break;
        case LTE_LC_EVT_PSM_UPDATE:
            printk("PSM parameter update: TAU: %d, Active time: %d\n",
                   evt->psm_cfg.tau, evt->psm_cfg.active_time);
            break;
        case LTE_LC_EVT_EDRX_UPDATE:
            printk("eDRX parameter update: eDRX: %f, PTW: %f\n",
                   evt->edrx_cfg.edrx, evt->edrx_cfg.ptw);
            break;
        case LTE_LC_EVT_RRC_UPDATE:
            printk("RRC mode: %s\n", evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ?
                   "Connected" : "Idle");
            break;
        case LTE_LC_EVT_CELL_UPDATE:
            printk("LTE cell changed: Cell ID: %d, Tracking area: %d\n",
                   evt->cell.id, evt->cell.tac);
            break;
        case LTE_LC_EVT_LTE_MODE_UPDATE:
            printk("Active LTE mode changed: %s\n",
                   evt->lte_mode == LTE_LC_LTE_MODE_NONE ? "None" :
                   evt->lte_mode == LTE_LC_LTE_MODE_LTEM ? "LTE-M" :
                   evt->lte_mode == LTE_LC_LTE_MODE_NBIOT ? "NB-IoT" : "Unknown");
            break;
        default:
            break;
    }
}

static int setup_lte(void)
{
    int err;

    printk("Setting up LTE connection...\n");
    
    /* Initialize the modem first */
    err = nrf_modem_lib_init();
    if (err) {
        printk("Failed to initialize modem library, error: %d\n", err);
        return err;
    }
    
    /* Initialize LTE link controller */
    err = lte_lc_init();
    if (err) {
        printk("Failed to initialize LTE link controller, error: %d\n", err);
        return err;
    }
    
    /* Configure modem for LTE-M/NB-IoT */
    err = lte_lc_system_mode_set(LTE_LC_SYSTEM_MODE_LTEM_GPS, 
                                LTE_LC_SYSTEM_MODE_PREFER_LTEM);
    if (err) {
        printk("Failed to set system mode, error: %d\n", err);
        return err;
    }
    
    /* Set LTE offline for certificate provisioning */
    err = lte_lc_offline();
    if (err) {
        printk("Failed to set modem offline: %d\n", err);
        return err;
    }
    
    /* Set up certificates while offline */
    err = setup_certificates();
    if (err) {
        printk("Failed to set up certificates, error: %d\n", err);
        return err;
    }
    
    /* Register event handler */
    lte_lc_register_handler(lte_handler);
    
    printk("Waiting for LTE connection...\n");
    
    /* Connect to network asynchronously */
    err = lte_lc_connect_async(lte_handler);
    if (err) {
        printk("Failed to connect to LTE network, error: %d\n", err);
        return err;
    }
    
    /* Wait for network registration with a timeout */
    err = k_sem_take(&lte_connected, K_SECONDS(120));
    if (err == -EAGAIN) {
        printk("Timeout waiting for LTE connection\n");
        return -ETIMEDOUT;
    }
    
    printk("LTE connected!\n");
    
    /* Additional check of registration status */
    enum lte_lc_nw_reg_status reg_status;
    err = lte_lc_nw_reg_status_get(&reg_status);
    if (err) {
        printk("Failed to get network registration status: %d\n", err);
    } else {
        printk("Network registration status: %d\n", reg_status);
    }
    
    return 0;
}

static int setup_certificates(void)
{
    int err;
    bool exists;

    printk("Checking for existing certificate...\n");
    
    /* Check if certificates already exist */
    err = modem_key_mgmt_exists(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, &exists);
    if (err) {
        printk("Failed to check for certificates, error: %d\n", err);
        return err;
    }

    if (exists) {
        /* Compare with existing certificate to see if it needs updating */
        err = modem_key_mgmt_cmp(SEC_TAG, 
                               MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                               ca_certificate, 
                               ca_certificate_len);
        if (err == 0) {
            printk("Certificate already provisioned and matches\n");
            return 0;
        }
        
        printk("Certificate exists but doesn't match, updating...\n");
        /* Delete existing certificate */
        err = modem_key_mgmt_delete(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
        if (err) {
            printk("Failed to delete existing certificate: %d\n", err);
            /* Continue anyway, as write might still succeed */
        }
    } else {
        printk("No existing certificate found, provisioning new one\n");
    }

    /* Provision certificates */
    printk("Provisioning certificate...\n");
    
    /* CA certificate */
    err = modem_key_mgmt_write(SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, 
                              ca_certificate, ca_certificate_len);
    if (err) {
        printk("Failed to provision CA certificate, error: %d\n", err);
        return err;
    }

    printk("Certificate provisioned successfully\n");
    return 0;
}

static int get_mqtt_broker_addrinfo(void)
{
    int err;
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    char port[6];

    snprintf(port, sizeof(port), "%d", SERVER_PORT);
    
    printk("Resolving hostname: %s\n", SERVER_HOST);
    
    err = getaddrinfo(SERVER_HOST, port, &hints, &result);
    if (err) {
        printk("getaddrinfo() failed, error: %d\n", err);
        return -ECHILD;
    }

    if (result == NULL) {
        printk("Error: Address not found\n");
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
    printk("Hostname %s resolved to %s\n", SERVER_HOST, resolved_ip);
    
    dns_resolved = true;
    freeaddrinfo(result);

    return 0;
}

static void prepare_fds(struct mqtt_client *client)
{
    if (client->transport.type == MQTT_TRANSPORT_SECURE) {
        fds[0].fd = client->transport.tls.sock;
    } else {
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

    if (nfds > 0) {
        ret = zsock_poll(fds, nfds, timeout);
        if (ret < 0) {
            printk("poll error: %d\n", errno);
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
            printk("MQTT connection failed %d\n", evt->result);
            break;
        }

        mqtt_connected = true;
        printk("MQTT client connected!\n");
        
        /* Subscribe to configuration topic */
        struct mqtt_topic subscribe_topic = {
            .topic = {
                .utf8 = (uint8_t *)MQTT_TOPIC_CONFIG_SEND,
                .size = strlen(MQTT_TOPIC_CONFIG_SEND)
            },
            .qos = MQTT_QOS_1_AT_LEAST_ONCE
        };
        
        const struct mqtt_subscription_list subscription_list = {
            .list = &subscribe_topic,
            .list_count = 1,
            .message_id = 1
        };
        
        err = mqtt_subscribe(client, &subscription_list);
        if (err) {
            printk("Failed to subscribe to %s topic (err %d)\n", 
                  MQTT_TOPIC_CONFIG_SEND, err);
        } else {
            printk("Subscribed to topic: %s\n", MQTT_TOPIC_CONFIG_SEND);
        }
        
        break;

    case MQTT_EVT_DISCONNECT:
        printk("MQTT client disconnected %d\n", evt->result);
        mqtt_connected = false;
        clear_fds();
        break;

    case MQTT_EVT_PUBACK:
        if (evt->result != 0) {
            printk("MQTT PUBACK error %d\n", evt->result);
            break;
        }
        printk("PUBACK packet id: %u\n", evt->param.puback.message_id);
        break;

    case MQTT_EVT_PUBREC:
        if (evt->result != 0) {
            printk("MQTT PUBREC error %d\n", evt->result);
            break;
        }
        printk("PUBREC packet id: %u\n", evt->param.pubrec.message_id);
        
        /* For QoS 2, we need to send a PUBREL */
        const struct mqtt_pubrel_param rel_param = {
            .message_id = evt->param.pubrec.message_id
        };
        
        err = mqtt_publish_qos2_release(client, &rel_param);
        if (err) {
            printk("Failed to send PUBREL: %d\n", err);
        }
        break;

    case MQTT_EVT_PUBCOMP:
        if (evt->result != 0) {
            printk("MQTT PUBCOMP error %d\n", evt->result);
            break;
        }
        printk("PUBCOMP packet id: %u\n", evt->param.pubcomp.message_id);
        break;
        
    case MQTT_EVT_PINGRESP:
        printk("PINGRESP packet\n");
        break;

    case MQTT_EVT_PUBLISH:
        {
            const struct mqtt_publish_param *pub = &evt->param.publish;
            /* Use a larger buffer for received messages */
            static uint8_t temp_buf[512];
            size_t len = pub->message.payload.len;
            
            printk("MQTT message received on topic: %.*s\n", 
                  pub->message.topic.topic.size, pub->message.topic.topic.utf8);
                  
            /* Safely handle message length */
            if (len > sizeof(temp_buf) - 1) {
                printk("Warning: Message truncated (got %u bytes, buffer size %u)\n", 
                       len, sizeof(temp_buf) - 1);
                len = sizeof(temp_buf) - 1;
            }
            
            /* Copy message payload to temporary buffer */
            memcpy(temp_buf, pub->message.payload.data, len);
            temp_buf[len] = '\0';
            
            printk("Received message: %s\n", temp_buf);
            
            /* Check if this is a configuration message by explicitly comparing topic length first */
            if (pub->message.topic.topic.size == strlen(MQTT_TOPIC_CONFIG_SEND) &&
                strncmp((char *)pub->message.topic.topic.utf8, MQTT_TOPIC_CONFIG_SEND, 
                       pub->message.topic.topic.size) == 0) {
                
                printk("Processing configuration command: %s\n", temp_buf);
                
                /* Process configuration command with proper error handling */
                int cmd_result = config_process_command((char *)temp_buf);
                if (cmd_result == 0) {
                    printk("Configuration command processed successfully\n");
                    
                    /* Get confirmation message and publish it */
                    char confirm_buf[256]; /* Increased buffer size */
                    int confirm_len = config_get_confirmation(confirm_buf, sizeof(confirm_buf));
                    
                    if (confirm_len > 0) {
                        printk("Sending confirmation: %s\n", confirm_buf);
                        int pub_result = mqtt_client_publish_config_confirm(confirm_buf);
                        if (pub_result != 0) {
                            printk("Failed to publish confirmation: %d\n", pub_result);
                        }
                    } else {
                        printk("No confirmation message available: %d\n", confirm_len);
                    }
                } else {
                    printk("Failed to process configuration command: %d\n", cmd_result);
                }
            }
            
            /* Send appropriate QoS acknowledgment */
            if (pub->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE) {
                const struct mqtt_puback_param puback = {
                    .message_id = pub->message_id
                };
                
                err = mqtt_publish_qos1_ack(client, &puback);
                if (err) {
                    printk("Failed to send MQTT PUBACK, error: %d\n", err);
                }
            } else if (pub->message.topic.qos == MQTT_QOS_2_EXACTLY_ONCE) {
                const struct mqtt_pubrec_param pubrec = {
                    .message_id = pub->message_id
                };
                
                err = mqtt_publish_qos2_receive(client, &pubrec);
                if (err) {
                    printk("Failed to send MQTT PUBREC, error: %d\n", err);
                }
            }
        }
        break;

    case MQTT_EVT_SUBACK:
        if (evt->result != 0) {
            printk("MQTT SUBACK error %d\n", evt->result);
            break;
        }
        printk("SUBACK packet id: %u\n", evt->param.suback.message_id);
        break;

    default:
        printk("MQTT event: %d\n", evt->type);
        break;
    }
}

int elfryd_mqtt_client_init(void)
{
    int err;
    
    printk("Initializing MQTT client...\n");
    
    /* Set up LTE connection */
    err = setup_lte();
    if (err) {
        printk("Failed to set up LTE connection, error: %d\n", err);
        return err;
    }
    
    /* Initialize broker address */
    err = get_mqtt_broker_addrinfo();
    if (err) {
        printk("Failed to get broker address, error: %d\n", err);
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
    static sec_tag_t sec_tag_list[] = { SEC_TAG };
    tls_config->sec_tag_list = sec_tag_list;
    tls_config->sec_tag_count = 1;
    tls_config->hostname = SERVER_HOST;
    
    /* Setup MQTT buffers */
    client_ctx.rx_buf = rx_buffer;
    client_ctx.rx_buf_size = sizeof(rx_buffer);
    client_ctx.tx_buf = tx_buffer;
    client_ctx.tx_buf_size = sizeof(tx_buffer);
    
    printk("MQTT client initialized\n");
    
    return 0;
}

int mqtt_client_connect(void)
{
    int err;
    int retry_count = 0;
    int max_retries = CONFIG_MQTT_CONNECT_ATTEMPTS;
    
    k_mutex_lock(&mqtt_mutex, K_FOREVER);
    
    if (mqtt_connected) {
        printk("Already connected to MQTT broker\n");
        k_mutex_unlock(&mqtt_mutex);
        return 0;
    }
    
    /* Verify DNS resolution happened */
    if (!dns_resolved) {
        err = get_mqtt_broker_addrinfo();
        if (err) {
            printk("Failed to resolve MQTT broker hostname, error: %d\n", err);
            k_mutex_unlock(&mqtt_mutex);
            return err;
        }
    }
    
    /* Try connecting with retries */
    while (retry_count < max_retries && !mqtt_connected) {
        printk("Attempting to connect to MQTT broker: %s:%d... (attempt %d/%d)\n", 
              SERVER_HOST, SERVER_PORT, retry_count + 1, max_retries);
              
        err = mqtt_connect(&client_ctx);
        if (err) {
            printk("Failed to connect to MQTT broker, error: %d\n", err);
            retry_count++;
            k_sleep(K_MSEC(CONFIG_MQTT_CONNECT_RETRY_DELAY_MS));
            continue;
        }
        
        prepare_fds(&client_ctx);
        
        /* Wait for the connection to complete */
        if (wait(APP_CONNECT_TIMEOUT_MS)) {
            err = mqtt_input(&client_ctx);
            if (err) {
                printk("MQTT input error: %d\n", err);
            }
        }
        
        if (!mqtt_connected) {
            printk("MQTT connection failed or timed out\n");
            mqtt_abort(&client_ctx);
            retry_count++;
            k_sleep(K_MSEC(CONFIG_MQTT_CONNECT_RETRY_DELAY_MS));
        }
    }
    
    if (!mqtt_connected) {
        printk("Failed to connect to MQTT broker after %d attempts\n", max_retries);
        k_mutex_unlock(&mqtt_mutex);
        return -ETIMEDOUT;
    }
    
    /* Send a ping to verify the connection */
    err = mqtt_ping(&client_ctx);
    if (err) {
        printk("Failed to ping MQTT broker: %d\n", err);
        mqtt_abort(&client_ctx);
        mqtt_connected = false;
        k_mutex_unlock(&mqtt_mutex);
        return err;
    }
    
    /* Process the ping response */
    if (wait(APP_CONNECT_TIMEOUT_MS)) {
        err = mqtt_input(&client_ctx);
        if (err) {
            printk("Error processing ping response: %d\n", err);
            mqtt_abort(&client_ctx);
            mqtt_connected = false;
            k_mutex_unlock(&mqtt_mutex);
            return err;
        }
    } else {
        printk("Ping response timeout\n");
    }
    
    k_mutex_unlock(&mqtt_mutex);
    return 0;
}

int mqtt_client_disconnect(void)
{
    int err;
    
    k_mutex_lock(&mqtt_mutex, K_FOREVER);
    
    if (!mqtt_connected) {
        printk("Not connected to MQTT broker\n");
        k_mutex_unlock(&mqtt_mutex);
        return 0;
    }
    
    err = mqtt_disconnect(&client_ctx);
    if (err) {
        printk("Failed to disconnect from MQTT broker, error: %d\n", err);
    } else {
        printk("Disconnected from MQTT broker\n");
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
    
    if (!mqtt_connected) {
        k_mutex_unlock(&mqtt_mutex);
        return -ENOTCONN;
    }
    
    /* Process MQTT events */
    err = wait(timeout);
    if (err < 0) {
        k_mutex_unlock(&mqtt_mutex);
        return err;
    }
    
    err = mqtt_input(&client_ctx);
    if (err) {
        printk("Error in MQTT input: %d\n", err);
        mqtt_abort(&client_ctx);
        mqtt_connected = false;
        clear_fds();
    }
    
    k_mutex_unlock(&mqtt_mutex);
    return err;
}

static int mqtt_client_publish(const char *topic, const char *message, enum mqtt_qos qos)
{
    int err;
    
    k_mutex_lock(&mqtt_mutex, K_FOREVER);
    
    if (!mqtt_connected) {
        printk("Not connected to MQTT broker\n");
        k_mutex_unlock(&mqtt_mutex);
        return -ENOTCONN;
    }
    
    /* Copy message to payload buffer */
    size_t message_len = strlen(message);
    if (message_len > sizeof(payload_buffer)) {
        printk("Message too long for payload buffer\n");
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
        .retain_flag = 0
    };
    
    /* Publish the message */
    err = mqtt_publish(&client_ctx, &param);
    if (err) {
        printk("Failed to publish to %s, error: %d\n", topic, err);
    }
    
    k_mutex_unlock(&mqtt_mutex);
    return err;
}

int mqtt_client_publish_battery(battery_reading_t *readings, int count)
{
    int err;
    char message[256];
    size_t offset = 0;
    bool first = true;
    
    /* Format multiple readings with pipe separator */
    for (int i = 0; i < count && offset < sizeof(message) - 1; i++) {
        size_t len;
        
        if (!first) {
            /* Add separator between readings */
            message[offset++] = '|';
        }
        
        /* Format: "{battery_id}/{voltage}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                     "%d/%d/%lld",
                     readings[i].battery_id,
                     readings[i].voltage,
                     readings[i].timestamp);
                     
        if (len >= sizeof(message) - offset) {
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }
        
        offset += len;
        first = false;
    }
    
    if (offset > 0) {
        /* Publish the message with QoS 1 */
        err = mqtt_client_publish(MQTT_TOPIC_BATTERY, message, MQTT_QOS_1_AT_LEAST_ONCE);
    } else {
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
    for (int i = 0; i < count && offset < sizeof(message) - 1; i++) {
        size_t len;
        
        if (!first) {
            /* Add separator between readings */
            message[offset++] = '|';
        }
        
        /* Format: "{temperature}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                     "%d/%lld",
                     readings[i].temperature,
                     readings[i].timestamp);
                     
        if (len >= sizeof(message) - offset) {
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }
        
        offset += len;
        first = false;
    }
    
    if (offset > 0) {
        /* Publish the message with QoS 1 */
        err = mqtt_client_publish(MQTT_TOPIC_TEMP, message, MQTT_QOS_1_AT_LEAST_ONCE);
    } else {
        err = -EINVAL;
    }
    
    return err;
}

int mqtt_client_publish_gyro(gyro_reading_t *readings, int count)
{
    int err;
    char message[512];  /* Gyro messages are longer */
    size_t offset = 0;
    bool first = true;
    
    /* Format multiple readings with pipe separator */
    for (int i = 0; i < count && offset < sizeof(message) - 1; i++) {
        size_t len;
        
        if (!first) {
            /* Add separator between readings */
            message[offset++] = '|';
        }
        
        /* Format: "{accel_x}/{accel_y}/{accel_z}/{gyro_x}/{gyro_y}/{gyro_z}/{timestamp}" */
        len = snprintf(message + offset, sizeof(message) - offset,
                     "%d/%d/%d/%d/%d/%d/%lld",
                     readings[i].accel_x,
                     readings[i].accel_y,
                     readings[i].accel_z,
                     readings[i].gyro_x,
                     readings[i].gyro_y,
                     readings[i].gyro_z,
                     readings[i].timestamp);
                     
        if (len >= sizeof(message) - offset) {
            /* Message too long, publish what we have so far */
            message[offset] = '\0';
            break;
        }
        
        offset += len;
        first = false;
    }
    
    if (offset > 0) {
        /* Publish the message with QoS 1 */
        err = mqtt_client_publish(MQTT_TOPIC_GYRO, message, MQTT_QOS_1_AT_LEAST_ONCE);
    } else {
        err = -EINVAL;
    }
    
    return err;
}

int mqtt_client_publish_config_confirm(const char *confirmation)
{
    return mqtt_client_publish(MQTT_TOPIC_CONFIG_CONFIRM, confirmation, MQTT_QOS_1_AT_LEAST_ONCE);
}