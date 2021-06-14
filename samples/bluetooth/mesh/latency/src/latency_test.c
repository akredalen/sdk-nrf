#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "settings.h"
#include "settings_srv.h"
#include "settings_cli.h"
#include "latency_test.h"
#include "pca20036_ethernet.h" // get_own_ utils
#include "ethernet_utils.h" // _are_equal utils
#include <bluetooth/mesh/cfg_cli.h>
#include "../../../../zephyr/subsys/bluetooth/mesh/prov.h"
#include "../../../../zephyr/include/bluetooth/mesh/cfg_cli.h"
#include "wizchip_conf.h" // modules\hal\wiznet\Ethernet\wizchip_conf.h

// static struct bt_mesh_settings_cli settings_cli = BT_MESH_SETTINGS_CLI_INIT(&status_handler);

/* MAC address of client/test node */
static uint8_t mac_addr_test_node[6] = {0xB0, 0xEE, 0x2F, 0x91, 0x30, 0x5A};
// 15 ---> B0:EE:2F:91:30:5A (TESTER NODE)
// 17 ---> B0:0A:75:A2:DF:53 (FIELD NODE)

static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data node_data_mac_ttl[NODES_TOTAL] = {
    {{0xB0, 0xEE, 0x2F, 0x91, 0x30, 0x5A}, 1},
    {{0xB0, 0x0A, 0x75, 0xA2, 0xDF, 0x53}, 2}
    // {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    // {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    // .....
    };

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates Round-trip time (rtt) and logs the results using ethernet.
The FIELD node(s) only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N};
static enum Role role;

/*
OUTBOUND messages are those that are sent from the TEST node, to the FIELD node.
INBOUND messages are those that are sent from the FIELD node, back to the TEST node
RTT is the calculated Round-Trip-Time
*/
static int64_t out_time; 
static int64_t in_time;
static int64_t rtt;
static int lost_msg_count = 0;


static const uint16_t net_idx = BT_MESH_NET_PRIMARY;
static const uint16_t app_idx = 0;
static uint8_t dev_key[16];
static const uint8_t net_key[16] = {1}; 
static uint8_t app_key[16] = {2};

static uint16_t target_addr = 0x0000;
static uint16_t own_addr = 0x0000;
static uint8_t own_mac[6];
static uint8_t target_mac[6];
static int target_ttl;

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// TOOLS /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/**
 * Returns true if the provided address is unicast address.
 */
static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}


/* Sets the address by combining the two rightmost 8-bits of the MAC address,
and left-shifting by two bits*/
static int set_unicast_addr(uint16_t *addr, uint8_t own_mac[6]){

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
static int set_dev_key(uint8_t key[16]){

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
    uint8_t tmp_str[6];

    err8 = ctlnetwork(CN_GET_NETINFO, (void *)&gWIZNETINFO);
    if (err8 == -1) {
        return 1;
    }

    err8 = ctlwizchip(CW_GET_ID, (void *)tmp_str);
    if (err8 == -1) {
        return 1;
    }

    memcpy(own_mac, gWIZNETINFO.mac, 6);

    return 0;

}

////////////////////////////////////////////////////////////////////////////
///////////////////////// LATENCY TEST /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int latency_test_run(){

    int err = 0;

	struct bt_mesh_settings_status status;
	struct bt_mesh_msg_ctx ctx;
	
	// err = latency_init_test();
	// if (err){
	// 	printk("Error: Failed to initialize node");
	// 	return err;
	// }

    // set_unicast_addr(&own_addr, mac_addr_test_node);

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
		
		set_unicast_addr(&target_addr, target_mac);
		
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
			// err = bt_mesh_settings_cli_get(&settings_cli, &ctx, &status);

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
///////////////////////// NODE INITIALIZATION //////////////////////////////
////////////////////////////////////////////////////////////////////////////

int latency_test_init(){

    int err;

    err = fetch_own_mac();
    if (err){
        printk("Error printing network info\n");
    }
    else{
        printf("\nMAC address: %u:%u:%u:%u:%u:%u\n", \
        own_mac[0], own_mac[1], own_mac[2], own_mac[3], own_mac[4], own_mac[5]);
    }

    err = set_unicast_addr(&own_addr, own_mac);
    if (err){
        printk("Failed to set Unicast Address\n");
    }
    printk("Unicast address: %u\n", own_addr);

    err = set_dev_key(dev_key);
    if (err){
        printk("Failed to set Device Key\n");
    }

///////////////////////////////////////////////////////////////////

    // if (bt_mesh_is_provisioned()){
    //     printk("Node is already provisioned!\n ");
    // }else{
    //     err = bt_mesh_provision(net_key, net_idx, 0, 0, 1, dev_key);
    //     if (err == -EALREADY) {
	// 	    printk("Using stored settings\n");
	//     } else if (err) {
	// 	    printk("Provisioning failed (err %d)\n", err);
	// 	    return err;
	//     } else {
	// 	    printk("Provisioning completed\n");
	//     }
    // }

    // err = bt_mesh_cfg_app_key_add(net_idx, own_addr, net_idx, app_idx, app_key, NULL);
    // if (err < 0) {
    //             printk("Failed to add application key\n");
    //         }

    // err = bt_mesh_cfg_mod_app_bind(net_idx, own_addr, 0 , app_idx,
    //                             BT_MESH_MODEL_ID_SETTINGS_SRV, NULL);
    //         if (err < 0) {
    //             printk("Failed to bind application\n");
    //         }

    // /* Tester node will also be using the cfg client model */
    // if (mac_addresses_are_equal(own_mac, mac_addr_test_node)){
    //     role = TESTER_N;

    //     err = bt_mesh_cfg_mod_app_bind(net_idx, own_addr, 0, app_idx,
    //                             BT_MESH_MODEL_ID_SETTINGS_CLI, NULL);
    //     if (err < 0) {
    //         printk("Failed to bind application\n");
    //     }
    // }
    // else{
    //     role = FIELD_N;
    // }

    return err;
}
