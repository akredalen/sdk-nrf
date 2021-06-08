#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "settings.h"
#include "settings_srv.h"
#include "latency_test.h"
#include "pca20036_ethernet.h" // get_own_ utils
#include "ethernet_utils.h" // _are_equal utils
#include "../../../../zephyr/subsys/bluetooth/mesh/prov.h"
#include "../../../../zephyr/include/bluetooth/mesh/cfg_cli.h"

// See example og self provisioning:
// C:\git_repos\ncs\zephyr\samples\bluetooth\mesh_provisioner\src\main.c

static int init_node(enum Role role){ 

    
    int err; 

    static const uint16_t net_idx;
    static const uint16_t app_idx;
    uint8_t dev_key[16], net_key[16], app_key[];

    // DO: set values...
    // bt_rand(net_key, 16);

    uint16_t own_addr;
    static uint8_t own_mac[6];

    /* Define a unicast address and device key based on MAC address */
        get_own_mac(&own_mac);
        own_addr = get_unicast_addr(own_mac);
        dev_key = get_dev_key(own_mac);

    /* Self provision */
        err = bt_mesh_provision(net_key, BT_MESH_NET_PRIMARY, 0, 0, own_addr, dev_key);

    /* Initial configurations */

        err = bt_mesh_cfg_app_key_add(net_idx, own_addr, net_idx, app_idx, app_key[16], NULL);
        if (err) {
                    printk("Failed to add application key");
                }

        err = bt_mesh_cfg_mod_app_bind(net_idx, own_addr, own_addr, app_idx,
                                BT_MESH_MODEL_ID_SETTINGS_SRV, NULL);
                if (err) {
                    printk("Failed to bind application");
                }

    /* Role specific configurations: */
    switch (role) {
        case TESTER_N:

            
            // DO: specify cfg...

            /* Set publication address */
            // err = bt_mesh_cfg_mod_pub_set(net_idx, addr, addr,

            return;

        case FIELD_N:

            // DO: specify cfg...

        return;

    return err;


    }

};

/* Initialize node and set up parameters */
int latency_init_test(){

    int err;
    uint16_t own_addr;
    static enum role;

    get_own_mac(&own_mac);

    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
        role = TESTER_N;
    }
    else{
        role = FIELD_N;
    }

    err = init_node(role);

    return err;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DNU; TESTER NODE ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

static int latency_send_test_msg(struct bt_mesh_settings_srv *srv, uint16_t addr){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr,
            //.send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime
    
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_LATENCY_TEST_OP, BT_MESH_LATENCY_MSG_LEN_TEST);
    bt_mesh_model_msg_init(&buf, BT_MESH_LATENCY_TEST_OP);

        /* Message contains only OP code */
        --

    return bt_mesh_model_send(&srv, &ctx, &buf, NULL, NULL); 
}


static void handle_latency_rsp_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    /* Get the immediate arrival time of the response message */
    int64_t current_uptime = k_uptime_get();

    if (buf->len != BT_MESH_LATENCY_MSG_LEN_RSP) {
			printk("Error: Buffer length out of scope (handle_latency)");
		return;
	}

    struct bt_mesh_settings_srv *srv;
    // struct bt_mesh_settings_srv *srv = model->user_data;
    static enum Test_State test_state = CONT; 

	srv->handlers->latency_rsp(srv, ctx, test_state, current_uptime);

    // return a boolean to use in latency_test()?
    }


////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DNU;FIELD NODES ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

static int handle_latency_test_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    if (buf->len != BT_MESH_LATENCY_MSG_LEN_TEST) {
			printk("Error: Buffer length out of scope (handle_latency_outbound)");
		return;

    /* Immediately reply to the source */
    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_LATENCY_RSP_OP, BT_MESH_LATENCY_MSG_LEN_RSP);
    bt_mesh_model_msg_init(&buf, BT_MESH_LATENCY_RSP_OP);

    return bt_mesh_model_send(model, ctx, &buf, NULL, NULL);

    }
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// TOOLS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/**
 * Returns true if the specified address is an address of the local element.
 */
static bool address_is_local(struct bt_mesh_model *mod, uint16_t addr)
{
	return bt_mesh_model_elem(mod)->addr == addr;
}

/**
 * Returns true if the provided address is unicast address.
 */
static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}

/* Sets the address by left-shifting the node MAC address */
uint16_t get_unicast_addr(uint8_t mac[]){

    uint16_t addr;
    // DO: left shift mac address by two spaces..
    // addr =

    return addr; // e.g. 0x7FFF
};

/* Sets the device key by left-shifting the node MAC address */
static uint8_t get_dev_key(uint8_t mac[]){

    uint8_t key[16] = {0};

    for (int i = 0, i < 6, i++){

        key[i] = mac[i];
    }

    return key;
};

static const uint8_t *extract_msg(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}


