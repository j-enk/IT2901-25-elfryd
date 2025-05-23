/**
 * @file main.c
 * @brief Main application for the Elfryd hub
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
/* Register the module with a dedicated log level and prefix */
LOG_MODULE_REGISTER(elfryd_hub, LOG_LEVEL_INF);
#define LOG_MODULE_NAME elfryd_hub
#define LOG_PREFIX_MAIN "[MAIN] "
#define LOG_PREFIX_MQTT "[MQTT] "
#define LOG_PREFIX_SENS "[SENS] "
#define LOG_PREFIX_TIME "[TIME] "
#define LOG_PREFIX_LTE "[LTE] "

#include <date_time.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include "sensors/sensors.h"
#include "config/config_module.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_publishers.h"
#include "utils/utils.h"

/* Configuration for sensor data generation */
#define MAIN_LOOP_INTERVAL_MS 1000

/* Thread stacks and definitions */
#define STACK_SIZE 4096
#define MQTT_THREAD_PRIORITY 5
#define SENSOR_THREAD_PRIORITY 6
#define TIME_THREAD_PRIORITY 4
#define PUBLISHER_THREAD_PRIORITY 5

/* Types of data to publish */
typedef enum
{
    PUBLISH_TYPE_BATTERY,
    PUBLISH_TYPE_TEMP,
    PUBLISH_TYPE_GYRO,
    PUBLISH_TYPE_CONFIG
} publish_type_t;

/* Message structure for publish queue */
typedef struct
{
    publish_type_t type;
    int64_t timestamp;
    int count;
} publish_msg_t;

static K_THREAD_STACK_DEFINE(mqtt_thread_stack, STACK_SIZE);
static struct k_thread mqtt_thread_data;

static K_THREAD_STACK_DEFINE(sensor_thread_stack, STACK_SIZE);
static struct k_thread sensor_thread_data;

static K_THREAD_STACK_DEFINE(time_thread_stack, STACK_SIZE);
static struct k_thread time_thread_data;

static K_THREAD_STACK_DEFINE(publisher_thread_stack, STACK_SIZE);
static struct k_thread publisher_thread_data;

/* Semaphore for signaling when date/time is synchronized */
K_SEM_DEFINE(date_time_ready, 0, 1);
K_SEM_DEFINE(lte_ready, 0, 1);

/* Message queue for publishing operations */
K_MSGQ_DEFINE(publish_msgq, sizeof(publish_msg_t), 10, 4);

/* Static buffers for readings to avoid stack allocations */
static battery_reading_t static_battery_readings[MAX_BATTERY_SAMPLES];
static temp_reading_t static_temp_readings[MAX_TEMP_SAMPLES];
static gyro_reading_t static_gyro_readings[MAX_GYRO_SAMPLES];

/* Timers for tracking when to publish data */
static int64_t last_battery_publish_time;
static int64_t last_temp_publish_time;
static int64_t last_gyro_publish_time;

/* Variables for storing the latest sensor readings */
#ifdef CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR
static battery_reading_t latest_battery_reading;
static int battery_count_cached = 0;
#endif

#ifdef CONFIG_ELFRYD_ENABLE_TEMP_SENSOR
static temp_reading_t latest_temp_reading;
static int temp_count_cached = 0;
#endif

#ifdef CONFIG_ELFRYD_ENABLE_GYRO_SENSOR
static gyro_reading_t latest_gyro_reading;
static int gyro_count_cached = 0;
#endif

/* External flags for immediate publishing - these definitions need to match
 * the extern declarations in config_module.c
 */
K_MUTEX_DEFINE(publish_flags_mutex);
bool battery_publish_request = false;
bool temp_publish_request = false;
bool gyro_publish_request = false;

/* MQTT processing thread function */
static void mqtt_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    int retry_count = 0;
    int max_retries = 5;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF(LOG_PREFIX_MQTT "MQTT thread started");

    /* Initialize MQTT client */
    err = elfryd_mqtt_client_init();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to initialize MQTT client: %d", err);
        return;
    }

    /* Connect to the MQTT broker with retries */
    do
    {
        err = mqtt_client_connect();
        if (err)
        {
            LOG_WRN(LOG_PREFIX_MQTT "Failed to connect to MQTT broker, retrying (%d/%d): %d",
                    retry_count + 1, max_retries, err);
            k_sleep(K_SECONDS(5));
        }
    } while (err && ++retry_count < max_retries);

    if (err)
    {
        LOG_ERR(LOG_PREFIX_MQTT "Failed to connect to MQTT broker after %d attempts", max_retries);
        return;
    }

    /* Main MQTT processing loop */
    while (1)
    {
        /* Process MQTT events */
        err = mqtt_client_process(1000);
        if (err && err != -EAGAIN)
        {
            LOG_ERR(LOG_PREFIX_MQTT "Error in MQTT processing: %d", err);

            /* Try to reconnect */
            if (!mqtt_client_is_connected())
            {
                LOG_INF(LOG_PREFIX_MQTT "Attempting to reconnect to MQTT broker");
                mqtt_client_connect();
            }
        }

        k_sleep(K_MSEC(100));
    }
}

/* Publisher thread function */
static void publisher_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    publish_msg_t msg;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF(LOG_PREFIX_MAIN "Publisher thread started");

    while (1)
    {
        /* Wait for a publish message */
        if (k_msgq_get(&publish_msgq, &msg, K_SECONDS(1)) == 0)
        {
            /* Handle different types of publish requests */
            switch (msg.type)
            {
            case PUBLISH_TYPE_BATTERY:
            {
                LOG_INF(LOG_PREFIX_MAIN "Processing battery publish request");
                /* Use static buffer instead of stack allocation */
                int count = sensors_get_battery_readings(static_battery_readings, MAX_BATTERY_SAMPLES);

                if (count > 0)
                {
                    if (mqtt_client_is_connected())
                    {
                        err = mqtt_client_publish_battery(static_battery_readings, count);
                        if (err)
                        {
                            LOG_ERR(LOG_PREFIX_MAIN "Failed to publish battery data: %d", err);
                        }
                        else
                        {
                            LOG_INF(LOG_PREFIX_MAIN "Published %d battery readings", count);
                            sensors_clear_battery_readings();
                        }
                    }
                    else
                    {
                        LOG_WRN(LOG_PREFIX_MAIN "MQTT not connected, skipping battery publish");
                    }
                }
                else
                {
                    LOG_WRN(LOG_PREFIX_MAIN "No battery readings to publish");
                }
            }
            break;

            case PUBLISH_TYPE_TEMP:
            {
                LOG_INF(LOG_PREFIX_MAIN "Processing temperature publish request");
                /* Use static buffer instead of stack allocation */
                int count = sensors_get_temp_readings(static_temp_readings, MAX_TEMP_SAMPLES);

                if (count > 0)
                {
                    if (mqtt_client_is_connected())
                    {
                        err = mqtt_client_publish_temp(static_temp_readings, count);
                        if (err)
                        {
                            LOG_ERR(LOG_PREFIX_MAIN "Failed to publish temperature data: %d", err);
                        }
                        else
                        {
                            LOG_INF(LOG_PREFIX_MAIN "Published %d temperature readings", count);
                            sensors_clear_temp_readings();
                        }
                    }
                    else
                    {
                        LOG_WRN(LOG_PREFIX_MAIN "MQTT not connected, skipping temperature publish");
                    }
                }
                else
                {
                    LOG_WRN(LOG_PREFIX_MAIN "No temperature readings to publish");
                }
            }
            break;

            case PUBLISH_TYPE_GYRO:
            {
                LOG_INF(LOG_PREFIX_MAIN "Processing gyroscope publish request");
                /* Use static buffer instead of stack allocation */
                int count = sensors_get_gyro_readings(static_gyro_readings, MAX_GYRO_SAMPLES);

                if (count > 0)
                {
                    if (mqtt_client_is_connected())
                    {
                        err = mqtt_client_publish_gyro(static_gyro_readings, count);
                        if (err)
                        {
                            LOG_ERR(LOG_PREFIX_MAIN "Failed to publish gyroscope data: %d", err);
                        }
                        else
                        {
                            LOG_INF(LOG_PREFIX_MAIN "Published %d gyroscope readings", count);
                            sensors_clear_gyro_readings();
                        }
                    }
                    else
                    {
                        LOG_WRN(LOG_PREFIX_MAIN "MQTT not connected, skipping gyroscope publish");
                    }
                }
                else
                {
                    LOG_WRN(LOG_PREFIX_MAIN "No gyroscope readings to publish");
                }
            }
            break;

            case PUBLISH_TYPE_CONFIG:
                /* Handle config publish requests */
                /* This would handle publishing config confirmations */
                break;

            default:
                LOG_WRN(LOG_PREFIX_MAIN "Unknown publish type: %d", msg.type);
                break;
            }
        }

        /* Sleep a short while if no messages to process */
        k_sleep(K_MSEC(100));
    }
}

/* Date time event handler */
static void date_time_event_handler(const struct date_time_evt *evt)
{
    switch (evt->type)
    {
    case DATE_TIME_OBTAINED_MODEM:
        LOG_INF(LOG_PREFIX_TIME "Date & time obtained from modem");
        k_sem_give(&date_time_ready);
        break;
    case DATE_TIME_OBTAINED_NTP:
        LOG_INF(LOG_PREFIX_TIME "Date & time obtained from NTP");
        k_sem_give(&date_time_ready);
        break;
    case DATE_TIME_OBTAINED_EXT:
        LOG_INF(LOG_PREFIX_TIME "Date & time obtained from external source");
        k_sem_give(&date_time_ready);
        break;
    case DATE_TIME_NOT_OBTAINED:
        LOG_INF(LOG_PREFIX_TIME "Date & time not obtained");
        break;
    default:
        break;
    }
}

/* Time synchronization thread function */
static void time_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    uint64_t ts = 0;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF(LOG_PREFIX_TIME "Time synchronization thread started");

    /* Force time update */
    LOG_INF(LOG_PREFIX_TIME "Requesting time update");
    err = date_time_update_async(date_time_event_handler);
    if (err)
    {
        LOG_ERR(LOG_PREFIX_TIME "Failed to request time update: %d", err);
        return;
    }

    /* Wait for time synchronization */
    LOG_INF(LOG_PREFIX_TIME "Waiting for time synchronization");
    k_sem_take(&date_time_ready, K_FOREVER);

    /* Notify the system that time is now synchronized */
    utils_notify_time_synchronized();

    LOG_INF(LOG_PREFIX_TIME "Time sync complete, entering time tracking loop");

    /* Main time tracking loop */
    while (1)
    {
        /* Get the current time */
        err = date_time_now(&ts);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_TIME "Unable to get date & time: %d", err);
        }
        else
        {
            /* Convert to seconds and log */
            ts = ts / 1000;
            LOG_INF(LOG_PREFIX_TIME "UTC Unix Epoch: %lld", ts);
        }

        /* Time updates are handled automatically by date_time library according to:
         * - TIME_AUTO_UPDATE setting
         * - TIME_UPDATE_INTERVAL_SECONDS for regular updates
         * - TIME_TOO_OLD_SECONDS for staleness checks
         */

        k_sleep(K_SECONDS(5)); /* Update time display every 5 seconds */
    }
}

/* Sensor data collection thread function */
static void sensor_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    publish_msg_t msg;
    int i2c_read_counter = 0;  /* Counter to track when to read I2C data */
    const int I2C_READ_INTERVAL = CONFIG_SENSOR_I2C_READ_INTERVAL;  /* Read I2C data based on config */

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF(LOG_PREFIX_SENS "Sensor thread started");
    LOG_INF(LOG_PREFIX_SENS "I2C read interval set to %d seconds", I2C_READ_INTERVAL);

    /* Initialize sensors */
    sensors_init();

    /* Get the current time */
    int64_t current_time = utils_get_timestamp();

    /* Get intervals */
    int battery_interval = config_get_battery_interval();
    int temp_interval = config_get_temp_interval();
    int gyro_interval = config_get_gyro_interval();

    /* Initialize last publish timestamps to ensure we wait a full interval before first publish */
    last_battery_publish_time = current_time;
    last_temp_publish_time = current_time;
    last_gyro_publish_time = current_time;

    LOG_INF(LOG_PREFIX_SENS "Waiting for first interval before publishing data");
    LOG_INF(LOG_PREFIX_SENS "Intervals (seconds) - Battery: %d, Temp: %d, Gyro: %d",
            battery_interval, temp_interval, gyro_interval);
    
#ifdef CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR
    LOG_INF(LOG_PREFIX_SENS "System configured for %d batteries", NUM_BATTERIES);
#else
    LOG_INF(LOG_PREFIX_SENS "Battery sensor disabled in config");
#endif

#ifndef CONFIG_ELFRYD_ENABLE_TEMP_SENSOR
    LOG_INF(LOG_PREFIX_SENS "Temperature sensor disabled in config");
#endif

#ifndef CONFIG_ELFRYD_ENABLE_GYRO_SENSOR
    LOG_INF(LOG_PREFIX_SENS "Gyroscope sensor disabled in config");
#endif

    /* Main sensor processing loop */
    while (1)
    {
        current_time = utils_get_timestamp();

        /* Only read from I2C sensors every I2C_READ_INTERVAL iterations */
        bool should_read_i2c = (i2c_read_counter == 0);

#ifdef CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR
        /* Generate new battery sensor data */
        if (sensors_using_i2c()) {
            if (should_read_i2c) {
                /* More efficient approach - read all battery data at once when using I2C */
                err = sensors_generate_all_battery_readings();
                if (err < 0 && err != -EAGAIN) {
                    LOG_ERR(LOG_PREFIX_SENS "Failed to generate battery readings: %d", err);
                } else if (err > 0) {
                    LOG_INF(LOG_PREFIX_SENS "Generated %d battery readings in a batch", err);
                }
            }
        } else {
            /* When not using I2C, use the individual approach to ensure consistent behavior */
            for (int battery_id = 1; battery_id <= NUM_BATTERIES; battery_id++)
            {
                err = sensors_generate_battery_reading(battery_id);
                if (err)
                {
                    LOG_ERR(LOG_PREFIX_SENS "Failed to generate battery reading for battery %d: %d",
                            battery_id, err);
                }
            }
        }

        /* Get the latest battery reading and count for monitoring */
        err = sensors_get_latest_battery_reading(&latest_battery_reading);
        battery_count_cached = sensors_get_battery_reading_count();
#endif

#ifdef CONFIG_ELFRYD_ENABLE_TEMP_SENSOR
        /* Generate temperature readings */
        if (!sensors_using_i2c() || should_read_i2c) {
            err = sensors_generate_temp_reading();
            if (err)
            {
                LOG_ERR(LOG_PREFIX_SENS "Failed to generate temperature reading: %d", err);
            }
            else
            {
                err = sensors_get_latest_temp_reading(&latest_temp_reading);
                temp_count_cached = sensors_get_temp_reading_count();
            }
        }
#endif

#ifdef CONFIG_ELFRYD_ENABLE_GYRO_SENSOR
        /* Generate gyro readings */
        if (!sensors_using_i2c() || should_read_i2c) {
            err = sensors_generate_gyro_reading();
            if (err)
            {
                LOG_ERR(LOG_PREFIX_SENS "Failed to generate gyroscope reading: %d", err);
            }
            else
            {
                err = sensors_get_latest_gyro_reading(&latest_gyro_reading);
                gyro_count_cached = sensors_get_gyro_reading_count();
            }
        }
#endif

        /* Print monitoring information for debugging */
#if defined(CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR) && defined(CONFIG_ELFRYD_ENABLE_TEMP_SENSOR) && defined(CONFIG_ELFRYD_ENABLE_GYRO_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Battery: %d/%d, Temp: %d/%d, Gyro: %d/%d",
                battery_count_cached, MAX_BATTERY_SAMPLES,
                temp_count_cached, MAX_TEMP_SAMPLES,
                gyro_count_cached, MAX_GYRO_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR) && defined(CONFIG_ELFRYD_ENABLE_TEMP_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Battery: %d/%d, Temp: %d/%d",
                battery_count_cached, MAX_BATTERY_SAMPLES,
                temp_count_cached, MAX_TEMP_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR) && defined(CONFIG_ELFRYD_ENABLE_GYRO_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Battery: %d/%d, Gyro: %d/%d",
                battery_count_cached, MAX_BATTERY_SAMPLES,
                gyro_count_cached, MAX_GYRO_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_TEMP_SENSOR) && defined(CONFIG_ELFRYD_ENABLE_GYRO_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Temp: %d/%d, Gyro: %d/%d",
                temp_count_cached, MAX_TEMP_SAMPLES,
                gyro_count_cached, MAX_GYRO_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Battery: %d/%d",
                battery_count_cached, MAX_BATTERY_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_TEMP_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Temp: %d/%d",
                temp_count_cached, MAX_TEMP_SAMPLES);
#elif defined(CONFIG_ELFRYD_ENABLE_GYRO_SENSOR)
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Gyro: %d/%d",
                gyro_count_cached, MAX_GYRO_SAMPLES);
#else
        LOG_INF(LOG_PREFIX_SENS "All sensor types disabled");
#endif

        /* Process immediate publish requests by sending messages to publisher thread */
        k_mutex_lock(&publish_flags_mutex, K_FOREVER);

        /* Process immediate publish requests even on first run,
           as these are explicit user requests */
#ifdef CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR
        if (battery_publish_request)
        {
            msg.type = PUBLISH_TYPE_BATTERY;
            msg.timestamp = current_time;
            msg.count = battery_count_cached;

            /* Try to send the message without blocking */
            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue battery publish request: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued immediate battery publish request");
                battery_publish_request = false;
            }
        }
#endif

#ifdef CONFIG_ELFRYD_ENABLE_TEMP_SENSOR
        if (temp_publish_request)
        {
            msg.type = PUBLISH_TYPE_TEMP;
            msg.timestamp = current_time;
            msg.count = temp_count_cached;

            /* Try to send the message without blocking */
            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue temperature publish request: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued immediate temperature publish request");
                temp_publish_request = false;
            }
        }
#endif

#ifdef CONFIG_ELFRYD_ENABLE_GYRO_SENSOR
        if (gyro_publish_request)
        {
            msg.type = PUBLISH_TYPE_GYRO;
            msg.timestamp = current_time;
            msg.count = gyro_count_cached;

            /* Try to send the message without blocking */
            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue gyroscope publish request: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued immediate gyroscope publish request");
                gyro_publish_request = false;
            }
        }
#endif

        k_mutex_unlock(&publish_flags_mutex);

        /* Refresh the interval values (in case config changed) */
        battery_interval = config_get_battery_interval();
        temp_interval = config_get_temp_interval();
        gyro_interval = config_get_gyro_interval();

        /* Check if it's time to publish battery data based on intervals */
#ifdef CONFIG_ELFRYD_ENABLE_BATTERY_SENSOR
        if (battery_interval > 0 &&
            (current_time - last_battery_publish_time) >= battery_interval)
        {
            /* Queue a battery publish request */
            msg.type = PUBLISH_TYPE_BATTERY;
            msg.timestamp = current_time;
            msg.count = battery_count_cached;

            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue battery interval publish: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued battery interval publish request");
                last_battery_publish_time = current_time;
            }
        }
#endif

        /* Check if it's time to publish temperature data */
#ifdef CONFIG_ELFRYD_ENABLE_TEMP_SENSOR
        if (temp_interval > 0 &&
            (current_time - last_temp_publish_time) >= temp_interval)
        {
            /* Queue a temperature publish request */
            msg.type = PUBLISH_TYPE_TEMP;
            msg.timestamp = current_time;
            msg.count = temp_count_cached;

            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue temperature interval publish: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued temperature interval publish request");
                last_temp_publish_time = current_time;
            }
        }
#endif

        /* Check if it's time to publish gyroscope data */
#ifdef CONFIG_ELFRYD_ENABLE_GYRO_SENSOR
        if (gyro_interval > 0 &&
            (current_time - last_gyro_publish_time) >= gyro_interval)
        {
            /* Queue a gyroscope publish request */
            msg.type = PUBLISH_TYPE_GYRO;
            msg.timestamp = current_time;
            msg.count = gyro_count_cached;

            err = k_msgq_put(&publish_msgq, &msg, K_NO_WAIT);
            if (err)
            {
                LOG_WRN(LOG_PREFIX_SENS "Failed to queue gyroscope interval publish: %d", err);
            }
            else
            {
                LOG_INF(LOG_PREFIX_SENS "Queued gyroscope interval publish request");
                last_gyro_publish_time = current_time;
            }
        }
#endif

        /* Update I2C read counter */
        i2c_read_counter = (i2c_read_counter + 1) % I2C_READ_INTERVAL;

        k_sleep(K_SECONDS(1)); /* Main loop still runs every second for timely response to commands */
    }
}

int main(void)
{
    int err;

    LOG_INF(LOG_PREFIX_MAIN "Elfryd Hub starting...");

    /* Initialize configuration */
    err = config_init();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MAIN "Failed to initialize configuration: %d", err);
        return -1;
    }

    /* We'll initialize only the modem library but not the LTE connection
     * This allows the MQTT thread to set up the LTE connection with its specific settings
     */
    err = nrf_modem_lib_init();
    if (err)
    {
        LOG_ERR(LOG_PREFIX_MAIN "Failed to initialize modem library: %d", err);
        return -1;
    }

    LOG_INF(LOG_PREFIX_MAIN "Modem library initialized");

    /* Start time synchronization thread */
    k_thread_create(&time_thread_data, time_thread_stack,
                    K_THREAD_STACK_SIZEOF(time_thread_stack),
                    time_thread_fn, NULL, NULL, NULL,
                    TIME_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&time_thread_data, "time_thread");

    /* Start publisher thread */
    k_thread_create(&publisher_thread_data, publisher_thread_stack,
                    K_THREAD_STACK_SIZEOF(publisher_thread_stack),
                    publisher_thread_fn, NULL, NULL, NULL,
                    PUBLISHER_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&publisher_thread_data, "publisher_thread");

    /* Start MQTT thread */
    k_thread_create(&mqtt_thread_data, mqtt_thread_stack,
                    K_THREAD_STACK_SIZEOF(mqtt_thread_stack),
                    mqtt_thread_fn, NULL, NULL, NULL,
                    MQTT_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&mqtt_thread_data, "mqtt_thread");

    /* Start sensor thread */
    k_thread_create(&sensor_thread_data, sensor_thread_stack,
                    K_THREAD_STACK_SIZEOF(sensor_thread_stack),
                    sensor_thread_fn, NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&sensor_thread_data, "sensor_thread");

    LOG_INF(LOG_PREFIX_MAIN "Elfryd Hub initialized and running");

    /* Main thread can sleep as the work is done in other threads */
    while (1)
    {
        k_sleep(K_FOREVER);
    }

    return 0; /* Never reached, but needed for correct return type */
}