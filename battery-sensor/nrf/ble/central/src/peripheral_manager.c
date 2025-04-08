#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include "peripheral_manager.h"
#include "connection_pool.h"
#include <zephyr/bluetooth/uuid.h>

static struct bt_conn *default_conn;

static bool is_scanning = false;

static const uint8_t target_uuid[16] = {
    0xCD, 0xEE, 0x3D, 0x67, 
    0x35, 0xCD, 0x3A, 0x94,
    0x1D, 0x45, 0xBD, 0xB7,
    0x5E, 0x67, 0x70, 0xBF
};

static void print_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printk("%02x ", data[i]);
    }
}

static bool parse_debug(struct bt_data *data, void *user_data) {
    bool *found = (bool *)user_data;
    
    printk("[AD Data] Type: 0x%02x, Length: %u, Data: ", data->type, data->data_len);
    print_hex(data->data, data->data_len);
    printk("\n");

    if (data->type == BT_DATA_UUID128_ALL || data->type == BT_DATA_UUID128_SOME) {
        printk("  UUID128 detected - ");
        if (data->data_len == 16) {
            printk("Valid length\n");
            if (memcmp(data->data, target_uuid, 16) == 0) {
                printk("  >> UUID MATCH FOUND <<\n");
                *found = true;
                return false;
            } else {
                printk("  No match\n");
            }
        } else {
            printk("Invalid length (expected 16, got %u)\n", data->data_len);
        }
    }
    return true;
}

bool debug_adv_data(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                   struct net_buf_simple *ad) {
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    
    printk("\n=== Advertisement Packet ===\n");
    printk("Device: %s\n", addr_str);
    printk("RSSI: %d dBm\n", rssi);
    printk("Type: 0x%02x (%s)\n", type, 
          (type == BT_GAP_ADV_TYPE_ADV_IND) ? "Connectable Undirected" :
          (type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) ? "Connectable Directed" :
          "Other");
    
    bool found = false;
    bt_data_parse(ad, parse_debug, &found);
    
    printk("UUID Match: %s\n", found ? "YES" : "NO");
    printk("=== End of Packet ===\n");
    return found;
}

static bool check_uuid(struct bt_data *data, void *user_data) {
    bool *found = (bool *)user_data;
    
    if ((data->type == BT_DATA_UUID128_ALL || data->type == BT_DATA_UUID128_SOME) &&
        (data->data_len == 16) && 
        (memcmp(data->data, target_uuid, 16) == 0)) {
        *found = true;
        return false;
    }
    return true;
}


static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                        struct net_buf_simple *ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];
    int err;

    /* We're only interested in connectable events */
    if (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        return;
    }

    /* Parse advertisement data */
    bool found = false;

    // bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    // if (strncmp(addr_str, "FF:EE:DD:CC:BB:AA", 17) != 0) {
	// 	return;
	// }

    // printk("Device found: %s (RSSI %d)\n", addr_str, rssi);
    if(!debug_adv_data(addr, rssi, type, ad)){
        return;
    }
    // bt_data_parse(ad, check_uuid, &found);
    
    // if (!found) {
    //     return; // Skip if our UUID not found
    // }

    


    if (bt_le_scan_stop()) {
        return;
    }
    is_scanning = false;

    if (connection_pool_count() >= CONFIG_BT_MAX_CONN) {
        printk("Connection pool full, not connecting to %s\n", addr_str);
        return;
    }

    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, 
                           BT_LE_CONN_PARAM_DEFAULT, &default_conn);
    if (err) {
        printk("Create conn to %s failed (%d)\n", addr_str, err);
        peripheral_manager_start_scan();
    }

}

void peripheral_manager_start_scan(void)
{
    if (is_scanning) {
        printk("[SCAN] Already scanning. Skipping start.\n");
        return;
    }

    int err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
    if (err) {
        printk("Scanning failed to start (err %d)\n", err);
    } else {
        printk("[SCAN] Started.\n");
        is_scanning = true;
    }
}

void peripheral_manager_init(void)
{
    /* Initialization if needed */
}
