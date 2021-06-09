/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include "model_handler.h"
#include "pca20036_ethernet.h"
#include "ethernet_command_system.h"
#include "ethernet_dfu.h"
#include "hp_led.h"
#include "model_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(test, CONFIG_LOG_DEFAULT_LEVEL);

/////////////////////////////// ETHERNET RX WORK ///////////////////////////////

static struct k_delayed_work ethernet_rx_work;

static void ethernet_rx_work_handler(struct k_work *work)
{
	ethernet_command_system_rx();
	k_delayed_work_submit(&ethernet_rx_work, K_MSEC(10));
}

static void ethernet_rx_work_init_start(void)
{
	k_delayed_work_init(&ethernet_rx_work, ethernet_rx_work_handler);

	k_delayed_work_submit(&ethernet_rx_work, K_NO_WAIT);
}

/////////////////////////////// MESH INIT ///////////////////////////////

static struct bt_conn *default_conn;

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk(" - Mesh initialized -\n");
}

void main(void)
{
	int err;
	default_conn = NULL;

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	printk("- Settings sample for PCA20036 -\n");
	printk("- DFU Version: %d -\n", DFU_APP_VERSION);

	//printk("***Some change***\n");

	err = hp_led_init();

	if (err) {
		printk("Error initializing HP LED\n");
		return;
	}

	err = pca20036_ethernet_init();

	if (err) {
		printk("Error initializing buttons\n");
		return;
	}

	ethernet_rx_work_init_start();

	printk("- Initiated -\n");

	/* DHCP may not be leased yet - check flag */

	err = latency_init_test();
	if (err) {
		printk("Error initializing latency test\n");
		return;
	}
}
