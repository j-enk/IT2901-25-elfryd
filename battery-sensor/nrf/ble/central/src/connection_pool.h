#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <zephyr/bluetooth/conn.h>

void connection_pool_init(void);
struct bt_conn *connection_pool_get(int index);
int connection_pool_count(void);

#endif