/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

// typedef struct{
// 	struct bt_conn *kobling;
// 	struct bt_addr_le_t address;
// } Connections;
// int connection_index = 0;

#define MAX_CONNECTIONS 2
static uint8_t conn_count;
static bool volatile is_disconnecting;
// Connections connections[MAX_CONNECTIONS];


static const uint8_t target_uuid[16] = {
    0xCD, 0xEE, 0x3D, 0x67, 
    0x35, 0xCD, 0x3A, 0x94,
    0x1D, 0x45, 0xBD, 0xB7,
    0x5E, 0x67, 0x70, 0xBF
};

static void regular(struct k_timer *);
K_TIMER_DEFINE(regular_timer, regular, NULL);

static void start_scan(void);

static struct bt_conn *connections[MAX_CONNECTIONS];

static void print_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printk("%02x ", data[i]);
    }
}

static bool parse_debug(struct bt_data *data, void *user_data) {
    bool *found = (bool *)user_data;

    if (data->type == BT_DATA_UUID128_ALL || data->type == BT_DATA_UUID128_SOME) {
        if (data->data_len == 16) {
            if (memcmp(data->data, target_uuid, 16) == 0) {
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

    bool found = false;
    bt_data_parse(ad, parse_debug, &found);
    return found;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;
	// struct bt_data data;

	if (conn_count >= MAX_CONNECTIONS) {
		printk("Max connections reached\n");
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

	if(!debug_adv_data(addr, rssi, type, ad)){
        return;
    }

	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	if (bt_le_scan_stop()) {
		return;
	}

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT,
				&connections[conn_count]);
	if (err) {
		printk("Create conn to %s failed (%d)\n", addr_str, err);
		start_scan();
	}	
}

static void start_scan(void)
{
	int err;

	//Starts the scanning on chip
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("\nScanning successfully started\n");
}

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_read_params read_params;

static uint8_t read_func(struct bt_conn *conn, uint8_t err,
	struct bt_gatt_read_params *params,
	const void *data, uint16_t length)
{
char addr[BT_ADDR_LE_STR_LEN];
bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

printk("\n[GATT READ CALLBACK]\n");
printk("Device: %s\n", addr);

if (!data) {
printk("  ✅ GATT Read complete. No more data.\n");
(void)memset(params, 0, sizeof(*params));
printk("[/GATT READ CALLBACK]\n");
return BT_GATT_ITER_STOP;
}

printk("  ✅ GATT Read successful\n");
printk("  Handle: 0x%04x\n", params->single.handle);
printk("  Data Length: %u bytes\n", length);
printk("  Raw Data: ");
for (uint16_t i = 0; i < length; i++) {
printk("%02x ", ((const uint8_t *)data)[i]);
}
printk("\n");

if (length == 4) {
int32_t value = *((int32_t *)data);
printk("  Interpreted Value:\n");
printk("    - int32_t: %d\n", value);
}

printk("[/GATT READ CALLBACK]\n");
return BT_GATT_ITER_CONTINUE;
}


static int cmd_read_uuid(struct bt_conn *conn, int target)
{
	int err;

	if (!conn) {
		printk("Not connected\n");
		return -ENOEXEC;
	}

	if (read_params.func) {
		printk("Read ongoing\n");
		return -ENOEXEC;
	}

	read_params.func = read_func;
	read_params.handle_count = 0;
	read_params.by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	read_params.by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;

	uuid.val = target;
	if (uuid.val) {
		read_params.by_uuid.uuid = &uuid.uuid;
	} else {
		printk("setting uuid failed\n");
		return -1;
	}

	err = bt_gatt_read(conn, &read_params);
	if (err) {
		printk("Read failed (err %d)\n", err);
	} else {
		printk("Read pending\n");
	}

	return err;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s %u %s\n", addr, err, bt_hci_err_to_str(err));

		start_scan();
		return;
	}

	conn_count++;
	if (conn_count < MAX_CONNECTIONS) {
		start_scan();
		printk("Scanning for more devices...\n");
	}
	connections[conn_count] = bt_conn_ref(conn);
	printk("Connected (%u): %s\n", conn_count, addr);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	for (int i = 0; i < conn_count; i++) {
		if (connections[i] == conn) {
			bt_conn_unref(connections[i]);
			for (int j = i; j < conn_count - 1; j++) {
				connections[j] = connections[j + 1];
			}
			conn_count--;
			break;
		}
	}
	start_scan();	
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void foreach_function(struct bt_conn *conn, void *data)
{
	struct bt_conn_remote_info remote_info;
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Connection #%d: %s\n", *(uint8_t *)data + 1, addr);

	err = bt_conn_get_remote_info(conn, &remote_info);
	if (err) {
		printk("  ❌ Failed to get remote info: err %d\n", err);
		return;
	}

	// Optional: check and print features, PHY, or other remote info here if needed

	uint8_t *actual_count = (uint8_t *)data;
	(*actual_count)++;

	printk("  🔄 Initiating GATT read by UUID...\n");
	int read_err = cmd_read_uuid(conn, BT_UUID_GATT_V_VAL);
	if (read_err) {
		printk("  ❌ GATT read failed with err %d\n", read_err);
	} else {
		printk("  📬 GATT read pending\n");
	}
}


static void regular(struct k_timer *) {
	if(connections[0] != NULL) {
		// cmd_read_uuid(BT_UUID_GATT_V_VAL);
		uint8_t actual_count = 0U;
		bt_conn_foreach(BT_CONN_TYPE_LE, foreach_function, &actual_count);
		printk("foreach suceeded for %d devices\n", actual_count);
	} else {
		printk("not connected\n");
	}
}

int main(void)
{
	int err;

	//Enable bluetooth
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");
	k_timer_start(&regular_timer, K_SECONDS(2), K_SECONDS(2));

	//scan for a connection
	start_scan();
	
	return 0;
}
