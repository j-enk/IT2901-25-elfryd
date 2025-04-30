#include "bluetooth.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>

static ATOMIC_DEFINE(state, 2U);

#define STATE_CONNECTED    1U
#define STATE_DISCONNECTED 2U

static void connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
        printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
    } else {
        printk("Connected!!!\n");
        (void)atomic_set_bit(state, STATE_CONNECTED);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
    printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
    (void)atomic_set_bit(state, STATE_DISCONNECTED);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

void bluetooth_init(void) {
    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }
    printk("Bluetooth initialized\n");
}