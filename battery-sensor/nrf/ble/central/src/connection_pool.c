#include <zephyr/bluetooth/conn.h>
#include "connection_pool.h"

#define MAX_CONNECTIONS CONFIG_BT_MAX_CONN

static struct bt_conn *connections[MAX_CONNECTIONS];

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        printk("Failed to connect to %s (%u)\n", addr, err);
        return;
    }

    /* Add to connection pool */
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!connections[i]) {
            connections[i] = bt_conn_ref(conn);
            printk("Connected: %s (slot %d)\n", addr, i);
            break;
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    /* Remove from connection pool */
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] == conn) {
            bt_conn_unref(connections[i]);
            connections[i] = NULL;
            printk("Disconnected: %s (slot %d)\n", addr, i);
            break;
        }
    }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

void connection_pool_init(void)
{
    memset(connections, 0, sizeof(connections));
}

struct bt_conn *connection_pool_get(int index)
{
    if (index < 0 || index >= MAX_CONNECTIONS) {
        return NULL;
    }
    return connections[index];
}

int connection_pool_count(void)
{
    int count = 0;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i]) {
            count++;
        }
    }
    return count;
}