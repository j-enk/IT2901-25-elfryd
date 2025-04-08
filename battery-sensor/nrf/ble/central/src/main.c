#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include "peripheral_manager.h"
#include "connection_pool.h"
#include "scheduler.h"

int main(void)
{
    int err;

    /* Initialize Bluetooth */
    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return 0;
    }

    printk("Bluetooth initialized\n");

    /* Initialize components */
    connection_pool_init();
    scheduler_init();

    /* Start operations */
    peripheral_manager_start_scan();
    
    return 0;
}