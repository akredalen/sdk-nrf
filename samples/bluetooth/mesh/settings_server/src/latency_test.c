// move to model_handler? 

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "settings.h"
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

///////////////////////// DRAFT OF TEST IMPLM ////////////////////////

#define MSG_AMOUNT 50
#define NODES_TOTAL 2

static uint8_t transmitt_value = 3;

static uint8_t own_mac[6];

static uint8_t mac_addr_test_node[6] = {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43};
static uint16_t app_idx; // needed?
static uint16_t addr;

static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data node_data_mac_ttl[NODES_TOTAL] = {
    {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    };

uint8_t target_mac[6];
int target_ttl;

static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_TEST_INIT(&latency_resp_handler);

struct bt_mesh_settings_latency latency_msg;

/* Run Latency test... */
void start_latency_test (void){

    int err;

    /* Fetch next node address and set TTL (hard coded)*/
    for (int i = 0; i < NODES_TOTAL; i++){
            
            target_mac = node_data_mac_ttl[i].mac_address;
            target_ttl = node_data_mac_ttl[i].ttl;
            
            err = set_unicast_addr(target_mac);
            if (err) {
                printk("Error: unvalid unicast address");
            }
            err = bt_mesh_cfg_ttl_set(NULL, addr, target_ttl, NULL); // DO: fix parameter - address
            if (err) {
                printk("Error: unable to set TTL value");
            }

            /* Send flood of messages to unicast address */
            for (int k = 0; k < MSG_AMOUNT; ++k) {

                err = send_latency_test_msg(&settings_cli, &addr, &latency_msg);
                if (err){
                    printk("ERROR: latency message nr. %d failed. \n", k);
                }
                else{
                    save_time_stamp();
                }

                // wait for reply from response handler...

                // logg over ethernet
                
            }
    }
}

static void save_time_stamp(){
    
    int64_t current_uptime = k_uptime_get();

    // save...
}

static int send_latency_test_msg(struct bt_mesh_settings_cli *cli, uint16_t addr, struct bt_mesh_settings_latency *msg){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr,
            //.send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP, BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY);
    bt_mesh_model_msg_init(&buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP);

    // Add sequence number to buffer
    // net_buf_simple_add_u8(&buf, msg->target_mac);
    // net_buf_simple_add_u8(&buf, msg->seq_num);

    return bt_mesh_model_send(&cli, &ctx, &buf, NULL, NULL);

}


static void handle_latency_resp_msg(struct bt_mesh_model *model,
                                  struct bt_mesh_msg_ctx *ctx,
                                  struct net_buf_simple *buf)
{

    if (buf->len != BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY) {
			printk("Error: Buffer length out of scope (handle_latency)");
		return;
	}

    int64_t current_uptime = k_uptime_get();

    // .....


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

static int define_unicast_addr(unit8_t mac_addr){
    
    int err;

    // left shift mac address by two spaces


    //err = !(address_is_unicast());
    return err;
};

/*  */

static int set_unicast_addr(){
    int err;

    // left shift mac address by two spaces
    // addr = ...


    //err = !(address_is_unicast());
    return err;
};

struct bt_mesh_cfg_mod_pub pub = {
    .addr = addr,
    .app_idx = app_idx,
    .ttl = 4,
    .period = 0,
}

static int init_node(enum Role role){

    int err;

    switch (role) {
        case TESTER_N:

            err = bt_mesh_cfg_app_key_add(net_idx, addr, key_net_idx, key_app_idx,
                            app_key[16], *status);

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

            err = bt_mesh_cfg_app_key_add(uint16_t net_idx, uint16_t addr, uint16_t key_net_idx, uint16_t key_app_idx,
                            const uint8_t app_key[16], uint8_t *status);

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

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates RRT and logs the results using ethernet.
Tge FIELD node only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N};
static enum role;

/* Initialize node and set up parameters */
static void init_latency_test(){

    get_own_mac(own_mac);

    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
        role = TESTER_N;
    }
    else{
        role = FIELD_N;
    }
    init_node(role);
}

/* cfg:

net_tansmitt_set
relay_set
mod_pub_set
mod_app_bind

 */



