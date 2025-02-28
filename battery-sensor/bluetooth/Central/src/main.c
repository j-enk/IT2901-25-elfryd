/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #include <zephyr/kernel.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/bluetooth/bluetooth.h>
 #include <zephyr/bluetooth/gap.h>
 #include <zephyr/bluetooth/gatt.h>
 #include <zephyr/bluetooth/uuid.h>
 #include <zephyr/bluetooth/addr.h>
 /* STEP 1 - Include the header file for managing Bluetooth LE Connections */
 #include <zephyr/bluetooth/conn.h>
 /* STEP 8.2 - Include the header file for the LED Button Service */
 #include <bluetooth/services/lbs.h>
 #include <dk_buttons_and_leds.h>
 #include <zephyr/sys/byteorder.h>

 
/*
Define Area
*/
LOG_MODULE_REGISTER(Bluetooth_testing, LOG_LEVEL_INF);
 
/*
Struct area
*/
 



/*
Function area
*/
void cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
	struct net_buf_simple *buf)
{


// Convert and print the advertiser's address
char addr_str[BT_ADDR_LE_STR_LEN];
snprintf(addr_str, sizeof(addr_str),
         "%02X:%02X:%02X:%02X:%02X:%02X",
         addr->a.val[5], addr->a.val[4], addr->a.val[3], 
         addr->a.val[2], addr->a.val[1], addr->a.val[0]);

if(strcmp(addr_str, "FF:EE:DD:CC:BB:AA") == 0){
LOG_INF("Callback called");
LOG_INF("Address: %s", addr_str);

// Print RSSI value
LOG_INF("RSSI: %d dBm", rssi);

// Print advertisement type
LOG_INF("Adv type: %u", adv_type);

// Print the raw advertising data
LOG_INF("Advertising Data (Length: %d):", buf->len);
for (int i = 0; i < buf->len; i++) {
	LOG_INF("%02X ", buf->data[i]);
}

// Decode specific advertising data
struct net_buf_simple temp_buf;
net_buf_simple_clone(buf, &temp_buf);

while (temp_buf.len > 0) {
	uint8_t length = net_buf_simple_pull_u8(&temp_buf);
	if (length == 0 || length > temp_buf.len) {
		break;  // Invalid length
	}
	
	uint8_t type = net_buf_simple_pull_u8(&temp_buf);
	length--;  // One byte used by 'type'

	switch (type) {
	case BT_DATA_NAME_COMPLETE:
	case BT_DATA_NAME_SHORTENED:
		LOG_INF("Device Name: %.*s", length, temp_buf.data);
		break;
	case BT_DATA_UUID16_ALL:
	case BT_DATA_UUID16_SOME:
		LOG_INF("UUID16 Services:");
		for (int i = 0; i < length; i += 2) {
			uint16_t uuid = sys_get_le16(&temp_buf.data[i]);
			LOG_INF("  - 0x%04X", uuid);
		}
		break;
	case BT_DATA_TX_POWER:
		LOG_INF("Tx Power: %d dBm", *(int8_t *)temp_buf.data);
		break;
	default:
		LOG_INF("Unknown AD Type: 0x%02X", type);
	}
	
	net_buf_simple_pull(&temp_buf, length);
}
LOG_INF("-----------------------------");
}}




//main 
 int main(void)
 {
	int err;

	//Må callback her?

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return -1;
	}

	err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, cb);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return -1;
	}
 }
 