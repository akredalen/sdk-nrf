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

/* CLIENT:

// on boot: check rolle

// SENDING MESSAGES:
for (all addresses) {
    // get address
    for (i < 50, i++){
        // save timestamp for outgoing message
        // send unicast-message
        // wait for response
    }

}

// RECEIVING MESSAGES:

    message_handler{
        // save timestamp
        // check sequence number
            // compare timestamps
            // calculate rrt and save to file
    }

*/


/* TEST NODE */

static int latency_send_msg(struct bt_mesh_settings_srv *srv, uint16_t addr, struct bt_mesh_settings_latency *msg){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr,
            //.send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_LATENCY_OUTBOUND_OP, BT_MESH_LATENCY_MSG_LEN_OUTBOUND);
    bt_mesh_model_msg_init(&buf, BT_MESH_LATENCY_OUTBOUND_OP);

    return bt_mesh_model_send(&srv, &ctx, &buf, NULL, NULL);
}


static void handle_latency_inbound_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    int64_t current_uptime = k_uptime_get();

    if (buf->len != BT_MESH_LATENCY_MSG_LEN_INBOUND) {
			printk("Error: Buffer length out of scope (handle_latency)");
		return;
	}

    // ..... continue latency test....


}

/* FIELD NODES */

static void handle_latency_outbound_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    if (buf->len != BT_MESH_LATENCY_MSG_LEN_OUTBOUND) {
			printk("Error: Buffer length out of scope (handle_latency_outbound)");
		return;

    /* Immediately reply with an inbound message */
    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_LATENCY_INBOUND_OP, BT_MESH_LATENCY_MSG_LEN_INBOUND);
    bt_mesh_model_msg_init(&buf, BT_MESH_LATENCY_INBOUND_OP);

    return bt_mesh_model_send(model, ctx, &buf, NULL, NULL);

}

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

static int define_unicast_addr(uint8_t mac_addr){

    uint16_t uni_addr;
    // left shift mac address by two spaces

    return address_is_unicast(uni_addr);
};


static int latency_set_unicast_addr(){
    int err;

    // left shift mac address by two spaces
    // addr = ...


    //err = !(address_is_unicast());
    return err;
};

static int init_node(enum Role role){

    int err;

    switch (role) {
        case TESTER_N:

            err = bt_mesh_cfg_app_key_add(net_idx, addr, key_net_idx, key_app_idx,
                            app_key[16], &status);

            if (err) {
                printk("Failed to add app key");
            }

            err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
                            BT_MESH_MODEL_ID_SETTINGS_SRV, &status); // use SETUP_SRV ?
            if (err) {
                printk("Failed to bind application");
            }

            err = bt_mesh_cfg_mod_pub_set(net_idx, addr, addr,
                            BT_MESH_MODEL_ID_SETTINGS_SRV, &pub, &status);

            if (err) {
                printk("Failed to set publication settings");
            }

            return;

        case FIELD_N:

            err = bt_mesh_cfg_app_key_add(net_idx, addr, key_net_idx, key_app_idx,
                            app_key[16], &status);

            if (err) {
                printk("Failed to add app key");
            }

            err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
                            BT_MESH_MODEL_ID_SETTINGS_SRV, &status); // use SETUP_SRV ?
            if (err) {
                printk("Failed to bind application");
            }
        
            err = bt_mesh_cfg_net_transmit_set(NULL, addr, transmitt_value, NULL);

            if (err) {
            printk("Failed to set transmit settings");
        }
        
            err = bt_mesh_cfg_relay_set(NULL, addr, NULL, NULL, NULL, NULL);

            if (err) {
            printk("Failed to set relay settings");
        }

        return;

    return err;


    }

};

/* Initialize node and set up parameters */
static int latency_init_test(){

    int err;
    static enum role = NONE;

    get_own_mac(own_mac);

    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
        role = TESTER_N;
    }
    else{
        role = FIELD_N;
    }

    err = init_node(role);

    return err;
}

/* cfg:

net_tansmitt_set
relay_set
mod_pub_set
mod_app_bind

 */



