// move to model_handler? 

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "settings.h"

#define flood 50
#define total_addr 3

static uint8_t mac_addr_1[6] = {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43};
static uint8_t mac_addr_2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t mac_addr_3[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t addr_arr[total_addr][6] = {
    {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

uint8_t seq_num = 0;
uint8_t target_addr[6];

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
        for (int k = 0; i < flood; ++k) {
                
            seq_num++;

            latency_msg.seq_num = seq_num;

            err = send_latency_test_msg(&settings_cli, &uni_addr, &latency_msg);
            if err{
                printk("ERROR: latency message failed. Sequence number: %d \n", seq_num);
                }
            else{
                save_time_stamp(seq_num);
                }
        
            }

    }
}

static void save_time_stamp(uint8_t seq_num){
    
    // DO: seach available library functions. Save to array.
}

// Q: use unicast address or send MAC address as payload?

static int send_latency_test_msg(struct bt_mesh_settings_cli *cli, uint16_t addr, struct bt_mesh_settings_latency *msg){

    struct bt_mesh_msg_ctx ctx = {
            .addr = addr, // unicast address. Set during prov.
            .send_ttl = BT_MESH_TTL_DEFAULT, // needs to be set dynamically during runtime over DFU
    };

    BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP, BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY);
    bt_mesh_model_msg_init(&buf, BT_MESH_DEVICE_SETTINGS_LATENCY_OP);

    // Add sequence number to buffer
    net_buf_simple_add_u8(&buf, msg->target_mac);
    net_buf_simple_add_u8(&buf, msg->seq_num);

    return bt_mesh_model_send(model, &ctx, &buf, NULL, NULL);

}


static void handle_latency_resp(struct bt_mesh_model *model,
                                  struct bt_mesh_msg_ctx *ctx,
                                  struct net_buf_simple *buf)
{

    if (buf->len != BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY {
			printk("Error: Buffer length out of scope (handle_latency)");
		return;
	}


    }