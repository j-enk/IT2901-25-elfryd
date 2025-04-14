/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #ifndef __CONFIG_H__
 #define __CONFIG_H__
 
 #define ZEPHYR_ADDR		"192.168.1.101"
 #define SERVER_ADDR		"20.93.19.231"
 #define SERVER_PORT		8885
 
 #define APP_CONNECT_TIMEOUT_MS	2000
 #define APP_SLEEP_MSECS		500
 #define APP_CONNECT_TRIES	    10
 #define APP_MQTT_BUFFER_SIZE	128
 
 #define MQTT_CLIENTID		"elfryd_hub"
 
 #endif
