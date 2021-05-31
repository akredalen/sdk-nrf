/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#ifndef LATENCY_TEST
#define LATENCY_TEST

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MSG_AMOUNT 50
#define NODES_TOTAL 2

static uint8_t transmitt_value = 3;

static uint8_t own_mac[6];

/* MAC address of client/test node */
static uint8_t mac_addr_test_node[6] = {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43};

// static uint16_t app_idx; // needed?
// static uint16_t addr;

uint8_t target_mac[6];
int target_ttl;

static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data node_data_mac_ttl[NODES_TOTAL] = {
    {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    };

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates RRT and logs the results using ethernet.
Tge FIELD node only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N, NONE};

/* State of the Latency Test. 
INIT: initializes nodes and 
RUN: latency test is running. Sends latency message 
and then waits for response.
CONT: latency response message has arrived. Go to rtt calculation, then RUN. 
*/
static enum Test_State {INIT, RUN, CONT}

static int latency_init_test();

/* Returns the bit-shifted mac address as a unique unicast address
    on success, returns 1 otherwise. */
static int latency_get_unicast_addr(uint8_t mac_addr[6]);

void handle_latency_inbound_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf);

void handle_latency_outbound_msg(struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf);

#endif /* LATENCY_TEST */