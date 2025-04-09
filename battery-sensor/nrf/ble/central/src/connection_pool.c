#include "connection_scanner.h"

static uint8_t conn_count;
struct bt_conn *connections[CONFIG_BT_MAX_CONN];


static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("\n[CONNECTED CALLBACK] Attempting to connect to %s\n", addr);

    if (err) {
        printk("❌ Connection failed with error code %u\n", err);
        return;
    }

    printk("✅ Connection successful! Scanning for empty slot...\n");

    bool added = false;

    for (int i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (connections[i] == NULL) {
            conn_count++;
            printk("📌 Assigned connection to slot %d\n", i);
            printk("🔢 Connection count incremented: %d\n", conn_count);
            added = true;
            break;
        } else {
            char existing[BT_ADDR_LE_STR_LEN];
            bt_addr_le_to_str(bt_conn_get_dst(connections[i]), existing, sizeof(existing));
            printk("  🔒 Slot %d already in use by %s\n", i, existing);
        }
    }

    if (!added) {
        printk("⚠️  No available slots for %s — connection NOT stored!\n", addr);
    }

    printk("🔁 Restarting scan...\n");
    peripheral_manager_start_scan();
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
    peripheral_manager_start_scan();
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
    if (index < 0 || index >= CONFIG_BT_MAX_CONN) {
        return NULL;
    }
    return connections[index];
}

int connection_pool_count(void)
{
    int count = 0;
    for (int i = 0; i < CONFIG_BT_MAX_CONN; i++) {
        if (connections[i]) {
            count++;
        }
    }
    return count;
}