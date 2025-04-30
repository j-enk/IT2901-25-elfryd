/* main.c - Application main entry point */

/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 
 #include <zephyr/bluetooth/bluetooth.h>
 #include <zephyr/bluetooth/hci.h>
 #include <zephyr/bluetooth/conn.h>
 #include <zephyr/bluetooth/uuid.h>
 #include <zephyr/bluetooth/gatt.h>
 #include <zephyr/drivers/gpio.h>
 #include <zephyr/drivers/adc.h>
 #include <zephyr/drivers/sensor.h>
 #include <math.h>
 #include <zephyr/kernel.h>
 
 // i made this up
 #define BT_UUID_MPU BT_UUID_DECLARE_16(0x2f01)
 
 #define BAUT_VOLTAGE
 #define BAUT_TEMPERATURE
 #define BAUT_MPU
 
 #ifdef BAUT_TEMPERATURE
 const struct device *bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);
 static struct sensor_value temperature;
 #endif
 
 #ifdef BAUT_MPU
 const struct device *const mpu6050 = DEVICE_DT_GET_ONE(invensense_mpu6050);
 #endif
 
 static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
 
 // First filtering on central
 static const uint8_t custom_uuid[] = {
     0x00, 0x00, 0x2A, 0x6E,
     0x00, 0x00,
     0x10, 0x00,
     0x80, 0x00,
     0x00, 0x80,
     0x5F, 0x9B, 0x34, 0xFB,
 };
 
 static const struct bt_data ad[] = {
     BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
     BT_DATA(BT_DATA_UUID128_ALL, custom_uuid, sizeof(custom_uuid)), 
 };
 
 static const struct bt_data sd[] = {
     BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
 };
 
 /* Use atomic variable, 2 bits for connection and disconnection state */
 static ATOMIC_DEFINE(state, 2U);
 
 #define STATE_CONNECTED    1U
 #define STATE_DISCONNECTED 2U
 
 static void connected(struct bt_conn *conn, uint8_t err)
 {
     if (err) {
         printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
     } else {
         printk("Connected!!!\n");
 
         (void)atomic_set_bit(state, STATE_CONNECTED);
     }
 }
 
 static void disconnected(struct bt_conn *conn, uint8_t reason)
 {
     printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
 
     (void)atomic_set_bit(state, STATE_DISCONNECTED);
 }
 
 BT_CONN_CB_DEFINE(conn_callbacks) = {
     .connected = connected,
     .disconnected = disconnected,
 };
 
 /* The devicetree node identifier for the "led0" alias. */
 #define LED0_NODE DT_ALIAS(led0)
 
 #define HAS_LED 1
 static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
 #define BLINK_ONOFF K_MSEC(500)
 
 static struct k_work_delayable blink_work;
 static bool led_is_on;
 
 static void blink_timeout(struct k_work *work)
 {
     led_is_on = !led_is_on;
     gpio_pin_set(led.port, led.pin, (int)led_is_on);
 
     k_work_schedule(&blink_work, BLINK_ONOFF);
 }
 
 static int blink_setup(void)
 {
     int err;
 
     printk("Checking LED device...");
     if (!gpio_is_ready_dt(&led)) {
         printk("failed.\n");
         return -EIO;
     }
     printk("done.\n");
 
     printk("Configuring GPIO pin...");
     err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
     if (err) {
         printk("failed.\n");
         return -EIO;
     }
     printk("done.\n");
 
     k_work_init_delayable(&blink_work, blink_timeout);
 
     return 0;
 }
 
 static void blink_start(void)
 {
     // printk("Start blinking LED...\n");
     led_is_on = false;
     gpio_pin_set(led.port, led.pin, (int)led_is_on);
     k_work_schedule(&blink_work, BLINK_ONOFF);
 }
 
 static void blink_stop(void)
 {
     struct k_work_sync work_sync;
 
     // printk("Stop blinking LED.\n");
     k_work_cancel_delayable_sync(&blink_work, &work_sync);
 
     /* Keep LED on */
     led_is_on = true;
     gpio_pin_set(led.port, led.pin, (int)led_is_on);
 }
 
 int baut_adc_init() {
     if (!adc_is_ready_dt(&adc_channel)) {
         printk("ADC controller devivce %s not ready\n", adc_channel.dev->name);
         return -1;
     }
 
     int err = adc_channel_setup_dt(&adc_channel);
     if (err) {
         printk("Could not setup channel #%d (%d)\n", 0, err);
         return err;
     }
 
     return 0;
 }
 
 int baut_adc_read(int32_t * mv) {
     int16_t buf;
     struct adc_sequence sequence = {
         .buffer = &buf,
         /* buffer size in bytes, not number of samples */
         .buffer_size = sizeof(buf),
         //Optional
         //.calibrate = true,
     };
     int err = adc_sequence_init_dt(&adc_channel, &sequence);
     if (err) {
         printk("Could not initalize sequnce\n");
         return -1;
     }
 
     err = adc_read(adc_channel.dev, &sequence);
     if (err) {
         printk("Could not read (%d)\n", err);
         return -1;
     }
 
     // printk("dbg, %x\n", buf);
 
     int32_t val_mv = buf;
     err = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
     if (err) {
         printk("adc_raw_to_millivolts_dt failed: %d\n", err);
         return -1;
     }
     // printk(" = %d mV\n", val_mv);
 
     // calibrated for first perfboard test
     val_mv *= 6.23;
 
     *mv = val_mv;
     return 0;
 }
 
 static ssize_t vol_read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                  uint16_t len, uint16_t offset)
 {
 
     int32_t mv = -1;
     if(baut_adc_read(&mv) == 0) {
         printk("mV = %d\n", mv);
     } else {
         printk("mV failed\n");
         mv = -1;
         // return;
     }
 
     return bt_gatt_attr_read(conn, attr, buf, len, offset, &mv, sizeof(mv));
 }
 
 int baut_bme_init() {
     if(bme280_dev == NULL) {
         printk("BME is missing!\n");
         return -1;
     }
 
     if (!device_is_ready(bme280_dev)) {
         printk("BME is not ready\n");
         return -1;
     }
 
     return 0;
 }
 
 int baut_bme_read(int32_t * celcius) {
     // we have to retry on EAGAIN (we just do it no matter the error :) )
     int suceeded = false;
 
     for(int i = 0; i < 10; i ++) {
         // printk("i %d\n", i);
         int err;
 
         err = sensor_sample_fetch(bme280_dev);
         if(err) {
             if(err != -EAGAIN) {
                 printk("BME sample fail: %d\n", err);
             }
             continue;
         }
 
         err = sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
         if (err) {
             printk("BME channel fail: %d\n", err);
             continue;
         }
 
         suceeded = true;
         break;
     }
 
     if(suceeded) {
         // printk("Temperature: %d Celsius\n", temperature.val1);
         *celcius = temperature.val1;	
     }
 
     return 0;
 }
 
 
 static ssize_t temp_read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                   uint16_t len, uint16_t offset)
 {
 
     int32_t celcius = -1;
     if(baut_bme_read(&celcius) == 0) {
         printk("celcius = %d\n", celcius);
     } else {
         printk("celcius failed\n");
         celcius = -1;
         // return;
     }
 
     return bt_gatt_attr_read(conn, attr, buf, len, offset, &celcius, sizeof(celcius));
 }
 
 int baut_mpu_init() {
     if(mpu6050 == NULL) {
         printk("MPU is missing!\n");
         return -1;
     }
 
     if (!device_is_ready(mpu6050)) {
         printk("MPU is not ready\n");
         return -1;
     }
 
     return 0;
 }
 
 // accel[3] + gyro[3]
 int baut_mpu_read(double values[6]) {
     struct sensor_value accel[3];
     struct sensor_value gyro[3];
 
     int err = sensor_sample_fetch(mpu6050);
     if(err) {
         printf("MPU6050: sample fetch failed: %d\n", err);
         return err;
     }
 
     err = sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ, accel);
     if(err) {
         printf("MPU6050: accel failed: %d\n", err);
         return err;
     }
 
     err = sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ, gyro);
     if(err) {
         printf("MPU6050: gyro failed: %d\n", err);
         return err;
     }
 
     values[0] = sensor_value_to_double(&accel[0]);
     values[1] = sensor_value_to_double(&accel[1]);
     values[2] = sensor_value_to_double(&accel[2]);
     values[3] = sensor_value_to_double(&gyro[0]);
     values[4] = sensor_value_to_double(&gyro[1]);
     values[5] = sensor_value_to_double(&gyro[2]);
 
     // printf(//"[%s]\n"
     // 		"  accel %f %f %f m/s/s\n"
     // 		"  gyro  %f %f %f rad/s\n",
     // 		// now_str(),
     // 		values[0],
     // 		values[1],
     // 		values[2],
     // 		values[3],
     // 		values[4],
     // 		values[5]);
 
     return 0;
 }
 
 static ssize_t mpu_read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                   uint16_t len, uint16_t offset)
 {
 
     double values[6];
     if(baut_mpu_read(values) == 0) {
         printk("MPU: read successfully\n");
         // printk("celcius = %d\n", celcius);
     } else {
         printk("MPU: read failed\n");
         // celcius = -1;
         // return;
     }
 
     return bt_gatt_attr_read(conn, attr, buf, len, offset, &values, sizeof(values));
 }
 
 static ssize_t id_read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                      uint16_t len, uint16_t offset)
 {
     uint8_t sensor_id = 1;
 
     return bt_gatt_attr_read(conn, attr, buf, len, offset, &sensor_id, sizeof(sensor_id));
 }
 
 #ifdef BAUT_VOLTAGE
 
 #define SENSOR_ID_UUID_VAL 0x2c05
 /**
  *  @brief GATT Characteristic Voltage
  */
 #define SENSOR_ID_UUID \
     BT_UUID_DECLARE_16(SENSOR_ID_UUID_VAL)
 
 BT_GATT_SERVICE_DEFINE(sensor_id_svc,
     BT_GATT_PRIMARY_SERVICE(SENSOR_ID_UUID),
 
     // Sensor ID characteristic
     BT_GATT_CHARACTERISTIC(SENSOR_ID_UUID, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                    id_read_function, NULL, NULL),
 );
 
 BT_GATT_SERVICE_DEFINE(vol_svc,
     BT_GATT_PRIMARY_SERVICE(BT_UUID_GATT_V),
 
     // Voltage reading characteristic
     BT_GATT_CHARACTERISTIC(BT_UUID_GATT_V, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
         vol_read_function, NULL, NULL),
 );
 #endif
 
 #ifdef BAUT_TEMPERATURE
 BT_GATT_SERVICE_DEFINE(tmp_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_TEMPERATURE),
                BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, temp_read_function, NULL, NULL), );
 #endif
 
 #ifdef BAUT_MPU
 BT_GATT_SERVICE_DEFINE(mpu_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_MPU),
                BT_GATT_CHARACTERISTIC(BT_UUID_MPU, BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, mpu_read_function, NULL, NULL), );
 #endif
 
 
 int main(void)
 {
     int err;
 
     err = bt_enable(NULL);
     if (err) {
         printk("Bluetooth init failed (err %d)\n", err);
         return 0;
     }
 
 #ifdef BAUT_VOLTAGE
     // yolo, no error check
     baut_adc_init();
 #endif
 
 #ifdef BAUT_TEMPERATURE
     baut_bme_init();
 #endif
 
 #ifdef BAUT_MPU
     baut_mpu_init();
 #endif
 
     printk("Bluetooth initialized\n");
 
     printk("Starting Legacy Advertising (connectable and scannable)\n");
     err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
     if (err) {
         printk("Advertising failed to start (err %d)\n", err);
         return 0;
     }
 
     printk("Advertising successfully started\n");
 
     err = blink_setup();
     if (err) {
         return 0;
     }
 
     blink_start();
 
     while (1) {
         k_sleep(K_SECONDS(1));
 
         if (atomic_test_and_clear_bit(state, STATE_CONNECTED)) {
             /* Connected callback executed */
             blink_stop();
         } else if (atomic_test_and_clear_bit(state, STATE_DISCONNECTED)) {
             printk("Starting Legacy Advertising (connectable and scannable)\n");
             err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd,
                           ARRAY_SIZE(sd));
             if (err) {
                 printk("Advertising failed to start (err %d)\n", err);
                 return 0;
             }
 
             blink_start();
         }
     }
 
     return 0;
 }