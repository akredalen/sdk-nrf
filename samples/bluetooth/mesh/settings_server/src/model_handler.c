/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/**
 * @file
 * @brief Model handler for the TX POWER CONTROL Server.
 *
 * Instantiates a Settings Client model for each button on the devkit, as
 * well as the standard Config and Health Server models. Handles all application
 * behavior related to the models.
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>

#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/hrs.h>

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>

#include "model_handler.h"
#include "settings_srv.h"

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static struct bt_conn *default_conn;


/** TX POWER Functions. Source: zephyr/samples/bluetooth/hci_pwr_ctrl/src/main.c*/
void hci_set_tx_power(uint8_t handle_type, uint16_t handle, uint8_t txp_lvl)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	// Allocate a HCI command buffer
	buf = bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL,
				sizeof(*cp));
	if (!buf) {
		printk("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;
	cp->tx_power_level = txp_lvl;

	//Send a HCI command synchronously
	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL,
				   buf, &rsp);
	if (err) {
		uint8_t reason = rsp ?
			((struct bt_hci_rp_vs_write_tx_power_level *)
			  rsp->data)->status : 0;
		printk("Set Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}
	rp = (void *)rsp->data;

	net_buf_unref(rsp);
}

static void hci_get_tx_power(uint8_t handle_type, uint16_t handle, uint8_t *txp_lvl)
{
	struct bt_hci_cp_vs_read_tx_power_level *cp;
	struct bt_hci_rp_vs_read_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	*txp_lvl = 0xFF;
	buf = bt_hci_cmd_create(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
				sizeof(*cp));
	if (!buf) {
		printk("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL,
				   buf, &rsp);
	if (err) {
		uint8_t reason = rsp ?
			((struct bt_hci_rp_vs_read_tx_power_level *)
			  rsp->data)->status : 0;
		printk("Read Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*txp_lvl = rp->tx_power_level;

	net_buf_unref(rsp);
}



static void txp_set(struct bt_mesh_settings_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_settings_set *set,
		    struct bt_mesh_settings_status *rsp)
{
	int8_t txp_get = 0xFF;
	
	if (!default_conn) {

		printk("\n");
		printk("Getting the TX Power level....\n");
		hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,
				     0, &txp_get);
		printk("Current TX Power is %d", txp_get);
		printk(" dB\n\n");

		k_sleep(K_SECONDS(2));

		printk("Setting TX Power level to %d", set->txp_value);
		printk(" dB...\n");
		hci_set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,
				     0, set->txp_value);

		k_sleep(K_SECONDS(2));

		hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,
				     0, &txp_get);
		printk("New TX Power level is set to %d", txp_get); 
		printk(" dB\n\n");

		if (set->txp_value == txp_get){
			rsp->status = true;
		}
		else{
			rsp->status = false;
		}
		rsp->present_txp = txp_get;
	}
}

static void txp_get(struct bt_mesh_settings_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_settings_status *rsp)
{	
	int8_t txp_get = 0xFF;

	if (!default_conn) {

		printk("\n");
		printk("Getting the TX Power level....\n");
		hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV,
				     0, &txp_get);
		printk("Current TX Power is %d", txp_get);
		printk(" dB\n\n");
	}
	rsp->present_txp = txp_get;
}

static const struct bt_mesh_settings_srv_handlers settings_handlers = {
	.set = txp_set,
	.get = txp_get,
};

static struct bt_mesh_settings_srv settings_srv = BT_MESH_SETTINGS_SRV_INIT(&settings_handlers);

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
			BT_MESH_MODEL_LIST(BT_MESH_MODEL_SETTINGS_SRV(&settings_srv))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	k_delayed_work_init(&attention_blink_work, attention_blink);

	//bt_mesh_net_transmit_set(BT_MESH_TRANSMIT(2, 20));

	return &comp;
}