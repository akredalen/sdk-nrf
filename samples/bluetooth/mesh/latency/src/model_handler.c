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

#include <shell/shell.h>
#include <shell/shell_uart.h>

#include "wizchip_conf.h"

#include "settings.h"
#include "settings_srv.h"
#include "settings_cli.h"
#include "latency_test.h"
#include "model_handler.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(test);

static const struct shell *test_shell;

static const struct bt_mesh_settings_srv_handlers settings_handlers = {
	// .get =
	// .set = 

	// no work...
};

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status);

static struct bt_mesh_settings_srv srv = BT_MESH_SETTINGS_SRV_INIT(&settings_handlers);
static struct bt_mesh_settings_cli cli = BT_MESH_SETTINGS_CLI_INIT(&status_handler);

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status){

				   // no work...
			   }

////////////////////////////////////////////////////////////////////////////
///////////////////////// HEALTH SERVER SETUP //////////////////////////////
////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////
///////////////////////// MODEL SETUP //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static struct bt_mesh_cfg_cli cfg_cli = {
};

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1,
		    BT_MESH_MODEL_LIST(
				 BT_MESH_MODEL_CFG_SRV,
				 BT_MESH_MODEL_CFG_CLI (&cfg_cli),
				 BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
			BT_MESH_MODEL_LIST(BT_MESH_MODEL_SETTINGS_SRV(&srv), 
			BT_MESH_MODEL_SETTINGS_CLI(&cli))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	k_delayed_work_init(&attention_blink_work, attention_blink);

	test_shell = shell_backend_uart_get_ptr();
	shell_print(test_shell, ">>> Bluetooth Mesh Tester sample <<<");

	return &comp;
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// DNU; RUN TEST SHELL ////////////////////////
////////////////////////////////////////////////////////////////////////////

/*COMMANDS:

test run latency
test run scalability
....*/

static int cmd_run_latency(const struct shell *shell, size_t argc, char *argv[]){
	
	int err = 0;
	err = latency_test_init();
	
	if(err){
		printk("ERROR: Failed to initialize Latency Test \n");}
	else{
		printk("Latency test initialized \n");}

	err = latency_test_run();
	
	dk_set_led(1, true);

	return err;
}

static int cmd_run_scalability(const struct shell *shell, size_t argc, char *argv[]){
	
	int err = 0; 
	// err = scalability_test();
	dk_set_led(1, false);

	return err;
}

/* Run (test) commands */
SHELL_STATIC_SUBCMD_SET_CREATE(run_cmds,
	SHELL_CMD_ARG(latency, NULL,
		      "Starting Latency Test...",
		      cmd_run_latency, 1, 0),
	SHELL_CMD_ARG(scalability, NULL,
		      "Starting Scalability Test...",
		      cmd_run_scalability, 1, 0),
	SHELL_SUBCMD_SET_END
);

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DNU; CONFIG TEST SHELL ////////////////////
////////////////////////////////////////////////////////////////////////////

/*COMMANDS:

test cfg app_key
test cfg bind
....*/

static int cmd_app_key(const struct shell *shell, size_t argc, char *argv[])
{
    uint16_t addr;
	uint16_t app_key;
	int err;

	addr = strtol(argv[1], NULL, 0);
	app_key = addr; // only for testing! Remove!
	
	shell_print(shell, "Sending app key config message...", argv[1]);
	
	// struct bt_mesh_test_config_msg *msg;
	// struct bt_mesh_settings_srv *srv;

	// msg->cmd = APP_KEY;
	// srv->model
	// msg->app_key = 

	
	//err = bt_mesh_cfg_app_key_add(NULL, addr, NULL, 1, app_key, NULL);

	if (err) {
		//LOG_WRN("Failed to send config message: %d", err);
	}
 
    return 0;
}


static int cmd_bind_app(const struct shell *shell, size_t argc,
			    char *argv[])
{
	int err = 0;

	//err = bt_mesh_cfg_mod_app_bind(ADD ARGUMENTS);

	return err;
}

/* Configuration commands */
SHELL_STATIC_SUBCMD_SET_CREATE(cfg_cmds,
	SHELL_CMD_ARG(app_key, NULL,
		      "App key ...",
		      cmd_app_key, 1, 0),
	SHELL_CMD_ARG(bind, NULL,
		      "Bind ...",
		      cmd_bind_app, 1, 0),
	
	SHELL_SUBCMD_SET_END
);
///////////////////////////////////////////////////////////////////////////

static int cmd_test(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		shell_help(shell);
		/* shell returns 1 when help is printed */
		return 1;
	}

	shell_error(shell, "%s unknown parameter: %s", argv[0], argv[1]);

	return -EINVAL;
}

SHELL_STATIC_SUBCMD_SET_CREATE(test_cmds,

	SHELL_CMD(run, &run_cmds, "Run test commands", cmd_test),
	SHELL_CMD(cfg, &cfg_cmds, "Run test commands", cmd_test),
	
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_ARG_REGISTER(test, &test_cmds, "Bluetooth Mesh Test commands",
               cmd_test, 1, 1);
