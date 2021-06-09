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

// #define BT_MESH_LATENCY_TEST_OP \
// 		BT_MESH_MODEL_OP_3(0xC4, BT_MESH_NORDIC_SEMI_COMPANY_ID)

// #define BT_MESH_LATENCY_RSP_OP \
// 		BT_MESH_MODEL_OP_3(0xC5, BT_MESH_NORDIC_SEMI_COMPANY_ID)

// #define BT_MESH_TEST_CONFIG_CONFIG_OP \
// 		BT_MESH_MODEL_OP_3(0xC6, BT_MESH_NORDIC_SEMI_COMPANY_ID)

// #define BT_MESH_LATENCY_MSG_LEN_TEST 1
// #define BT_MESH_LATENCY_MSG_LEN_RSP 1
// #define BT_MESH_TEST_MSG_LEN_CONFIG 2

///////////////////////////////////////////////////////


// static uint8_t transmitt_value = 3;

/* MAC address of client/test node */
static uint8_t mac_addr_test_node[6] = {0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43};

static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data node_data_mac_ttl[NODES_TOTAL] = {
    {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    // .....
    };

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates RRT and logs the results using ethernet.
Tge FIELD node only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N};

/* Initializes node: self provisioning and default configuration */
int latency_init_test();

/* Sets the address by combining the MSB and the LSB of the MAC address,
and left-shifting by two bits*/
static int set_unicast_addr(uint16_t *addr);

/* Sets the device key by filling the left-most octets with the node MAC address */
static int set_dev_key(uint8_t *key[]);


#endif /* LATENCY_TEST */