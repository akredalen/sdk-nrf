/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @defgroup bt_mesh_txp_srv Device TXP Server model
 *  @{
 *  @brief API for the Device TXP Server model.
 */

#ifndef BT_MESH_TXP_SRV_H__
#define BT_MESH_TXP_SRV_H__

#include <bluetooth/mesh/model_types.h>
#include <bluetooth/mesh/dk_prov.h>

#include "txp_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @def BT_MESH_TXP_SRV_INIT
 *
 * @brief Init parameters for a bt_mesh_txp_srv instance.
 *
 * @param[in] _handlers State access handlers to use in the model instance.
 */
#define BT_MESH_TXP_SRV_INIT(_handlers)                                                            \
	{                                                                                          \
		.handlers = _handlers,                                                             \
		.pub = {                                                                           \
			.msg = NET_BUF_SIMPLE(                                                     \
				BT_MESH_MODEL_BUF_LEN(BT_MESH_DEVICE_TXP_STATUS_OP,                \
						      BT_MESH_DEVICE_TXP_MSG_MAXLEN_STATUS)),      \
		},                                                                                 \
	}

/** @def BT_MESH_MODEL_TXP_SRV
 *
 * @brief Device Settings Server model composition data entry.
 *
 * @param[in] _srv Pointer to a bt_mesh_txp_srv instance.
 */
#define BT_MESH_MODEL_TXP_SRV(_srv)                                                                \
	BT_MESH_MODEL_VND_CB(BT_MESH_NORDIC_SEMI_COMPANY_ID, BT_MESH_MODEL_ID_TXP_SRV,             \
			     _bt_mesh_txp_srv_op, &(_srv)->pub,                                    \
			     BT_MESH_MODEL_USER_DATA(struct bt_mesh_txp_srv, _srv),                \
			     &_bt_mesh_txp_srv_cb)

/**
 * Device Settings Server instance. Should primarily be initialized with the
 * BT_MESH_TXP_SRV_INIT macro.
 */
struct bt_mesh_txp_srv {
	/** Transaction ID tracker. */
	struct bt_mesh_tid_ctx prev_transaction;
	/** Handler function structure. */
	const struct bt_mesh_txp_srv_handlers *handlers;
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
};

/** Device Settings Server state access handlers. */
struct bt_mesh_txp_srv_handlers {
	/** @brief Set the Settings state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to set the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[in] set Parameters of the state change.
	 * @param[out] rsp Response structure to be filled, or NULL if no
	 * response is required.
	 */
	void (*const set)(struct bt_mesh_txp_srv *srv, struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_txp_set *set, struct bt_mesh_txp_status *rsp);

	/** @brief Get the Settings state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to get the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const get)(struct bt_mesh_txp_srv *srv, struct bt_mesh_msg_ctx *ctx,
			  struct bt_mesh_txp_status *rsp);
};

void handle_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		struct net_buf_simple *buf);

void handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		struct net_buf_simple *buf);

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _bt_mesh_txp_srv_op[];
extern const struct bt_mesh_model_cb _bt_mesh_txp_srv_cb;
/** @endcond */

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* BT_MESH_TXP_SRV_H__ */