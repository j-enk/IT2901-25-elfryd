#include <zephyr/kernel.h>
#include "sensor.h"
#include "ble.h"
#include "led.h"

extern int is_connected;

int main(void){
    int err;

    // Initialize Bluetooth
    err = ble_init();
    if (err){
        printk("Bluetooth initialization failed (err %d)\n", err);
        return err;
    }

    // Initialize LED
    init_led();
    err = led_init();
    if (err) {
        printk("LED initialization failed (err %d)\n", err);
        return err;
    }

    // Start LED blinking
    led_set_connection_state(false);

    // Initialize sensors
    err = sensor_init();
    if (err) {
        printk("Sensor initialization failed (err %d)\n", err);
        return err;
    }

    err = ble_advertise();
    if (err) {
        printk("Advertising failed (err %d)\n", err);
        return err;
    }
    k_sleep(K_FOREVER);
    return 0;
}