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

#include "sensors/sensors.h"
#include "config/config_module.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_publishers.h"
#include "utils/utils.h"

/* Configuration for sensor data generation */
#define MAIN_LOOP_INTERVAL_MS 1000 /* Main loop interval in milliseconds */

/* Thread stacks and definitions */
#define STACK_SIZE 2048
#define MQTT_THREAD_PRIORITY 5
#define SENSOR_THREAD_PRIORITY 6

static K_THREAD_STACK_DEFINE(mqtt_thread_stack, STACK_SIZE);
static struct k_thread mqtt_thread_data;

static K_THREAD_STACK_DEFINE(sensor_thread_stack, STACK_SIZE);
static struct k_thread sensor_thread_data;

/* Timers for tracking when to publish data */
static int64_t last_battery_publish_time;
static int64_t last_temp_publish_time;
static int64_t last_gyro_publish_time;

/* Variables for storing the latest sensor readings */
static battery_reading_t latest_battery_reading;
static temp_reading_t latest_temp_reading;
static gyro_reading_t latest_gyro_reading;
static int battery_count_cached = 0;
static int temp_count_cached = 0;
static int gyro_count_cached = 0;

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

/* Function to publish sensor data immediately */
static void process_immediate_publish_requests(void)
{
    int err;
    bool has_requests = false;
    
    /* Check for publish requests */
    k_mutex_lock(&publish_flags_mutex, K_FOREVER);
    has_requests = battery_publish_request || temp_publish_request || gyro_publish_request;
    k_mutex_unlock(&publish_flags_mutex);
    
    if (!has_requests) {
        return;
    }
    
    /* Only process requests if we're connected to MQTT */
    if (!mqtt_client_is_connected()) {
        LOG_WRN(LOG_PREFIX_MAIN "MQTT not connected, immediate publishing requests will be ignored");
        
        /* Clear all requests since we can't process them now */
        k_mutex_lock(&publish_flags_mutex, K_FOREVER);
        battery_publish_request = false;
        temp_publish_request = false;
        gyro_publish_request = false;
        k_mutex_unlock(&publish_flags_mutex);
        
        return;
    }
    
    LOG_INF(LOG_PREFIX_MAIN "Processing immediate publish requests");
    
    /* Check and publish battery data if requested */
    k_mutex_lock(&publish_flags_mutex, K_FOREVER);
    bool publish_battery = battery_publish_request;
    battery_publish_request = false;
    k_mutex_unlock(&publish_flags_mutex);
    
    if (publish_battery) {
        LOG_INF(LOG_PREFIX_MAIN "Processing immediate battery publish request");
        battery_reading_t readings[MAX_BATTERY_SAMPLES];
        int count = sensors_get_battery_readings(readings, MAX_BATTERY_SAMPLES);
        
        if (count > 0) {
            err = mqtt_client_publish_battery(readings, count);
            if (err) {
                LOG_ERR(LOG_PREFIX_MAIN "Failed to publish battery data: %d", err);
            } else {
                LOG_INF(LOG_PREFIX_MAIN "Immediately published %d battery readings", count);
                sensors_clear_battery_readings();
                battery_count_cached = 0;
            }
        } else {
            LOG_WRN(LOG_PREFIX_MAIN "No battery readings available to publish");
        }
    }
    
    /* Check and publish temperature data if requested */
    k_mutex_lock(&publish_flags_mutex, K_FOREVER);
    bool publish_temp = temp_publish_request;
    temp_publish_request = false;
    k_mutex_unlock(&publish_flags_mutex);
    
    if (publish_temp) {
        LOG_INF(LOG_PREFIX_MAIN "Processing immediate temperature publish request");
        temp_reading_t readings[MAX_TEMP_SAMPLES];
        int count = sensors_get_temp_readings(readings, MAX_TEMP_SAMPLES);
        
        if (count > 0) {
            err = mqtt_client_publish_temp(readings, count);
            if (err) {
                LOG_ERR(LOG_PREFIX_MAIN "Failed to publish temperature data: %d", err);
            } else {
                LOG_INF(LOG_PREFIX_MAIN "Immediately published %d temperature readings", count);
                sensors_clear_temp_readings();
                temp_count_cached = 0;
            }
        } else {
            LOG_WRN(LOG_PREFIX_MAIN "No temperature readings available to publish");
        }
    }
    
    /* Check and publish gyroscope data if requested */
    k_mutex_lock(&publish_flags_mutex, K_FOREVER);
    bool publish_gyro = gyro_publish_request;
    gyro_publish_request = false;
    k_mutex_unlock(&publish_flags_mutex);
    
    if (publish_gyro) {
        LOG_INF(LOG_PREFIX_MAIN "Processing immediate gyroscope publish request");
        gyro_reading_t readings[MAX_GYRO_SAMPLES];
        int count = sensors_get_gyro_readings(readings, MAX_GYRO_SAMPLES);
        
        if (count > 0) {
            err = mqtt_client_publish_gyro(readings, count);
            if (err) {
                LOG_ERR(LOG_PREFIX_MAIN "Failed to publish gyroscope data: %d", err);
            } else {
                LOG_INF(LOG_PREFIX_MAIN "Immediately published %d gyroscope readings", count);
                sensors_clear_gyro_readings();
                gyro_count_cached = 0;
            }
        } else {
            LOG_WRN(LOG_PREFIX_MAIN "No gyroscope readings available to publish");
        }
    }
}

/* Sensor data collection thread function */
static void sensor_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    static int battery_id = 1; /* Rotate through battery IDs 1-4 */

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF(LOG_PREFIX_SENS "Sensor thread started");

    /* Initialize sensors */
    sensors_init();

    /* Initialize timestamps */
    int64_t current_time = utils_get_timestamp();
    last_battery_publish_time = current_time;
    last_temp_publish_time = current_time;
    last_gyro_publish_time = current_time;

    /* Main sensor processing loop */
    while (1)
    {
        current_time = utils_get_timestamp();

        /* Generate new sensor data every second regardless of publishing interval */
        err = sensors_generate_battery_reading(battery_id);
        if (err)
        {
            LOG_ERR(LOG_PREFIX_SENS "Failed to generate battery reading: %d", err);
        }
        else
        {
            err = sensors_get_latest_battery_reading(&latest_battery_reading);
            battery_count_cached = sensors_get_battery_reading_count();
        }

        /* Rotate through battery IDs */
        battery_id = (battery_id % 4) + 1;

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

        /* Print monitoring information for debugging */
        LOG_INF(LOG_PREFIX_SENS "Array sizes - Battery: %d, Temp: %d, Gyro: %d",
                battery_count_cached, temp_count_cached, gyro_count_cached);

        /* Process any immediate publish requests */
        process_immediate_publish_requests();

        /* Check if it's time to publish battery data */
        int battery_interval = config_get_battery_interval();
        if (battery_interval > 0 &&
            (current_time - last_battery_publish_time) >= battery_interval)
        {

            if (mqtt_client_is_connected())
            {
                /* Get accumulated battery readings and publish them */
                battery_reading_t readings[MAX_BATTERY_SAMPLES];
                int count = sensors_get_battery_readings(readings, MAX_BATTERY_SAMPLES);

                if (count > 0)
                {
                    err = mqtt_client_publish_battery(readings, count);
                    if (err)
                    {
                        LOG_ERR(LOG_PREFIX_SENS "Failed to publish battery data: %d", err);
                    }
                    else
                    {
                        LOG_INF(LOG_PREFIX_SENS "Published %d battery readings", count);
                        sensors_clear_battery_readings();
                        battery_count_cached = 0;
                    }
                }
            }

            last_battery_publish_time = current_time;
        }

        /* Check if it's time to publish temperature data */
        int temp_interval = config_get_temp_interval();
        if (temp_interval > 0 &&
            (current_time - last_temp_publish_time) >= temp_interval)
        {

            if (mqtt_client_is_connected())
            {
                /* Get accumulated temperature readings and publish them */
                temp_reading_t readings[MAX_TEMP_SAMPLES];
                int count = sensors_get_temp_readings(readings, MAX_TEMP_SAMPLES);

                if (count > 0)
                {
                    err = mqtt_client_publish_temp(readings, count);
                    if (err)
                    {
                        LOG_ERR(LOG_PREFIX_SENS "Failed to publish temperature data: %d", err);
                    }
                    else
                    {
                        LOG_INF(LOG_PREFIX_SENS "Published %d temperature readings", count);
                        sensors_clear_temp_readings();
                        temp_count_cached = 0;
                    }
                }
            }

            last_temp_publish_time = current_time;
        }

        /* Check if it's time to publish gyroscope data */
        int gyro_interval = config_get_gyro_interval();
        if (gyro_interval > 0 &&
            (current_time - last_gyro_publish_time) >= gyro_interval)
        {

            if (mqtt_client_is_connected())
            {
                /* Get accumulated gyroscope readings and publish them */
                gyro_reading_t readings[MAX_GYRO_SAMPLES];
                int count = sensors_get_gyro_readings(readings, MAX_GYRO_SAMPLES);

                if (count > 0)
                {
                    err = mqtt_client_publish_gyro(readings, count);
                    if (err)
                    {
                        LOG_ERR(LOG_PREFIX_SENS "Failed to publish gyroscope data: %d", err);
                    }
                    else
                    {
                        LOG_INF(LOG_PREFIX_SENS "Published %d gyroscope readings", count);
                        sensors_clear_gyro_readings();
                        gyro_count_cached = 0;
                    }
                }
            }

            last_gyro_publish_time = current_time;
        }

        k_sleep(K_SECONDS(1)); /* Generate new data every second */
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