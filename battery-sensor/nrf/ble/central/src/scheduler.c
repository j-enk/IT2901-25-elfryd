#include <zephyr/kernel.h>
#include <zephyr/bluetooth/gatt.h>
#include "scheduler.h"
#include "connection_pool.h"
#include "peripheral_manager.h"

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_read_params read_params;

#define MAX_CONNECTIONS CONFIG_BT_MAX_CONN
#define SCAN_INTERVAL_SECONDS 10

static void scan_if_needed(struct k_timer *timer)
{
    printk("[SCAN] Checking if scan is needed...\n");
    if (connection_pool_count() < 16) {
        peripheral_manager_start_scan();  // Function to kick off scanning
    }
    printk("[SCAN] Scan check complete.  connections: %d\n",connection_pool_count());
}

static uint8_t read_callback(struct bt_conn *conn, uint8_t err,
    struct bt_gatt_read_params *params,
    const void *data, uint16_t length)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
    printk("[GATT READ ERROR] Device: %s, Handle: 0x%04x, Error: 0x%02x\n",
    addr, params->single.handle, err);
    return BT_GATT_ITER_STOP;
    }

    if (!data) {
    printk("[GATT READ COMPLETE] Device: %s, Handle: 0x%04x, Status: No data\n",
    addr, params->single.handle);
    return BT_GATT_ITER_STOP;
    }

    printk("[GATT READ SUCCESS] Device: %s\n", addr);
    printk("  Handle: 0x%04x\n", params->single.handle);
    printk("  Data Length: %u bytes\n", length);

    // Hex dump of received data
    printk("  Raw Data: ");
    for (uint16_t i = 0; i < length; i++) {
    printk("%02x ", ((const uint8_t *)data)[i]);
    }
    printk("\n");

    // Special handling for 4-byte values
    if (length == 4) {
    int32_t value = *((int32_t *)data);
    printk("  Interpreted as:\n");
    printk("    int32_t: %d\n", value);
}

return BT_GATT_ITER_STOP;
}

static void read_peripheral_data(struct bt_conn *conn)
{
    memset(&read_params, 0, sizeof(read_params));
    
    read_params.func = read_callback;
    read_params.handle_count = 0;
    read_params.by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    read_params.by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    uuid.val = BT_UUID_GATT_V_VAL;
    read_params.by_uuid.uuid = &uuid.uuid;

    bt_gatt_read(conn, &read_params);
}

static void schedule_read(struct k_timer *timer)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct bt_conn *conn = connection_pool_get(i);
        if (conn) {
            read_peripheral_data(conn);
        }
    }
}

K_TIMER_DEFINE(read_timer, schedule_read, NULL);
K_TIMER_DEFINE(scan_timer, scan_if_needed, NULL);

void scheduler_init(void)
{
    k_timer_start(&read_timer, K_SECONDS(2), K_SECONDS(2));
    k_timer_start(&scan_timer, K_SECONDS(9), K_SECONDS(SCAN_INTERVAL_SECONDS));
}