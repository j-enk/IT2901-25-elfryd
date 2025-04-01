#include "gpio_utils.h"
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "lock_detect_event.h"

LOG_MODULE_DECLARE(mqtt_simple, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

const struct gpio_dt_spec led_3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

bool lock_pressed_state = false;

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & DK_BTN1_MSK)
    {
        if (button_state & DK_BTN1_MSK)
        {
            LOG_INF("Button 1 pressed");
            lock_pressed_state = true;
            create_submit_lock_detect_event(true);
        }
        else
        {
            LOG_INF("Button 1 released");
            lock_pressed_state = false;
            create_submit_lock_detect_event(false);
        }
    }
    return;
}

int gpio_setup(void)
{
    LOG_INF("Setting up GPIO...");

    int ret;
    if (!device_is_ready(led_3.port))
    {
        LOG_ERR("LED device is not ready");
        return ret;
    }

    ret = gpio_pin_configure_dt(&led_3, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure GPIO pin");
        return ret;
    }

#if defined(CONFIG_DK_LIBRARY)
    ret = dk_buttons_init(button_handler);
    if (ret < 0)
    {
        LOG_ERR("Failed to initialize buttons");
        return ret;
    }
#endif /*CONFIG_DK_LIBRARY*/

    return ret;
}