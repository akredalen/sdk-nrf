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

#define MSG_AMOUNT 1
#define NODES_TOTAL 1

/* Defines number of elements in array */
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

/* MAC address of client/test node */
static uint8_t mac_addr_test_node[6] = {0xB0, 0x0A, 0x75, 0xA2, 0xDF, 0x53};
// 17 ---> B0:0A:75:A2:DF:53  (TESTER NODE)
// 15 ---> B0:EE:2F:91:30:5A  (1. FIELD NODE)   Unicast address: 16744 / 0x4168
// 2 ---> B0:75:7F:01:64:3B (2. FIELD NODE)
// 12 --->  B0:74:07:DF:98:60 (3. FIELD NODE)
static struct Node_data{
    uint8_t mac_address[6];
    uint8_t ttl;
};

/* Hard-coded TTL values for each MAC address */
static struct Node_data node_data_mac_ttl[NODES_TOTAL] = {
    {{0xB0, 0xEE, 0x2F, 0x91, 0x30, 0x5A}, 0},
    // {{0xB0, 0x0A, 0x75, 0xA2, 0xDF, 0x53}, 2}
    // {{0xB0, 0xAE, 0xD4, 0xDA, 0x35, 0x43}, 1},
    // {{0xDE, 0xAD, 0xFA, 0xCE, 0x00, 0x00}, 2}
    // .....
    };

/* Role of node in network.
The TESTER node is responsible for sending messages to and receiving from the other FIELD noed in the network. 
The TESTER node calculates Round-trip time (rtt) and logs the results using ethernet.
The FIELD node(s) only responds to incoming messages  */
static enum Role {TESTER_N, FIELD_N};

int latency_test_run();

#endif /* LATENCY_TEST */