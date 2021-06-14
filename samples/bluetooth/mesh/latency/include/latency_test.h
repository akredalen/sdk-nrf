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

/* Defines numer of elements in array */
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))


// static uint8_t transmitt_value = 3;

int latency_test_run();

/* Initializes node (self provisioning and default configuration)*/
int latency_test_init();

#endif /* LATENCY_TEST */