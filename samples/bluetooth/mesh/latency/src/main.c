/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <bluetooth/mesh/cfg_cli.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <dk_buttons_and_leds.h>

#include <settings.h>
#include <settings_srv.h>
#include <settings_cli.h>
#include <wizchip_conf.h>
#include <w5500.h>
#include <logging/log.h>
#include <shell/shell.h>
#include <shell/shell_uart.h>

// LOG_MODULE_REGISTER(test, CONFIG_LOG_DEFAULT_LEVEL);
LOG_MODULE_DECLARE(test);

static const struct shell *test_shell;

#include "pca20036_ethernet.h"
#include "ethernet_command_system.h"
#include "ethernet_dfu.h"
#include "hp_led.h"
#include "latency_test.h"
#include "ethernet_utils.h"

static uint16_t own_addr = 0x0000;
static uint8_t own_mac[6];
static enum Role role; 

static const uint16_t net_idx = BT_MESH_NET_PRIMARY;
static const uint16_t app_idx = 0;
static uint8_t dev_key[16];
static const uint8_t net_key[16] ={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; 
// static uint8_t app_key[16] = {2};
static const uint8_t dev_uuid[16] = { 0xdd, 0xdd };

////////////////////////////////////////////////////////////////////////
///////////////////////// TEST COMMANDS ////////////////////////////////
////////////////////////////////////////////////////////////////////////

/** COMMANDS;
 * test latency
*/
 
static int cmd_test(const struct shell *shell, size_t argc, char *argv[])
{
    shell_print(shell, "Hello world!", argv[1]);
	latency_test_run();
 
    return 0;
}
 
SHELL_STATIC_SUBCMD_SET_CREATE(test_cmds,
    SHELL_CMD_ARG(latency, NULL, "Print Test", cmd_test, 1, 0), // TEST
    SHELL_SUBCMD_SET_END
);
 
SHELL_CMD_ARG_REGISTER(test, &test_cmds, "Bluetooth Mesh Test commands",
               cmd_test, 1, 1);

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// TOOLS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/* Returns true if the provided address is unicast address.*/
static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}

/* Sets the address by combining the two rightmost 8-bits of the MAC address,
and left-shifting by two bits*/
static int define_unicast_addr(uint16_t *addr, uint8_t own_mac[6]){

    /* Showing example for MAC {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43} */

    uint8_t msb = own_mac[4];                   // MSB = 0x35 = 0011 0101
    uint8_t lsb = own_mac[5];                   // LSB = 0x43 = 0100 0011

    uint16_t ph;
    
    ph = ((uint16_t)msb << 8) | lsb;            // addr = 0x3543 = 0011 0101 0100 0011
    ph = ph << 2;                               // addr = 0xD50C = 1101 0101 0000 1100
    ph = ph & 0x7FFF;                           // addr = 0x550C = 0101 0101 0000 1100

    *addr = ph; 

    if(address_is_unicast(own_addr)){
        return 0;
    }
    else{
        return 1;
    }             
}

/* Sets the device key by filling the left-most octets with the node MAC address */
static int define_own_dev_key(){

    uint8_t ph[16] = {0};
    for (int i = 0; i < NELEMS(own_mac); i++){

        ph[i] = own_mac[i];
    }
    
    memcpy(dev_key, ph, 16);

    return 0;
}

static int fetch_own_mac(){
    static wiz_NetInfo gWIZNETINFO;
    int8_t err8;

    err8 = ctlnetwork(CN_GET_NETINFO, (void *)&gWIZNETINFO);
    if (err8 == -1) {
        return 1;
    }

    memcpy(own_mac, gWIZNETINFO.mac, 6);

    return 0;

}

static void setup_cdb(void)
{
	struct bt_mesh_cdb_app_key *key;

	key = bt_mesh_cdb_app_key_alloc(net_idx, app_idx);
	if (key == NULL) {
		printk("Failed to allocate app-key 0x%04x\n", app_idx);
		return;
	}

	bt_rand(key->keys[0].app_key, 16);

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		bt_mesh_cdb_app_key_store(key);
	}
}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// ETHERNET RX WORK ///////////////////////////
////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////
///////////////////////// MODEL SETUP //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static struct bt_mesh_health_srv health_srv = {
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static void settings_set(struct bt_mesh_settings_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_settings_set *set,
			  struct bt_mesh_settings_status *rsp){};

static void settings_get(struct bt_mesh_settings_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_settings_status *rsp){};

static const struct bt_mesh_settings_srv_handlers settings_handlers = {
	.get = settings_get,
	.set = settings_set,
};

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status){};

static struct bt_mesh_settings_srv settings_srv = BT_MESH_SETTINGS_SRV_INIT(&settings_handlers);
static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_CLI_INIT(&status_handler);


static struct bt_mesh_cfg_cli cfg_cli = {};

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1,
		    BT_MESH_MODEL_LIST(
				 BT_MESH_MODEL_CFG_SRV,
				 BT_MESH_MODEL_CFG_CLI (&cfg_cli),
				 BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
			BT_MESH_MODEL_LIST(
				BT_MESH_MODEL_SETTINGS_CLI(&settings_cli),
				BT_MESH_MODEL_SETTINGS_SRV(&settings_srv))),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_MESH_NORDIC_SEMI_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	// .unprovisioned_beacon = unprovisioned_beacon,
	// .node_added = node_added,
};

////////////////////////////////////////////////////////////////////////////
///////////////////////// NODE CONFIGURATION ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void configure_self(struct bt_mesh_cdb_node *self){

    struct bt_mesh_cdb_app_key *key;
	int err;

	uint8_t status;

	printk("Configuring self...\n");

	key = bt_mesh_cdb_app_key_get(app_idx);
	if (key == NULL) {
		printk("No app-key 0x%04x\n", app_idx);
		return;
	}

    err = bt_mesh_cfg_app_key_add(self->net_idx, self->addr, self->net_idx,
				      app_idx, key->keys[0].app_key, &status);
    if (err < 0) {
            printk("Failed to add application key: %d, %u\n", err, status);
            }

    err = bt_mesh_cfg_mod_app_bind_vnd(self->net_idx, self->addr, self->addr,
				       app_idx, BT_MESH_MODEL_ID_SETTINGS_SRV, BT_MESH_NORDIC_SEMI_COMPANY_ID, &status);
            if (err < 0) {
                printk("Failed to bind (settings server) to application\n");
            }

    /* Tester node will also be using the settings client model */
    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
		role = TESTER_N;
		printk("Role: 	TESTER NODE\n");
		err = bt_mesh_cfg_mod_app_bind_vnd(self->net_idx, self->addr, self->addr,
				       app_idx, BT_MESH_MODEL_ID_SETTINGS_CLI, BT_MESH_NORDIC_SEMI_COMPANY_ID, &status);
        if (err < 0) {
            printk("Failed to bind (settings client) application\n");
        }
	}else{
		role = FIELD_N;
		printk("Role: 	FIELD NODE\n");
	}


	atomic_set_bit(self->flags, BT_MESH_CDB_NODE_CONFIGURED);

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		bt_mesh_cdb_node_store(self);
	}

	printk("Configuration complete\n");

}

static uint8_t check_unconfigured(struct bt_mesh_cdb_node *node, void *data)
{
	if (!atomic_test_bit(node->flags, BT_MESH_CDB_NODE_CONFIGURED)) {
		if (node->addr == own_addr) {
			printk("Unicast address: %u\n", node->addr);
			configure_self(node);
		}
	}

	return BT_MESH_CDB_ITER_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MESH INIT //////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	printk("Mesh initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* Create and initialize the Mesh Configuration Database. A primary subnet, 
	ie one with NetIdx 0, will be added and the provided key will be used as
	NetKey for that subnet. */
	err = bt_mesh_cdb_create(net_key);
	if (err == -EALREADY) {
		printk("Using stored CDB\n");
	} else if (err) {
		printk("Failed to create CDB (err %d)\n", err);
	} else {
		printk("Created CDB\n");
		setup_cdb();
	}

	err = fetch_own_mac();
    if (err){
        printk("Error printing network info\n");
    }
    else{
        printf("\nMAC address: %u:%u:%u:%u:%u:%u\n", \
        own_mac[0], own_mac[1], own_mac[2], own_mac[3], own_mac[4], own_mac[5]);
    }

    err = define_unicast_addr(&own_addr, own_mac);
    if (err){
        printk("Failed to set Unicast Address\n");
    }
    printk("Unicast address: %u\n", own_addr);

    err = define_own_dev_key();
    if (err){
        printk("Failed to set Device Key\n");
    }

	if (bt_mesh_is_provisioned()){
        printk("Node is already provisioned!\n ");
    }
    else{
        printk("Provisioning device...\n ");
        err = bt_mesh_provision(net_key, net_idx, 0, 0, own_addr, dev_key);
        if (err == -EALREADY) {
		    printk("Using stored settings\n");
	    } else if (err) {
		    printk("Provisioning failed (err %d)\n", err);
	    } else {
		    printk("Provisioning completed\n");
	    }
    }

	// /* This will be a no-op if settings_load() loaded provisioning info */
	// bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	bt_mesh_cdb_node_foreach(check_unconfigured, NULL);
	// static struct bt_mesh_cdb_node node;
	// check_unconfigured(&node, NULL);

	return;

}

////////////////////////////////////////////////////////////////////////////
///////////////////////// LATENCY TEST /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*
OUTBOUND messages are those that are sent from the TEST node, to the FIELD node.
INBOUND messages are those that are sent from the FIELD node, back to the TEST node
RTT is the calculated Round-Trip-Time
*/
static int64_t out_time; 
static int64_t in_time;
static int64_t rtt;
static int lost_msg_count = 0;

static uint16_t target_addr = 0x0000;

static uint8_t target_mac[6];
static int target_ttl;

int latency_test_run(){

    if(role == TESTER_N){
		printk("STARTING LATENCY TEST");
	}else{
		printk("ERROR: can't run latency test from field node!\n");
		return 1;
	}

	int err = 0;

	struct bt_mesh_settings_status status;
	struct bt_mesh_msg_ctx ctx;

    static struct bt_mesh_cfg_mod_pub pub = {
        .addr = 0x0000,
        .app_idx = app_idx,
        .cred_flag = false,
        .ttl = 0,
        .period = BT_MESH_PUB_PERIOD_SEC(2),
        .transmit = 3, 
    };

	for (int i = 0; i < NODES_TOTAL; i++){
	
		memcpy(target_mac, node_data_mac_ttl[i].mac_address, sizeof(target_mac));
		target_ttl = node_data_mac_ttl[i].ttl;
		
		define_unicast_addr(&target_addr, target_mac);
		
		ctx.addr = target_addr;

        pub.addr = target_addr;
        pub.ttl = target_ttl;  

        err = bt_mesh_cfg_mod_pub_set_vnd(net_idx, own_addr, 0,
         BT_MESH_MODEL_ID_SETTINGS_CLI, BT_MESH_NORDIC_SEMI_COMPANY_ID,
         &pub, NULL);
         
        if (err) {
			printk("Error: unable to set publication values\n");
			return err;
		}

		/* Send messages to node address */
		for (int j = 0; j < MSG_AMOUNT; j++) {

			out_time = k_uptime_get();
			err = bt_mesh_settings_cli_get(&settings_cli, &ctx, &status);

			/* Blocking while waiting for a response... */
			if (err < 0){
                lost_msg_count++;
				printk("ERROR: latency message nr. %d failed.\nTotal lost messages: %d \n", j, lost_msg_count);
			}
			else{
				/* Response is received. Record new time-stamp */
				in_time = k_uptime_get();
			}

			rtt = in_time - out_time;
			printk("Round-trip time: %lld \n", rtt);

			// DO: send response over ethernet...
		} 
	} 
	return err;
}


////////////////////////////////////////////////////////////////////////////
///////////////////////// MAIN /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void main(void)
{
	int err;

	test_shell = shell_backend_uart_get_ptr();
	shell_print(test_shell, ">>> Bluetooth Mesh TEST sample <<<");

	printk("- Latency Test for PCA20036 -\n");
	printk("- DFU Version: %d -\n", DFU_APP_VERSION);

	err = hp_led_init();

	if (err) {
		printk("Error initializing HP LED\n");
	}

	err = pca20036_ethernet_init();

	if (err) {
		printk("Error initializing buttons\n");
	}

	ethernet_rx_work_init_start();

	printk("- Initiated -\n");

	/* DHCP may not be leased yet - check flag */

	printk("Initializing mesh...\n");

	/* Initialize BT Mesh, provision and configure local device */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	bt_ready(0);
	
}
