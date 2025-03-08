#include <zephyr/drivers/gpio.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* GPIO PIN */
#define LED3_NODE DT_ALIAS(led3)
extern const struct gpio_dt_spec led_3;

extern bool lock_pressed_state;

int setup_gpio(void);