/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/**
 * @file
 * @brief TXP Server sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
#include <bluetooth/mesh/cfg_cli.h>

#include <txp_settings_srv.h>
#include <txp_settings.h>

#define LOG_MODULE_NAME txp_srv
#include "common/log.h"

static struct bt_conn *default_conn;

static uint8_t own_mac[6] = { 0 };
static uint16_t own_addr = 0x0000;
static const uint16_t net_idx = BT_MESH_NET_PRIMARY;
static const uint16_t app_idx = 0;
static uint8_t dev_key[16] = {0};
static const uint8_t net_key[16] = { 1, 2,  3,	4,  5,	6,  7,	8,
				     9, 10, 11, 12, 13, 14, 15, 16 };

static const uint8_t dev_uuid[16] = { 0xdd, 0xdd };
static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
};

static const int8_t txp_values[] = { -40, -20, -16, -8, -4, 0, 4 };

/** TX Power is initiallized at 0 dB */
int txp_index = ARRAY_SIZE(txp_values) - 2;

struct bt_mesh_txp_set set_msg = {
	.txp_value = 0,
};

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// TOOLS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/* Returns true if the provided address is unicast address.*/
static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}

/* Sets the address by combining the two rightmost 8-bits of the MAC address,
and left-shifting by two bits */
static int unicast_addr_get(uint16_t *addr, uint8_t mac[6])
{
	uint8_t msb = mac[4];
	uint8_t lsb = mac[5];

	uint16_t ph = 0x7FFF & ((((uint16_t)msb << 8) | lsb) << 2);

	*addr = ph;

	if (address_is_unicast(ph)) {
		return 0;
	} else {
		return -1;
	}
}

/* Sets the device key by filling the left-most octets with the node MAC address */
static int own_dev_key_compute()
{
	uint8_t ph[16] = { 0 };
	for (int i = 0; i < ARRAY_SIZE(own_mac); i++) {
		ph[i] = own_mac[i];
	}

	memcpy(dev_key, ph, 16);

	return 0;
}

static void get_own_mac(uint8_t *p_mac)
{
	p_mac[0] = (0xB0) & 0xFF;
	p_mac[1] = (NRF_FICR->DEVICEADDR[0] >> 8) & 0xFF;
	p_mac[2] = (NRF_FICR->DEVICEADDR[0] >> 16) & 0xFF;
	p_mac[3] = (NRF_FICR->DEVICEADDR[0] >> 24);
	p_mac[4] = (NRF_FICR->DEVICEADDR[1]) & 0xFF;
	p_mac[5] = (NRF_FICR->DEVICEADDR[1] >> 8) & 0xFF;
}

static void setup_cdb(void)
{
	struct bt_mesh_cdb_app_key *key;

	key = bt_mesh_cdb_app_key_alloc(net_idx, app_idx);
	if (key == NULL) {
		BT_WARN("Failed to allocate app-key 0x%04x", app_idx);
		return;
	}

	memset(key->keys[0].app_key, 0xAA, 16);

	bt_mesh_cdb_app_key_store(key);
}

static void set_led_indicator(int8_t val){
	
	if (val == 4){
		dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);
	}else if (val >= -16 && val < 4){ // [-16, 3)
		dk_set_leds_state((DK_LED1_MSK | DK_LED2_MSK |\
		DK_LED3_MSK), DK_LED4_MSK);
	}else if (val > -40 && val < 16){ // [-39, -17)
		dk_set_leds_state((DK_LED1_MSK | DK_LED2_MSK ), (DK_LED3_MSK | DK_LED4_MSK));
	}else if (val == -40){
		dk_set_leds_state((DK_LED1_MSK), (DK_LED2_MSK | DK_LED3_MSK |\
		DK_LED4_MSK));
	}else{
		printk("set_led_indicator: Invalid TXP value: %d /n", val);
	}
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// MODEL LOGIC ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

/** TX POWER Functions. Source: zephyr/samples/bluetooth/hci_pwr_ctrl/src/main.c*/
static void hci_set_tx_power(uint8_t handle_type, uint16_t handle, int8_t txp_lvl)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, sizeof(*cp));
	if (!buf) {
		printk("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;
	cp->tx_power_level = txp_lvl;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf,
				   &rsp);
	if (err) {
		uint8_t reason =
			rsp ? ((struct bt_hci_rp_vs_write_tx_power_level *)
				       rsp->data)->status : 0;
		printk("Set Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}
	rp = (void *)rsp->data;

	net_buf_unref(rsp);
}

static void hci_get_tx_power(uint8_t handle_type, uint16_t handle,
			     int8_t *txp_lvl)
{
	struct bt_hci_cp_vs_read_tx_power_level *cp;
	struct bt_hci_rp_vs_read_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, sizeof(*cp));
	if (!buf) {
		printk("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->handle_type = handle_type;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_TX_POWER_LEVEL, buf, &rsp);
	if (err) {
		uint8_t reason =
			rsp ? ((struct bt_hci_rp_vs_read_tx_power_level *)
				       rsp->data)->status : 0;
		printk("Read Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}

	rp = (void *)rsp->data;
	*txp_lvl = rp->tx_power_level;

	net_buf_unref(rsp);
}

static void txp_set(struct bt_mesh_txp_srv *srv,
		    struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_txp_set *set,
		    struct bt_mesh_txp_status *rsp)
{
	int8_t txp_get;

	if (!default_conn) {

		printk("Setting TX Power level to %d dB...\n", set->txp_value);
		hci_set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0,
				 set->txp_value);

		hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, &txp_get);

		if (set->txp_value == txp_get) {
			rsp->status = true;
			set_led_indicator(set->txp_value);
			printk("New TX Power level is set to %d dB\n\n", txp_get);

		} else {
			rsp->status = false;
		}
		rsp->present_txp = txp_get;
	}
}

static void txp_get(struct bt_mesh_txp_srv *srv,
		    struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_txp_status *rsp)
{
	int8_t txp_get;

	if (!default_conn) {
		printk("Getting the TX Power level....\n");
		hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, &txp_get);
		printk("Current TX Power is %d dB\n\n", txp_get);
	}
	rsp->present_txp = txp_get;
}

////////////////////////////////////////////////////////////////////////////
///////////////////////// MODEL SETUP //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static const struct bt_mesh_txp_srv_handlers txp_handlers = {
	.set = txp_set,
	.get = txp_get,
};

static struct bt_mesh_txp_srv txp_srv =
	BT_MESH_TXP_SRV_INIT(&txp_handlers);

static struct bt_mesh_cfg_cli cfg_cli = {};

static struct bt_mesh_health_srv health_srv = {};
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(
		0,
		BT_MESH_MODEL_LIST(BT_MESH_MODEL_CFG_SRV, BT_MESH_MODEL_CFG_CLI(&cfg_cli),
				   BT_MESH_MODEL_HEALTH_SRV(&health_srv,
							    &health_pub)),
		BT_MESH_MODEL_LIST(BT_MESH_MODEL_TXP_SRV(&txp_srv))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	
	int8_t txp_current;
	hci_get_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, &txp_current);
	set_led_indicator(txp_current);

	return &comp;
}

///////////////////////////////// CONFIG ////////////////////////////////////

static void configure_self(struct bt_mesh_cdb_node *self)
{
	struct bt_mesh_cdb_app_key *key;
	int err, err_count = 0;

	uint8_t status;
	printk("Configuring local node...\n");

	/** Get and add application key */
	key = bt_mesh_cdb_app_key_get(app_idx);
	if (key == NULL) {
		BT_WARN("No app-key 0x%04x", app_idx);
		return;
	}

	BT_HEXDUMP_DBG(key->keys[0].app_key, 16, "AppKey");

	err = bt_mesh_cfg_app_key_add(net_idx, own_addr, net_idx, app_idx,
				      key->keys[0].app_key, &status);
	if (err < 0) {
		BT_WARN("Failed to add application key: %d, %u", err, status);
		err_count++;
	}

	/** Bind and store server model to application */
	err = bt_mesh_cfg_mod_app_bind_vnd(net_idx, own_addr, own_addr, app_idx,
					   BT_MESH_MODEL_ID_TXP_SRV,
					   BT_MESH_NORDIC_SEMI_COMPANY_ID,
					   &status);
	if (err < 0) {
		BT_ERR("Failed to bind TXP_SRV model to application");
		err_count++;
	}

	err = bt_mesh_cfg_mod_sub_add_vnd(net_idx, own_addr, own_addr, 
							TXP_MODEL_CH_ADDR, 
							BT_MESH_MODEL_ID_TXP_SRV, BT_MESH_NORDIC_SEMI_COMPANY_ID, 
							&status);
	if (err < 0) {
			BT_ERR("Failed to set subscription to group channel");
			err_count++;
		}

	if (err_count == 0) {
		atomic_set_bit(self->flags, BT_MESH_CDB_NODE_CONFIGURED);
		bt_mesh_cdb_node_store(self);
		printk("\n--- Configuration complete ---\n");
	}
}

static uint8_t check_unconfigured(struct bt_mesh_cdb_node *node, void *data)
{
	ARG_UNUSED(data);
	if (node->addr == own_addr) {
		configure_self(node);
	}

	return BT_MESH_CDB_ITER_CONTINUE;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////// MESH INIT ///////////////////////////////
/////////////////////////////////////////////////////////////////////////

void bt_ready(int err)
{
	if (err) {
		BT_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	printk("\n--- Bluetooth initialized ---\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(&prov, model_handler_init());
	if (err) {
		BT_ERR("Initializing mesh failed (err %d)", err);
		return;
	}

	printk("\n--- Mesh initialized ---\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_mesh_cdb_create(net_key);
	if (err == -EALREADY) {
		BT_DBG("Using stored CDB");
	} else if (err) {
		BT_DBG("Failed to create CDB (err %d)", err);
	} else {
		BT_DBG("Created CDB");
		setup_cdb();
	}

	get_own_mac(own_mac);
	BT_DBG("MAC address: %02X:%02X:%02X:%02X:%02X:%02X", own_mac[0],
			own_mac[1], own_mac[2], own_mac[3], own_mac[4],
			own_mac[5]);

	err = unicast_addr_get(&own_addr, own_mac);
	if (err) {
		BT_WARN("Failed to set Unicast Address");
	}else{
		BT_DBG("Unicast address: 0x%04x", own_addr);
	}

	err = own_dev_key_compute();
	if (err) {
		BT_WARN("Failed to set Device Key");
	}

	if (bt_mesh_is_provisioned()) {
		BT_DBG("Node is already provisioned! ");
	} else {
		printk("Provisioning device...\n");
		err = bt_mesh_provision(net_key, net_idx, 0, 0, own_addr,
					dev_key);
		if (err == -EALREADY) {
			BT_DBG("Using stored settings");
		} else if (err) {
			BT_ERR("Provisioning failed (err %d)", err);
		} else {
			printk("\n--- Provisioning completed ---\n");
		}
	}
	bt_mesh_cdb_node_foreach(check_unconfigured, NULL);

	return;
}

void main(void)
{
	int err;

	printk("Initializing Bluetooth...\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	bt_ready(0);
}