/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @defgroup bt_mesh_settings Device Settings Models
 *  @{
 *  @brief API for the Device Settings model.
 */

#ifndef BT_MESH_SETTINGS_H__
#define BT_MESH_SETTINGS_H__

#include <stdlib.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/model_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_MESH_NORDIC_SEMI_COMPANY_ID 0x0059
#define BT_MESH_MODEL_ID_SETTINGS_CLI 0x0005
#define BT_MESH_MODEL_ID_SETTINGS_SRV 0x0006 

#define BT_MESH_DEVICE_SETTINGS_GET_OP \
		BT_MESH_MODEL_OP_3(0xC1, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_SETTINGS_SET_OP \
		BT_MESH_MODEL_OP_3(0xC2, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_SETTINGS_STATUS_OP \
		BT_MESH_MODEL_OP_3(0xC3, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_SETTINGS_STATUS_OP \
		BT_MESH_MODEL_OP_3(0xC3, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_LATENCY_OUTBOUND_OP \
		BT_MESH_MODEL_OP_3(0xC4, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_LATENCY_INBOUND_OP \
		BT_MESH_MODEL_OP_3(0xC4, BT_MESH_NORDIC_SEMI_COMPANY_ID)


#define BT_MESH_DEVICE_SETTINGS_MSG_LEN_GET 0 
#define BT_MESH_DEVICE_SETTINGS_MSG_MINLEN_SET 1
#define BT_MESH_DEVICE_SETTINGS_MSG_MAXLEN_SET 5
#define BT_MESH_DEVICE_SETTINGS_MSG_MINLEN_STATUS 1
#define BT_MESH_DEVICE_SETTINGS_MSG_MAXLEN_STATUS 2

#define BT_MESH_LATENCY_MSG_LEN_OUTBOUND 1
#define BT_MESH_LATENCY_MSG_LEN_INBOUND 1

/** Mandatory parameters for the Settings Set message. */
struct bt_mesh_settings_set {
	/** TX Power to set on server */
	int8_t txp_value;
};

/** Mandatory parameters for the Settings Latency message. */
struct bt_mesh_settings_latency {
	/** MAC address of target node */
	uint8_t target_mac[6];
};

/** Parameters for the Settings Status message. */
struct bt_mesh_settings_status {
	/** Success / Failure status of Set message */
	bool status;
	/** The present value of the Settings state. */
	int8_t present_txp;
};

// MOVE THIS!
/** @def BT_MESH_SETTINGS_CLI_TEST_INIT
 *
 * @brief Initialization parameters for a bt_mesh_settings_cli test instance.
 *
 * @param[in] _status_handler Latency response message handler.
 */
#define BT_MESH_SETTINGS_TEST_INIT(_status_handler)						\
	{																	\
		.handle_latency_resp = _handle_latency_resp,								\
		.pub = {.msg = NET_BUF_SIMPLE(BT_MESH_MODEL_BUF_LEN(			\
				BT_MESH_DEVICE_SETTINGS_LATENCY_OP,							\
				BT_MESH_DEVICE_SETTINGS_MSG_LEN_LATENCY)) }				\
	}

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* BT_MESH_SETTINGS_CLI_H__ */