#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <zephyr/bluetooth/conn.h>

extern struct bt_conn *connections[CONFIG_BT_MAX_CONN];

void connection_pool_init(void);
struct bt_conn *connection_pool_get(int index);
int connection_pool_count(void);

void peripheral_manager_init(void);
void peripheral_manager_start_scan(void);

#endif