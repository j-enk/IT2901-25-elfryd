#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

const struct device *uart;

void main(void)
{
        uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
        if (!device_is_ready(uart))
        {
                return;
        }

        while (1)
        {
                char c = '1';
                uart_tx(uart, (const uint8_t *)&c, 1, SYS_FOREVER_US);
                k_sleep(K_SECONDS(1));

                c = '0';
                uart_tx(uart, (const uint8_t *)&c, 1, SYS_FOREVER_US);
                k_sleep(K_SECONDS(1));
        }
}
