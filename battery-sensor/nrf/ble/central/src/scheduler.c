#include <zephyr/kernel.h>
#include <zephyr/bluetooth/gatt.h>
#include "scheduler.h"
#include "connection_scanner.h"

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
struct bt_gatt_read_params read_params_array[CONFIG_BT_MAX_CONN];


static uint8_t read_callback(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params, const void *data, uint16_t length)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("\n[GATT READ CALLBACK]\n");
	printk("Device: %s\n", addr);

	if (!data) {
	printk("  ✅ GATT Read complete. No more data.\n");
	printk("[/GATT READ CALLBACK]\n");
	return BT_GATT_ITER_STOP;
	}

	printk("  ✅ GATT Read successful\n");
	printk("  Handle: 0x%04x\n", params->single.handle);
	printk("  Data Length: %u bytes\n", length);

	if (length == 4) {
	int32_t value = *((int32_t *)data);
	printk("  Interpreted Value:\n");
	printk("    - int32_t: %d\n", value);
	}

	printk("[/GATT READ CALLBACK]\n");
	return BT_GATT_ITER_CONTINUE;
}



static void gatt_read(struct bt_conn *conn, int index)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    printk("GATT Read for: %s\n", addr);

    

    read_params_array[index].func = read_callback;
    read_params_array[index].by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    read_params_array[index].by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    uuid.val = BT_UUID_GATT_V_VAL;
    read_params_array[index].by_uuid.uuid = &uuid.uuid;

    err = bt_gatt_read(conn, &read_params_array[index]);
    if (err) {
        printk("  ❌ GATT read failed with err %d\n", err);
    } else {
        printk("  📬 GATT read pending\n");
    }
}





static void schedule_read(struct k_timer *timer)
{
    for (int i = 0; i < connection_pool_count(); i++) {
        struct bt_conn *conn = connection_pool_get(i);
        if (conn) {
			printk("READING GYYYAAATTT");
            gatt_read(conn, i);
        }
    }
}




K_TIMER_DEFINE(read_timer, schedule_read, NULL);



void scheduler_init(void)
{
    k_timer_start(&read_timer, K_SECONDS(2), K_SECONDS(2));
}