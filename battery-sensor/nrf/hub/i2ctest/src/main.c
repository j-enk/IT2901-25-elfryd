#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

int main(void){
    
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

    if (!device_is_ready(dev))
    {
        printk("I2C device not ready\n");
        return -ENODEV;
    }

    uint8_t buf[4];
    uint16_t addr = 0x69; // I2C address of the device
    uint8_t reg = 0x01;

    int ret = i2c_write_read(dev, addr, &reg, 1, buf, sizeof(buf));
    if (ret) {
        printk("Error reading response: %d\n", ret);
    } else {
        printk("Received ID: %u\n", buf[0]);
        printk("Received Payload: %02x %02x %02x\n",
               buf[1], buf[2], buf[3]);
    }
    if (ret < 0)
    {
        printk("\nI2C read failed with error: %d\n", ret);
        return ret;
    }
    
    printk("Data read from I2C device: %02X, %02X, %02X, %02X\n", 
           buf[0], buf[1], buf[2], buf[3]);
    
    return 0;
}