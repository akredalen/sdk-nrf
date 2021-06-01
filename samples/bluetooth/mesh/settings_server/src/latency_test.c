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

//////////////////////////// NODE INITIALIZATION /////////////////////////////////

static int init_node(enum Role role){ 

    int err;

    switch (role) {
        case TESTER_N:

            /* Provisioning */ // [DRAFT]

            /* Configurations */ // [DRAFT]

            // err = bt_mesh_cfg_app_key_add(net_idx, addr, key_net_idx, key_app_idx,
            //                 app_key[16], &status);

            // if (err) {
            //     printk("Failed to add app key");
            // }

            // err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
            //                 BT_MESH_MODEL_ID_SETTINGS_SRV, &status); // use SETUP_SRV ?
            // if (err) {
            //     printk("Failed to bind application");
            // }

            // err = bt_mesh_cfg_mod_pub_set(net_idx, addr, addr,
            //                 BT_MESH_MODEL_ID_SETTINGS_SRV, &pub, &status);

            // if (err) {
            //     printk("Failed to set publication settings");
            // }

            return;

        case FIELD_N:

            /* Provisioning */ // [DRAFT]

            // Unicast address

            uint16_t own_addr;
            static uint8_t own_mac[6];

            get_own_mac(&own_mac);
            own_addr = latency_get_unicast_addr(own_mac);

            // See the mesh shell for bt_mesh_prov... function. 

            // Device key

            // Network Key

            // IV index


            /* Configurations */ // MOVE this to be handled as commands from client
            
        //     err = bt_mesh_cfg_app_key_add(net_idx, addr, key_net_idx, key_app_idx,
        //                     app_key[16], &status);

        //     if (err) {
        //         printk("Failed to add app key");
        //     }

        //     err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
        //                     BT_MESH_MODEL_ID_SETTINGS_SRV, &status);
        //     if (err) {
        //         printk("Failed to bind application");
        //     }
        
        //     err = bt_mesh_cfg_net_transmit_set(NULL, addr, transmitt_value, NULL);

        //     if (err) {
        //     printk("Failed to set transmit settings");
        // }
        
        //     err = bt_mesh_cfg_relay_set(NULL, addr, NULL, NULL, NULL, NULL);

        //     if (err) {
        //     printk("Failed to set relay settings");
        // }

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
//////////////////////////////// TESTER NODE ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

static int latency_send_test_msg(struct bt_mesh_settings_srv *srv, uint16_t addr){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr,
            //.send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime
    
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_LATENCY_TEST_OP, BT_MESH_LATENCY_MSG_LEN_TEST);
    bt_mesh_model_msg_init(&buf, BT_MESH_LATENCY_TEST_OP);

        /* Message contains only OP code */

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
//////////////////////////////// FIELD NODES ///////////////////////////////
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
static int set_unicast_addr(uint8_t mac, uint16_t *addr){

    // DO: left shift mac address by two spaces..
    // &uni_addr =

    return address_is_unicast(addr); // e.g. 0x7FFF
};

static const uint8_t *extract_msg(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}


