#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

#include "ble.h"
#include "led.h"
#include "sensors/uuid.h"

int ble_init(void)
{
    int err;

    // Initialize Bluetooth stack
    err = bt_enable(NULL);
    if (err) {
        return err;
    }
}

/*
-----------------------------------------------------------------------------
                    ADVERTISING AND CONNECTION HANDLING
-----------------------------------------------------------------------------
*/

int is_connected = 0; // 0 = disconnected, 1 = connected

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_UUID128_ALL, custom_uuid, sizeof(custom_uuid)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

int ble_advertise(){
    int err;

    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        return err;
    }
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
    } else {
        is_connected = 1;
        led_set_connection_state(true);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    is_connected = 0;
    led_set_connection_state(false);
    ble_advertise();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/*
-----------------------------------------------------------------------------
                               GATT Service
-----------------------------------------------------------------------------
*/

static ssize_t id_read_function(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
    uint16_t len, uint16_t offset)
{
uint8_t sensor_id = CONFIG_BATTERY_ID;

return bt_gatt_attr_read(conn, attr, buf, len, offset, &sensor_id, sizeof(sensor_id));
}

BT_GATT_SERVICE_DEFINE(id_srvc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ID),
    BT_GATT_CHARACTERISTIC(BT_UUID_ID, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                   id_read_function, NULL, NULL),
);