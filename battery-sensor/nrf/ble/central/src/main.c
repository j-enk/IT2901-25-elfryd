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

#define MAX_CONNECTIONS 16
static uint8_t volatile conn_count;
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

static struct bt_conn *default_conn;

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

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;
	// struct bt_data data;

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
	// printk("DATA PARSE...\n")
	// bt_data_parse(ad, [](struct bt_data *data, void *user_data) {
    //     if (data->type == BT_DATA_UUID128_ALL && data->data_len == 16) {
    //         if (memcmp(data->data, target_uuid, 16) == 0) {
    //             *(bool *)user_data = true;
    //         }
    //     }
    // }, &found_uuid);
	// printk("DATA PARSE FINISH: %s\n")

    // If UUID was not found, return
    // if (!found_uuid) return;

	// nrf51dks:
	// D6:13:6C:7E:F6:C6
	// C9:22:D6:F4:67:96

	// if (
	// 	strncmp(addr_str, "DB:F7:59:82:13:DA", 17) != 0
	// 	&&
	// 	strncmp(addr_str, "C9:22:D6:F4:67:96", 17) != 0
	// ) {
	// 	printk("Skip: %s (RSSI %d)\n", addr_str, rssi);
	// 	return;
	// }
	if(!debug_adv_data(addr, rssi, type, ad)){
        return;
    }

	printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

	if (bt_le_scan_stop()) {
		return;
	}

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT,
				&default_conn);
	if (err) {
		printk("Create conn to %s failed (%d)\n", addr_str, err);
		start_scan();
	}	
	// int err = bt_addr_le_from_str(addr_str, &connections[connection_index++].addr);
	// if (err) {
	// 	printk("Failed to parse address: %s\n", addr_str);
	// }
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

		bt_conn_unref(default_conn);
		default_conn = NULL;

		start_scan();
		return;
	}

	// if (conn != default_conn) {
	// 	printk("Uhohnr2??\n")
	// 	return;
	// }

	conn_count++;
	if (conn_count < MAX_CONNECTIONS) {
		start_scan();
	}

	printk("Connected (%u): %s\n", conn_count, addr);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != default_conn) {
		printk("UH OH? or ok?\n");
		// return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s, reason 0x%02x %s\n", addr, reason, bt_hci_err_to_str(reason));

	bt_conn_unref(default_conn);
	default_conn = NULL;

	// start_scan();
	if ((conn_count == 1U) && (is_disconnecting || (reason == BT_HCI_ERR_CONN_FAIL_TO_ESTAB))) {
		is_disconnecting = false;
		start_scan();
	}
	conn_count--;
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

	printk("Get remote info %s...\n", addr);
	err = bt_conn_get_remote_info(conn, &remote_info);
	if (err) {
		printk("Failed remote info %s.\n", addr);
		return;
	}
	printk("success.\n");

	uint8_t *actual_count = (void *)data;

	(*actual_count)++;

	cmd_read_uuid(conn, BT_UUID_GATT_V_VAL);
}


static void regular(struct k_timer *) {
	if(default_conn) {
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
