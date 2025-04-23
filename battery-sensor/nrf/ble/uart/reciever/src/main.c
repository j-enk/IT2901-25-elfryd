#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)

// LED control characters
#define CMD_LED_ON '1'
#define CMD_LED_OFF '0'

static const struct device *uart_control;
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static uint8_t rx_buf[1];

/**
 * @brief UART event callback
 *
 * Handles UART reception events and toggles the LED accordingly.
 */
void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
        switch (evt->type)
        {
        case UART_RX_RDY:
                if (evt->data.rx.len > 0)
                {
                        char c = evt->data.rx.buf[evt->data.rx.offset];

                        switch (c)
                        {
                        case CMD_LED_ON:
                                gpio_pin_set_dt(&led0, 1);
                                printk("LED ON\n");
                                break;
                        case CMD_LED_OFF:
                                gpio_pin_set_dt(&led0, 0);
                                printk("LED OFF\n");
                                break;
                        default:
                                printk("Unknown command: %c\n", c);
                                break;
                        }
                }
                break;

        case UART_RX_DISABLED:
                // Re-enable UART reception
                if (uart_rx_enable(uart_control, rx_buf, sizeof(rx_buf), 50) < 0)
                {
                        printk("Failed to re-enable UART RX\n");
                }
                break;

        default:
                break;
        }
}

/**
 * @brief Main function
 *
 * Initializes UART and GPIO for LED control based on UART input.
 */
int main(void)
{
        int ret;

        // Use uart1 for control, leaving uart0 for debug/console
        uart_control = DEVICE_DT_GET(DT_NODELABEL(uart1));
        if (!device_is_ready(uart_control))
        {
                printk("UART control device not ready\n");
                return 1;
        }

        if (!device_is_ready(led0.port))
        {
                printk("LED device not ready\n");
                return 1;
        }

        ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
        if (ret < 0)
        {
                printk("Failed to configure LED GPIO: %d\n", ret);
                return 1;
        }

        ret = uart_callback_set(uart_control, uart_cb, NULL);
        if (ret < 0)
        {
                printk("Failed to set UART callback: %d\n", ret);
                return 1;
        }

        ret = uart_rx_enable(uart_control, rx_buf, sizeof(rx_buf), 50);
        if (ret < 0)
        {
                printk("Failed to enable UART RX: %d\n", ret);
                return 1;
        }

        printk("UART LED control ready\n");
        return 0;
}
