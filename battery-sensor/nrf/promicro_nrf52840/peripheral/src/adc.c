#include "adc.h"
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

int baut_adc_init(void) {
    if (!adc_is_ready_dt(&adc_channel)) {
        printk("ADC controller device %s not ready\n", adc_channel.dev->name);
        return -1;
    }

    int err = adc_channel_setup_dt(&adc_channel);
    if (err) {
        printk("Could not setup channel #%d (%d)\n", 0, err);
        return err;
    }

    return 0;
}

int baut_adc_read(int32_t *mv) {
    int16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };
    int err = adc_sequence_init_dt(&adc_channel, &sequence);
    if (err) {
        printk("Could not initialize sequence\n");
        return -1;
    }

    err = adc_read(adc_channel.dev, &sequence);
    if (err) {
        printk("Could not read (%d)\n", err);
        return -1;
    }

    int32_t val_mv = buf;
    err = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
    if (err) {
        printk("adc_raw_to_millivolts_dt failed: %d\n", err);
        return -1;
    }

    val_mv *= 6.23;
    *mv = val_mv;
    return 0;
}