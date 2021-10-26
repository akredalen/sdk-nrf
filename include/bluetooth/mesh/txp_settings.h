/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @defgroup bt_mesh Device TXP Models
 *  @{
 *  @brief API for the Device TXP Model.
 */

#ifndef BT_MESH_TXP_H__
#define BT_MESH_TXP_H__

#include <stdlib.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/model_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_MESH_NORDIC_SEMI_COMPANY_ID 0x0059
#define BT_MESH_MODEL_ID_TXP_CLI 0x0005
#define BT_MESH_MODEL_ID_TXP_SRV 0x0006

#define BT_MESH_DEVICE_TXP_GET_OP BT_MESH_MODEL_OP_3(0xC1, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_TXP_SET_OP BT_MESH_MODEL_OP_3(0xC2, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_TXP_STATUS_OP BT_MESH_MODEL_OP_3(0xC3, BT_MESH_NORDIC_SEMI_COMPANY_ID)

#define BT_MESH_DEVICE_TXP_MSG_LEN_GET 0
#define BT_MESH_DEVICE_TXP_MSG_MINLEN_SET 1
#define BT_MESH_DEVICE_TXP_MSG_MAXLEN_SET 5
#define BT_MESH_DEVICE_TXP_MSG_MINLEN_STATUS 1
#define BT_MESH_DEVICE_TXP_MSG_MAXLEN_STATUS 2

/** Mandatory parameters for the Settings Set message. */
struct bt_mesh_txp_set {
	/** TX Power to set on server */
	int8_t txp_value;
};

/** Parameters for the Settings Status message. */
struct bt_mesh_txp_status {
	/** Success / Failure status of Set message */
	bool status;
	/** The present value of the Settings state. */
	int8_t present_txp;
};

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* BT_MESH_TXP_H__ */
