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
#include <bluetooth/mesh/cfg_cli.h>
#include "../../../../zephyr/subsys/bluetooth/mesh/prov.h"
#include "../../../../zephyr/include/bluetooth/mesh/cfg_cli.h"

// See example og self provisioning:
// C:\git_repos\ncs\zephyr\samples\bluetooth\mesh_provisioner\src\main.c


uint16_t addr;
static uint8_t own_mac[6];


static int init_node(enum Role role){ 

    
    int err;

    static const uint16_t net_idx;
    static const uint16_t app_idx;
    uint8_t *dev_key[16];

    uint8_t net_key[16] = {0}; 
    uint8_t app_key[16] = {0};

    get_own_mac(&own_mac);

    /* Define a unicast address and device key based on MAC address */
        err = set_unicast_addr(&addr);
        if (err){
            printk("Failed to set unicast address");
        }

        set_dev_key(&dev_key);

    /* Self provision */
        err = bt_mesh_provision(net_key, BT_MESH_NET_PRIMARY, 0, 0, addr, dev_key);

    /* Initial configurations */

        err = bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key[16], NULL);
        if (err) {
                    printk("Failed to add application key");
                }

        err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx,
                                BT_MESH_MODEL_ID_SETTINGS_SRV, NULL);
                if (err) {
                    printk("Failed to bind application");
                }

    /* Role specific configurations: */
    switch (role) {
        case TESTER_N:

            /* Set publication address */
            // struct bt_mesh_cfg_mod_pub pub{
            //     .addr = addr;
            //     .pub = 
            // };

            // err = bt_mesh_cfg_mod_pub_set(net_idx, addr, addr, BT_MESH_MODEL_ID_SETTINGS_SRV, pub, );

            break;

        case FIELD_N:

            // DO: specify cfg...

        break;

    return err;


    }

};

/* Initialize node and set up parameters */
int latency_init_test(){

    int err;
    
    static enum Role role;

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

/* Formula: Sets the address by using the 16 first bit of the MAC address,
and left-shifting by two bits. Comments provide an example*/
static int set_unicast_addr(uint16_t *addr){

    /* Showing example for MAC {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43} */

    uint8_t msb = own_mac[4];                   // MSB = 0x35 = 0011 0101
    uint8_t lsb = own_mac[5];                   // LSB = 0x43 = 0100 0011

    uint16_t ph;
    
    ph = ((uint16_t)msb << 8) | lsb;            // addr = 0x3543 = 0011 0101 0100 0011
    ph = ph << 2;                               // addr = 0xD50C = 1101 0101 0000 1100

    while (!address_is_unicast(ph)){            // addr = 0x750C = 0111 0101 0000 1100
        ph = ph - 0x1000;                      
    }

     *addr = ph; 

    return address_is_unicast(ph);              // New 16-bit addr:  0x750C
};

static int set_dev_key(uint8_t *key[]){

    for (int i = 0; i < len(own_mac); i++){

        *key[i] = own_mac[i];
    }

    return 0;
};

static const uint8_t *extract_msg(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}


