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


int max_connections = 16;
typedef struct{
	struct bt_conn *kobling;
	struct bt_addr_le_t address;
} Connections;
int connection_index = 0;
Connections *connections = (*Connections) malloc(sizeof(Connections)*max_connections);


static const uint8_t filter_uuid[] = {
    0xCD, 0xEE, 0x3D, 0x67,  
    0x35, 0xCD, 0x3A, 0x94,  
    0x1D, 0x45, 0xBD, 0xB7,  
    0x5E, 0x67, 0x70, 0xBF  
};

static void regular(struct k_timer *);
K_TIMER_DEFINE(regular_timer, regular, NULL);

static void start_scan(void);

static struct bt_conn *default_conn;


static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;
	struct bt_data data;

	if (default_conn) {
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

	/* connect only to devices in close proximity */
	// if (rssi < -60) {
	// 	return;
	// }

	// Usikker på om dette er riktig måte å gjøre det på
	bt_data_parse(ad, [](struct bt_data *data, void *user_data) {
        if (data->type == BT_DATA_UUID128_ALL && data->data_len == 16) {
            if (memcmp(data->data, target_uuid, 16) == 0) {
                *(bool *)user_data = true;
            }
        }
    }, &found_uuid);

    // If UUID was not found, return
    if (!found_uuid) return;
	int found = 0;
	for(int i = 0; i<max_connections;i++){
		if (strncmp(addr_str, connections[i], 17) == 0) {
			found = 1;
			break;
		}
	}
	if(!found)return;
	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	if (bt_le_scan_stop()) {
		return;
	}

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT,
				&connections[connection_index].kobling);
	if (err) {
		printk("Create conn to %s failed (%d)\n", addr_str, err);
		start_scan();
	}	
	int err = bt_addr_le_from_str(addr_str, &connections[connection_index++].addr);
	if (err) {
		printk("Failed to parse address: %s\n", addr_str);
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
	printk("Read complete: err 0x%02x length %u\n", err, length);

	if (!data) {
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	} else {
		// bt_shell_hexdump(data, length);
		if(length == 4) {
			// assume endianess is correct :)
			int32_t value = *((int32_t *)data);
			printk("as int %d\n", value);
		}
	}

	return BT_GATT_ITER_CONTINUE;
}

static int cmd_read_uuid(int target)
{
	int err;

	if (!default_conn) {
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

	err = bt_gatt_read(default_conn, &read_params);
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

		bt_conn_unref(default_conn);
		default_conn = NULL;

		start_scan();
		return;
	}

	if (conn != default_conn) {
		return;
	}

	printk("Connected: %s\n", addr);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != default_conn) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s, reason 0x%02x %s\n", addr, reason, bt_hci_err_to_str(reason));

	bt_conn_unref(default_conn);
	default_conn = NULL;

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};


static void regular(struct k_timer *) {
	if(default_conn) {
		cmd_read_uuid(BT_UUID_GATT_V_VAL);
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
