/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>

#if defined(CONFIG_APP_EVENT_MANAGER)
#include <app_event_manager.h>
#endif /* defined(APP_EVENT_MANAGER) */

#include "gpio_utils.h"
#include "mqtt_helpers.h"
#include "nfc_helpers.h"
#include "nfc_event.h"
#include "lock_detect_event.h"

LOG_MODULE_REGISTER(mqtt_simple, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

#define NFC_THREAD_STACK_SIZE 1024
#define NFC_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(nfc_stack_area, NFC_THREAD_STACK_SIZE);
struct k_thread nfc_thread_data;

void nfc_event_thread(void *arg1, void *arg2, void *arg3)
{
	while (true)
	{
		k_poll(events, ARRAY_SIZE(events), K_FOREVER);
		int err = st25r3911b_nfca_process();
		if (err)
		{
			printk("NFC-A process failed, err: %d.\n", err);
			return;
		}
	}
}

void main(void)
{

	/* GPIO set-up */
	int err = gpio_setup();
	if (err < 0)
	{
		LOG_ERR("GPIO setup failed, stopping");
		return;
	}

	err = setup_nfc();
	if (err)
	{
		LOG_ERR("NFC setup failed, stopping");
		return;
	}

	uint32_t connect_attempt = 0;

	LOG_INF("The MQTT simple sample started");

#if defined(CONFIG_MQTT_LIB_TLS)
	err = certificates_provision();
	if (err)
	{
		LOG_ERR("Failed to provision certificates");
		return;
	}
#endif /* defined(CONFIG_MQTT_LIB_TLS) */

	do
	{
		err = modem_configure();
		if (err)
		{
			LOG_INF("Retrying in %d seconds",
					CONFIG_LTE_CONNECT_RETRY_DELAY_S);
			k_sleep(K_SECONDS(CONFIG_LTE_CONNECT_RETRY_DELAY_S));
		}
	} while (err);

	err = client_init(&client);
	if (err != 0)
	{
		LOG_ERR("client_init: %d", err);
		return;
	}

	app_event_manager_init();

	k_tid_t tid = k_thread_create(
		&nfc_thread_data,
		nfc_stack_area,
		K_THREAD_STACK_SIZEOF(nfc_stack_area),
		nfc_event_thread,
		NULL, NULL, NULL,
		NFC_THREAD_PRIORITY, 0, K_NO_WAIT);

	if (tid == 0)
	{
		LOG_ERR("Failed to create nfc thread, stopping");
		return;
	}

do_connect:
	if (connect_attempt++ > 0)
	{
		LOG_INF("Reconnecting in %d seconds...",
				CONFIG_MQTT_RECONNECT_DELAY_S);
		k_sleep(K_SECONDS(CONFIG_MQTT_RECONNECT_DELAY_S));
	}
	err = mqtt_connect(&client);
	if (err != 0)
	{
		LOG_ERR("mqtt_connect %d", err);
		goto do_connect;
	}

	err = fds_init(&client);
	if (err != 0)
	{
		LOG_ERR("fds_init: %d", err);
		return;
	}

	while (1)
	{

		err = poll(&fds, 1, mqtt_keepalive_time_left(&client));
		if (err < 0)
		{
			LOG_ERR("poll: %d", errno);
			break;
		}

		err = mqtt_live(&client);
		if ((err != 0) && (err != -EAGAIN))
		{
			LOG_ERR("ERROR: mqtt_live: %d", err);
			break;
		}

		if ((fds.revents & POLLIN) == POLLIN)
		{
			err = mqtt_input(&client);
			if (err != 0)
			{
				LOG_ERR("mqtt_input: %d", err);
				break;
			}
		}

		if ((fds.revents & POLLERR) == POLLERR)
		{
			LOG_ERR("POLLERR");
			break;
		}

		if ((fds.revents & POLLNVAL) == POLLNVAL)
		{
			LOG_ERR("POLLNVAL");
			break;
		}
	}

	LOG_INF("Disconnecting MQTT client...");

	err = mqtt_disconnect(&client);
	if (err)
	{
		LOG_ERR("Could not disconnect MQTT client: %d", err);
	}
	goto do_connect;
}
