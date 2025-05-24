#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "led.h"

ATOMIC_DEFINTE(state, 2U);

#define STATE_CONNECTED    1U
#define STATE_DISCONNECTED 2U
#define BLINK_TIMER K_MSEC(500)
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct k_work_delayable blink_work;
static bool led_is_on;

static void blink_handler(struct k_work *work){
    led_is_on = !led_is_on;
    gpio_pin_set(led.port, led.pin, (int)led_is_on);
    k_work_schedule(&blink_work, BLINK_TIMER);
}

void init_led(void){
    int err;

    if(!gpio_is_ready_dt(&led)){
        printk("LED device not ready\n");
        return -EIO;
    }
    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if(err){
        printk("Could not configure GPIO pin %d (%d)\n", led.pin, err);
        return -EIO;
    }
    k_work_init_delayable(&blink_work, blink_handler);
}

void led_set_connection_state(bool state){
    if(state == true){
        (void)atomic_set_bit(state, STATE_CONNECTED);
        k_work_cancel_delayable(&blink_work);
    }
    else if (state == false) {
        (void)atomic_set_bit(state, STATE_DISCONNECTED);
        k_work_schedule(&blink_work, BLINK_TIMER);
    }
    else
    {
        /* Error */
    }   
}