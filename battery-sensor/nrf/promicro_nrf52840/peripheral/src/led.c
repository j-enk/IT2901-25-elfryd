#include "led.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static struct k_work_delayable blink_work;
static bool led_is_on;

#define BLINK_ONOFF K_MSEC(500)

static void blink_timeout(struct k_work *work) {
    led_is_on = !led_is_on;
    gpio_pin_set(led.port, led.pin, (int)led_is_on);
    k_work_schedule(&blink_work, BLINK_ONOFF);
}

int led_setup(void) {
    if (!gpio_is_ready_dt(&led)) {
        printk("LED device not ready\n");
        return -EIO;
    }

    int err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err) {
        printk("Failed to configure LED GPIO pin\n");
        return err;
    }

    k_work_init_delayable(&blink_work, blink_timeout);
    return 0;
}

void led_start_blinking(void) {
    led_is_on = false;
    gpio_pin_set(led.port, led.pin, (int)led_is_on);
    k_work_schedule(&blink_work, BLINK_ONOFF);
}

void led_stop_blinking(void) {
    struct k_work_sync work_sync;
    k_work_cancel_delayable_sync(&blink_work, &work_sync);
    led_is_on = true;
    gpio_pin_set(led.port, led.pin, (int)led_is_on);
}