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
 
 #include <math.h>
 #include <zephyr/kernel.h>
 
 static const uint8_t custom_uuid[] = {
     0xCD, 0xEE, 0x3D, 0x67, 
     0x35, 0xCD, 0x3A, 0x94,
     0x1D, 0x45, 0xBD, 0xB7,
     0x5E, 0x67, 0x70, 0xBF
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
 
 #define HAS_LED     1
 static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
 #define BLINK_ONOFF K_MSEC(500)
 
 static struct k_work_delayable blink_work;
 static bool                  led_is_on;
 
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
 
 static ssize_t read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr,
              void *buf, uint16_t len, uint16_t offset)
 {
 
     int64_t ms = k_uptime_get();
 
     double s = ms / 1000.0;
     double y = cos(s / 10);
 
     int32_t my_data = (int32_t) (y * 1000);
 
     printk("uptime = %lld, my_data = %d\n", ms, my_data);
 
     return bt_gatt_attr_read(conn, attr, buf, len, offset, &my_data,
                  sizeof(my_data));
 }
 
 BT_GATT_SERVICE_DEFINE(vol_svc,
     BT_GATT_PRIMARY_SERVICE(BT_UUID_GATT_V),
     BT_GATT_CHARACTERISTIC(BT_UUID_GATT_V, BT_GATT_CHRC_READ,
         BT_GATT_PERM_READ, read_function, NULL, NULL),
 );
 
 
 int main(void)
 {
     int err;
     
     bt_addr_le_t addr;
     err = bt_enable(NULL);
     if (err) {
         printk("Bluetooth init failed (err %d)\n", err);
         return 0;
     }
 
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
 
     /* Implement notification. */
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