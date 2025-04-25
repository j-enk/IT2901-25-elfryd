#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include "adc_driver.h"

/* Ensure a devicetree overlay with io-channels is provided */
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

/* Retrieve the ADC channel configuration from the devicetree (index 0) */
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

/* Declare a single-sample buffer with proper alignment */
static int16_t sample_buffer[1] __aligned(4);

int adc_driver_init(void)
{
    int err;

    if (!adc_is_ready_dt(&adc_channel))
    {
        printk("ADC device %s is not ready\n", adc_channel.dev->name);
        return -ENODEV;
    }

    err = adc_channel_setup_dt(&adc_channel);
    if (err < 0)
    {
        printk("Could not setup ADC channel (err %d)\n", err);
        return err;
    }

    return 0;
}

int adc_driver_read(int32_t *voltage_mv)
{
    int err;
    struct adc_sequence sequence = {
        .buffer = sample_buffer,
        .buffer_size = sizeof(sample_buffer),
    };

    /* Initialize the sequence structure from the device tree spec */
    (void)adc_sequence_init_dt(&adc_channel, &sequence);

    err = adc_read_dt(&adc_channel, &sequence);
    if (err < 0)
    {
        printk("Could not read ADC (err %d)\n", err);
        return err;
    }

    int32_t millivolts = sample_buffer[0];
    err = adc_raw_to_millivolts_dt(&adc_channel, &millivolts);
    if (err < 0)
    {
        printk("mV conversion not available (err %d)\n", err);
        return err;
    }
    else
    {
        /* Multiply by 2 to compensate for the voltage divider (R1 = R2) */
        *voltage_mv = millivolts * 2;
    }

    return 0;
}
