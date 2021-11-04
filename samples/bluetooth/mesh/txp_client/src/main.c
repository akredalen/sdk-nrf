/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
/**
 * @file
 * @brief TXP Client sample.
 *
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include <bluetooth/mesh/cfg_cli.h>
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <sys/util.h>

#define LOG_MODULE_NAME txp_cli
#include "common/log.h"

static const struct shell *shell;

static uint8_t own_mac[6] = { 0 };
static uint16_t own_addr = 0x0000;
static const uint16_t net_idx = BT_MESH_NET_PRIMARY;
static const uint16_t app_idx = 0;
static uint8_t dev_key[16] = { 0 };
static const uint8_t net_key[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
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

static int increment(int *txp_index)
{
	if (*txp_index == ARRAY_SIZE(txp_values) - 1) {
		return 1;
	} else {
		(*txp_index)++;
	}
	return 0;
}

static int decrement(int *txp_index)
{
	if (*txp_index == 0) {
		return 1;
	} else {
		(*txp_index)--;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// MODEL SETUP ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void status_handler(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_txp_status *status);

static struct bt_mesh_txp_cli txp_cli = BT_MESH_TXP_CLI_INIT(&status_handler);
static struct bt_mesh_cfg_cli cfg_cli = {};

static struct bt_mesh_health_srv health_srv = {};
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0,
		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_CFG_SRV, BT_MESH_MODEL_CFG_CLI(&cfg_cli),
					BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
		     BT_MESH_MODEL_LIST(BT_MESH_MODEL_TXP_CLI(&txp_cli))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static int bt_mesh_txp_cli_init(struct bt_mesh_model *model)
{
	struct bt_mesh_txp_cli *cli = model->user_data;

	cli->model = model;
	net_buf_simple_init(cli->pub.msg, 0);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_txp_cli_cb = {
	.init = bt_mesh_txp_cli_init,
};

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
		BT_ERR("No app-key 0x%04x", app_idx);
		err_count++;
	}

	BT_HEXDUMP_DBG(key->keys[0].app_key, 16, "AppKey");

	err = bt_mesh_cfg_app_key_add(net_idx, own_addr, net_idx, app_idx, key->keys[0].app_key,
				      &status);
	if (err < 0) {
		BT_WARN("Failed to add application key: %d, %u", err, status);
		err_count++;
	}

	/** Bind and store client model to application */
	err = bt_mesh_cfg_mod_app_bind_vnd(net_idx, own_addr, own_addr, app_idx,
					   BT_MESH_MODEL_ID_TXP_CLI, BT_MESH_NORDIC_SEMI_COMPANY_ID,
					   &status);
	if (err < 0) {
		BT_ERR("Failed to bind TXP_CLI model to application");
		err_count++;
	}

	/** Set and store client publication parameters */
	struct bt_mesh_cfg_mod_pub cli_pub = {
		.addr = TXP_MODEL_CH_ADDR,
		.ttl = 0,
	};

	err = bt_mesh_cfg_mod_pub_set_vnd(net_idx, own_addr, own_addr, BT_MESH_MODEL_ID_TXP_CLI,
					  BT_MESH_NORDIC_SEMI_COMPANY_ID, &cli_pub, &status);
	if (err < 0) {
		BT_ERR("Failed to set publication to group channel");
		err_count++;
	}

	if (err_count == 0) {
		atomic_set_bit(self->flags, BT_MESH_CDB_NODE_CONFIGURED);
		bt_mesh_cdb_node_store(self);
		printk("\n--- Configuration complete ---\n");
	} else {
		BT_ERR("Configuration was not completed correctly!");
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

////////////////////////////////////////////////////////////////////////////
////////////////////// MODEL CALLBACKS AND FUNCTIONALITY ///////////////////
////////////////////////////////////////////////////////////////////////////

static void status_handler(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_txp_status *status)
{
	if (status->status == true) {
		printk("<0x%04x> Current TX Power is %d dB\n", ctx->addr, status->present_txp);

		/** Update index for continuous button functionality  */
		for (int i = 0; status->present_txp >= txp_values[i] && i < sizeof(txp_values); i++)
		{
			txp_index = i;
		}
	} else {
		printk("Operation failed.\n");
	}
}

static void txp_cli_send_msg_pub(int8_t txp_val)
{
	int err;
	set_msg.txp_value = txp_val;
	err = bt_mesh_txp_cli_set_ack(&txp_cli, NULL, &set_msg);

	if (err) {
		printk("Settings SET message failed: %d\n", err);
	}
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

		/** Button 1 (Decrease TXP level on server) */
		if (i == 0) {
			err = decrement(&txp_index);
			if (err) {
				printk("TX Power is currently at the lowest level! \n");
				return;
			}
			printk("\nDecreasing TX Power to %d \n", txp_values[txp_index]);
		}

		/** Button 2 (Increase TXP level on server) */
		else if (i == 1) {
			err = increment(&txp_index);
			if (err) {
				printk("TX Power is currently on the highest level! \n");
				return;
			}
			printk("\nIncreasing TX Power to %d \n", txp_values[txp_index]);
		}

		/** Button 3 (Get TX Power from server) */
		else if (i == 2) {
			err = bt_mesh_txp_cli_get(&txp_cli, NULL);
			return;
		}

		else {
			return;
		}

		txp_cli_send_msg_pub(txp_values[txp_index]);
	}
}

////////////////////////////////////////////////////////////////////////
///////////////////////// UART COMMANDS ////////////////////////////////
////////////////////////////////////////////////////////////////////////

static int cmd_help(const struct shell *shell, size_t argc, char *argv[])
{
	shell_print(shell, "COMMANDS: \n	\
 * Transmit Power (TXP) client. Servers will use their LEDs to indicate current TXP level. All LEDs active indicates MAX level of 4 dB.  \
 \n	\
 * Set new TX power value for subscribing nodes: \t\t >> txp set <value in range [%d - %d]> \
 \n	\
 \t * Alternatively: Use the buttons to decrease (Button 1) or increase (Button 2) the TXP level \
 \n	\
 * Get TX power status update from subscribing nodes: \t\t >> txp get \
 \n	\
 \t * Alternatively: Use Button 3 to get status from the nodes \
 \n	\
 * Get network info: \t \t	>> txp net \n",
		    txp_values[0], txp_values[sizeof(txp_values) - 1]);

	return 0;
}

/** Publish a SET message containing the TXP value */
static int cmd_set_txp_pub(const struct shell *shell, size_t argc, char *argv[])
{
	int8_t val = strtol(argv[1], NULL, 0);
	if (val > txp_values[sizeof(txp_values) - 1] || val < txp_values[0]) {
		shell_print(shell, "Invalid TXP value. Must be in range [%d - %d]", txp_values[0],
			    txp_values[sizeof(txp_values) - 1]);
	} else {
		txp_cli_send_msg_pub(val);
	}

	return 0;
}

static int cmd_get_txp(const struct shell *shell, size_t argc, char *argv[])
{
	shell_print(shell, "Getting current TXP status from network...", argv[1]);
	int err;
	err = bt_mesh_txp_cli_get(&txp_cli, NULL);
	return 0;
}

static int cmd_net(const struct shell *shell, size_t argc, char *argv[])
{
	shell_print(shell, "--- NODE INFO: ---", argv[1]);
	get_own_mac(own_mac);
	shell_print(shell, "LOCAL MAC: \t \t %02X:%02X:%02X:%02X:%02X:%02X", own_mac[0], own_mac[1],
		    own_mac[2], own_mac[3], own_mac[4], own_mac[5]);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(txp_cmds, SHELL_CMD_ARG(help, NULL, "", cmd_help, 1, 0),
			       SHELL_CMD_ARG(net, NULL, "Fetch network info", cmd_net, 1, 0),
			       SHELL_CMD_ARG(get, NULL, "Fetch network info", cmd_get_txp, 1, 0),
			       SHELL_CMD_ARG(set, NULL, "Configure segmentation", cmd_set_txp_pub,
					     2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_ARG_REGISTER(txp, &txp_cmds,
		       "Bluetooth Mesh Test commands. Run <test help> for more info.", NULL, 1, 1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////// MESH INIT ///////////////////////////////
/////////////////////////////////////////////////////////////////////////

const struct bt_mesh_comp *model_handler_init(void)
{
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};

	dk_button_handler_add(&button_handler);

	return &comp;
}

void bt_ready(int err)
{
	if (err) {
		BT_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	printk("\n--- Bluetooth initialized ---\n");

	err = bt_mesh_init(&prov, model_handler_init());
	if (err) {
		BT_ERR("Initializing mesh failed (err %d)", err);
		return;
	}

	printk("\n--- Mesh initialized ---\n");

	dk_leds_init();
	dk_buttons_init(NULL);

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
	BT_DBG("MAC address: %02X:%02X:%02X:%02X:%02X:%02X", own_mac[0], own_mac[1], own_mac[2],
	       own_mac[3], own_mac[4], own_mac[5]);

	err = unicast_addr_get(&own_addr, own_mac);
	if (err) {
		BT_WARN("Failed to set Unicast Address");
	} else {
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
		err = bt_mesh_provision(net_key, net_idx, 0, 0, own_addr, dev_key);
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

	shell = shell_backend_uart_get_ptr();

	printk("Initializing Bluetooth...\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	bt_ready(0);

	printk("Type <txp help> for info \n");
}