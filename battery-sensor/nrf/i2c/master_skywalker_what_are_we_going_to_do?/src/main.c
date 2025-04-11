#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_master, LOG_LEVEL_INF);

#define I2C_ADDR 0x69  // Address of the TinyGo target device

int main(void)
{

    uint8_t buf[16];
    int num_bytes = 4;
    int ret;

    // shell get device
    const struct device *dev = device_get_binding("i2c@40003000");

	if (IS_ENABLED(CONFIG_DEVICE_DT_METADATA) && dev == NULL) {
		dev = device_get_by_dt_nodelabel("i2c@40003000");
	}
    if(!dev){
        LOG_ERR("I2C device not found");
        return 0; 
    }

    while(1){
        k_msleep(1000);
    ret = i2c_read(dev, buf, num_bytes, 0x69);
    if (ret) {
        LOG_ERR("I2C read failed: %d", ret);
    }
    else{
        printk("I2C read successful: %02x %02x %02x %02x", 
            (unsigned int)buf[0], (unsigned int)buf[1], 
            (unsigned int)buf[2], (unsigned int)buf[3]);
    }
    }

    return 0;
}
