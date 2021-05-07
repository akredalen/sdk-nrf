
/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/**
 * @file
 * @brief Model handler for the Settings Client.
 *
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>

#include "settings_cli.h"

#define DEVICE_TXPOWER_NUM  7

static const int8_t txp[DEVICE_TXPOWER_NUM] = {-40, -20, -16, -8, -4, 0, 4};

/** TX Power is initiallized at 0 dB */
int txp_index = DEVICE_TXPOWER_NUM - 2;

static void status_handler(struct bt_mesh_settings_cli *cli,
 			   struct bt_mesh_msg_ctx *ctx,
 			   const struct bt_mesh_settings_status *status);

static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_CLI_INIT(&status_handler);

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status)
{
	if (status->status == true){
	printk("Operation was successful. Current TX Power is %d dB\n",
	       status->present_txp);
	}
	else{
		printk("Set operation failed.\n");
		}
}

static int increment(int *txp_index){
	if (*txp_index == DEVICE_TXPOWER_NUM-1){
		return 1;
		}
	else{
		int p;
		p = *txp_index;
		p++;
		*txp_index = p;
		}
	return 0;
}

static int decrement(int *txp_index){
	if (*txp_index == 0){
		return 1;
		}
	else{
		int p;
		p = *txp_index;
		p--;
		*txp_index = p;
		}
	return 0;
}

static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
	if (!bt_mesh_is_provisioned()) {
		return;
	}

	int err;
	for (int i = 0; i < 4; ++i) { 
		if (!(pressed & changed & BIT(i))) {
			continue;
		}

		/** Button 1 (Increase TXP level on server) */
		if (i == 0){
			err = increment(&txp_index);
			if (err){
				printk("TX Power is currently on the highest level! \n");
				return;
			}
			printk("\nIncreasing TX Power to %d \n", txp[txp_index]);
		}

		/** Button 2 (Decrease TXP level on server) */
		else if (i == 1){
			err = decrement(&txp_index);
			if (err){
				printk("TX Power is currently on the lowest level! \n");
			return;
			}
			printk("\nDecreasing TX Power to %d \n", txp[txp_index]);
		}

		/** Button 3 (Get TX Power from server) */
		else if (i == 2){
			err = bt_mesh_settings_cli_get(&settings_cli, NULL, NULL);
			return;
		}

		else{
			return;
		}


		struct bt_mesh_settings_set set = {
		 	.txp_value = txp[txp_index],
		 };
		
		err = bt_mesh_settings_cli_set(&settings_cli,
							  NULL, &set, NULL);
	
		if (err) {
			printk("Settings %d set failed: %d\n", i + 1, err);
		}
	}
}

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */
static struct k_delayed_work attention_blink_work;

static void attention_blink(struct k_work *work)
{
	static int idx;
	const uint8_t pattern[] = {
		BIT(0) | BIT(1),
		BIT(1) | BIT(2),
		BIT(2) | BIT(3),
		BIT(3) | BIT(0),
	};

	dk_set_leds(pattern[idx++ % ARRAY_SIZE(pattern)]);
	k_delayed_work_submit(&attention_blink_work, K_MSEC(30));
}

static void attention_on(struct bt_mesh_model *mod)
{
	k_delayed_work_submit(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(struct bt_mesh_model *mod)
{
	k_delayed_work_cancel(&attention_blink_work);
	dk_set_leds(DK_NO_LEDS_MSK);
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1,
		    BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_CFG_SRV,
			     BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
			BT_MESH_MODEL_LIST(BT_MESH_MODEL_SETTINGS_CLI(&settings_cli))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};

	dk_button_handler_add(&button_handler);
	k_delayed_work_init(&attention_blink_work, attention_blink);

	return &comp;
}
