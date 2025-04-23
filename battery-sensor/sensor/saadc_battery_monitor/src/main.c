#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include "adc_driver.h"

int main(void)
{
        int ret;
        uint32_t count = 0;
        int32_t voltage_mv;

        ret = adc_driver_init();
        if (ret < 0)
        {
                printk("ADC initialization failed (err %d)\n", ret);
                return ret;
        }

        while (1)
        {
                printk("ADC reading [%u]: ", count++);
                ret = adc_driver_read(&voltage_mv);
                if (ret < 0)
                {
                        printk("Error reading ADC (err %d)\n", ret);
                }
                else
                {
                        printk("%" PRId32 " mV (Input Voltage)\n", voltage_mv);
                }
                k_sleep(K_MSEC(1000));
        }

        return 0;
}
