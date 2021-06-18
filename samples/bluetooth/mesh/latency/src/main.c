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

#include "pca20036_ethernet.h"
#include "ethernet_command_system.h"
#include "ethernet_dfu.h"
#include "hp_led.h"
#include "latency_test.h"
#include "ethernet_utils.h"

static uint16_t own_addr = 0x0000;
static uint8_t own_mac[6];

static const uint16_t net_idx = BT_MESH_NET_PRIMARY;
static const uint16_t app_idx = 0;
static uint8_t dev_key[16];
static const uint8_t net_key[16] ={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; 
static uint8_t app_key[16] = {2};
static const uint8_t dev_uuid[16] = { 0xdd, 0xdd };

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

static const struct bt_mesh_settings_srv_handlers settings_handlers = {
	// .get =
	// .set = 

	// no work...
};

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status);

static struct bt_mesh_settings_srv settings_srv = BT_MESH_SETTINGS_SRV_INIT(&settings_handlers);
static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_CLI_INIT(&status_handler);

static void status_handler(struct bt_mesh_settings_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_settings_status *status){

				   // no work...
			   }


static struct bt_mesh_cfg_cli cfg_cli = {
};

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
///////////////////////// NODE INITIALIZATION //////////////////////////////
////////////////////////////////////////////////////////////////////////////

int latency_configure_self(){

    struct bt_mesh_cdb_app_key *key;
	int err;

	printk("Configuring self...\n");

	key = bt_mesh_cdb_app_key_get(app_idx);
	if (key == NULL) {
		printk("No app-key 0x%04x\n", app_idx);
		return;
	}

    err = bt_mesh_cfg_app_key_add(net_idx, own_addr, net_idx, app_idx, key->keys[0].app_key, NULL);
    if (err < 0) {
            printk("Failed to add application key\n");
            }

    err = bt_mesh_cfg_mod_app_bind(net_idx, own_addr, 0 , app_idx,
                                BT_MESH_MODEL_ID_SETTINGS_SRV, NULL);
            if (err < 0) {
                printk("Failed to bind application\n");
            }

    /* Tester node will also be using the cfg client model */
    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
        role = TESTER_N;

        err = bt_mesh_cfg_mod_app_bind(net_idx, own_addr, 0, app_idx,
                                BT_MESH_MODEL_ID_SETTINGS_CLI, NULL);
        if (err < 0) {
            printk("Failed to bind application\n");
        }
    }
    else{
        role = FIELD_N;
    }

    return err;
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

	// dk_leds_init();
	// dk_buttons_init(NULL);

	// err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

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
        printf("\n**MAC address**: %u:%u:%u:%u:%u:%u\n", \
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
        err = bt_mesh_provision(net_key, net_idx, 0, 0, 1, dev_key);
        if (err == -EALREADY) {
		    printk("Using stored settings\n");
	    } else if (err) {
		    printk("Provisioning failed (err %d)\n", err);
	    } else {
		    printk("Provisioning completed\n");
	    }
    }

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk(" - Mesh initialized -\n");
	
	err = latency_configure_self();

	if (err) {
		printk("Failed to self configurate\n");
		return;
	}
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

	printk("- Latency Test for PCA20036 -\n");
	printk("- DFU Version: %d -\n", DFU_APP_VERSION);

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

	printk("Initializing mesh...\n");

	/* Initialize BT Mesh, provision and configure local device */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	
}
