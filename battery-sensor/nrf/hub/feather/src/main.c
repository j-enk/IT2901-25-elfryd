/**
 * @file main.c
 * @brief Main application for the Elfryd hub
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(elfryd_hub, LOG_LEVEL_INF);

#include "sensors/sensors.h"
#include "config/config_module.h"
#include "mqtt/mqtt_client.h"
#include "utils/utils.h"

/* Configuration for sensor data generation */
#define MAIN_LOOP_INTERVAL_MS 100  /* Main loop interval in milliseconds */

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

/* MQTT processing thread function */
static void mqtt_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    int retry_count = 0;
    int max_retries = 5;

    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("MQTT thread started");

    /* Initialize MQTT client */
    err = elfryd_mqtt_client_init();
    if (err) {
        LOG_ERR("Failed to initialize MQTT client: %d", err);
        return;
    }

    /* Connect to the MQTT broker with retries */
    do {
        err = mqtt_client_connect();
        if (err) {
            LOG_WRN("Failed to connect to MQTT broker, retrying (%d/%d): %d", 
                   retry_count + 1, max_retries, err);
            k_sleep(K_SECONDS(5));
        }
    } while (err && ++retry_count < max_retries);

    if (err) {
        LOG_ERR("Failed to connect to MQTT broker after %d attempts", max_retries);
        return;
    }

    /* Main MQTT processing loop */
    while (1) {
        /* Process MQTT events */
        err = mqtt_client_process(1000);
        if (err && err != -EAGAIN) {
            LOG_ERR("Error in MQTT processing: %d", err);
            
            /* Try to reconnect */
            if (!mqtt_client_is_connected()) {
                LOG_INF("Attempting to reconnect to MQTT broker");
                mqtt_client_connect();
            }
        }

        k_sleep(K_MSEC(100));
    }
}

/* Sensor data collection thread function */
static void sensor_thread_fn(void *arg1, void *arg2, void *arg3)
{
    int err;
    static int battery_id = 1;  /* Rotate through battery IDs 1-4 */
    
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    LOG_INF("Sensor thread started");

    /* Initialize sensors */
    sensors_init();

    /* Initialize timestamps */
    int64_t current_time = utils_get_timestamp();
    last_battery_publish_time = current_time;
    last_temp_publish_time = current_time;
    last_gyro_publish_time = current_time;

    /* Main sensor processing loop */
    while (1) {
        current_time = utils_get_timestamp();
        
        /* Generate new sensor data every second regardless of publishing interval */
        err = sensors_generate_battery_reading(battery_id);
        if (err) {
            LOG_ERR("Failed to generate battery reading: %d", err);
        }
        
        /* Rotate through battery IDs */
        battery_id = (battery_id % 4) + 1;
        
        err = sensors_generate_temp_reading();
        if (err) {
            LOG_ERR("Failed to generate temperature reading: %d", err);
        }
        
        err = sensors_generate_gyro_reading();
        if (err) {
            LOG_ERR("Failed to generate gyroscope reading: %d", err);
        }
        
        /* Check if it's time to publish battery data */
        int battery_interval = config_get_battery_interval();
        if (battery_interval > 0 && 
            (current_time - last_battery_publish_time) >= battery_interval) {
            
            if (mqtt_client_is_connected()) {
                /* Get accumulated battery readings and publish them */
                battery_reading_t readings[MAX_BATTERY_SAMPLES];
                int count = sensors_get_battery_readings(readings, MAX_BATTERY_SAMPLES);
                
                if (count > 0) {
                    err = mqtt_client_publish_battery(readings, count);
                    if (err) {
                        LOG_ERR("Failed to publish battery data: %d", err);
                    } else {
                        LOG_INF("Published %d battery readings", count);
                        sensors_clear_battery_readings();
                    }
                }
            }
            
            last_battery_publish_time = current_time;
        }
        
        /* Check if it's time to publish temperature data */
        int temp_interval = config_get_temp_interval();
        if (temp_interval > 0 && 
            (current_time - last_temp_publish_time) >= temp_interval) {
            
            if (mqtt_client_is_connected()) {
                /* Get accumulated temperature readings and publish them */
                temp_reading_t readings[MAX_TEMP_SAMPLES];
                int count = sensors_get_temp_readings(readings, MAX_TEMP_SAMPLES);
                
                if (count > 0) {
                    err = mqtt_client_publish_temp(readings, count);
                    if (err) {
                        LOG_ERR("Failed to publish temperature data: %d", err);
                    } else {
                        LOG_INF("Published %d temperature readings", count);
                        sensors_clear_temp_readings();
                    }
                }
            }
            
            last_temp_publish_time = current_time;
        }
        
        /* Check if it's time to publish gyroscope data */
        int gyro_interval = config_get_gyro_interval();
        if (gyro_interval > 0 && 
            (current_time - last_gyro_publish_time) >= gyro_interval) {
            
            if (mqtt_client_is_connected()) {
                /* Get accumulated gyroscope readings and publish them */
                gyro_reading_t readings[MAX_GYRO_SAMPLES];
                int count = sensors_get_gyro_readings(readings, MAX_GYRO_SAMPLES);
                
                if (count > 0) {
                    err = mqtt_client_publish_gyro(readings, count);
                    if (err) {
                        LOG_ERR("Failed to publish gyroscope data: %d", err);
                    } else {
                        LOG_INF("Published %d gyroscope readings", count);
                        sensors_clear_gyro_readings();
                    }
                }
            }
            
            last_gyro_publish_time = current_time;
        }
        
        k_sleep(K_SECONDS(1));  /* Generate new data every second */
    }
}

int main(void)
{
    int err;

    LOG_INF("Elfryd Hub starting...");

    /* Initialize configuration */
    err = config_init();
    if (err) {
        LOG_ERR("Failed to initialize configuration: %d", err);
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

    LOG_INF("Elfryd Hub initialized and running");

    /* Main thread can sleep as the work is done in other threads */
    while (1) {
        k_sleep(K_FOREVER);
    }
    
    return 0;  /* Never reached, but needed for correct return type */
}