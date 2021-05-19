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
#define NODES_TOTAL 3

static uint8_t transmitt_value;

static uint8_t own_mac[6];

static uint8_t mac_addr_test_node[6] = {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43};

uint8_t addr_arr[NODES_TOTAL][6] = {
    {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

uint8_t target_addr[6];
uint16_t uni_addr;

static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_CLI_TEST_INIT(&latency_resp_handler);

struct bt_mesh_settings_latency latency_msg;

void start_latency_test (void){

int err;

    for (int i = 0; i < len(addr_arr); ++i) {

        /* Fill inn current mac address*/
        for (int j = 0; j < 6; ++j) {
            
            target_addr[j] = addr_arr[i][j];
            latency_msg.target_mac = target_addr;
            }

        /* Send flood of messages to unicast address */
        for (int k = 0; i < MSG_AMOUNT; ++k) {

            //latency_msg.seq_num = new_seq_num;

            err = send_latency_test_msg(&settings_cli, &uni_addr, &latency_msg);
            if (err){
                printk("ERROR: latency message failed. \n");
                }
            else{
                //save_time_stamp(new_seq_num);
                }
        
            }

    }
}

static void save_time_stamp(uint8_t seq_num){
    
    // DO: use k_uptime_get()
}

static int send_latency_test_msg(struct bt_mesh_settings_cli *cli, uint16_t addr, struct bt_mesh_settings_latency *msg){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr, // unicast address. Set during prov.
            .send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP, BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY);
    bt_mesh_model_msg_init(&buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP);

    // Add sequence number to buffer
    net_buf_simple_add_u8(&buf, msg->target_mac);
    net_buf_simple_add_u8(&buf, msg->seq_num);

    return bt_mesh_model_send(&cli, &ctx, &buf, NULL, NULL);

}


static void latency_resp_handler(struct bt_mesh_model *model,
                                  struct bt_mesh_msg_ctx *ctx,
                                  struct net_buf_simple *buf)
{

    if (buf->len != BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY) {
			printk("Error: Buffer length out of scope (handle_latency)");
		return;
	}


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

static int define_unicast_addr(){
    int err;

    get_own_mac(own_mac);
    // left shift mac address by two spaces


    //err = !(address_is_unicast());
    return err;
};

/*  */

static int define_unicast_addr(){
    int err;

    get_own_mac(own_mac);
    for (int i = 0; i < 6; ++i){
        uni_addr[i] = own_mac[i];
    }
    // left shift mac address by two spaces


    //err = !(address_is_unicast());
    return err;
};


static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data mac_to_ttl_value[NODES_TOTAL] = {
    {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    };

static int init_node(enum Role role){

    int err;

    switch (role) {
        case TESTER_N:

        // set parameters

        return;

        case FIELD_N:
    
        for (int i = 0; i < NODES_TOTAL; i++){
            if (mac_addresses_are_equal(mac_to_ttl_value[i].mac_address, own_mac)){
                bt_mesh_cfg_ttl_set(NULL, uni_addr, mac_to_ttl_value[i].ttl, NULL);
                break;
            }
        }
        
        err = bt_mesh_cfg_net_transmit_set(NULL, uni_addr, transmitt_value, NULL);
        
        // err = bt_mesh_cfg_relay_set(NULL, uni_addr, ....);
        return;

        default:
        err = 1;;

    return err;


    }

};

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates RRT and logs the results using ethernet.
Tge FIELD node only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N};

/* Initialize node and set up parameters */
static void init_latency_test(){

    get_own_mac(own_mac);

    if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
        init_node(TESTER_N);
    }
    else{
        init_node(FIELD_N);
    }
}





