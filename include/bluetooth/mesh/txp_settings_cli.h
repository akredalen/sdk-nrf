/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @defgroup bt_mesh_txp_cli TXP Client model
 *  @{
 *  @brief API for the TXP Client model.
 */

#ifndef BT_MESH_TXP_CLI_H__
#define BT_MESH_TXP_CLI_H__

#include <bluetooth/mesh/model_types.h>
#include "txp_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @def BT_MESH_TXP_CLI_INIT
 *
 * @brief Initialization parameters for a bt_mesh_txp_cli instance.
 *
 * @param[in] _status_handler Status message handler.
 */
#define BT_MESH_TXP_CLI_INIT(_status_handler)                                                      \
	{                                                                                          \
		.status_handler = _status_handler,                                                 \
		.pub = {.msg = NET_BUF_SIMPLE(BT_MESH_MODEL_BUF_LEN(                               \
				BT_MESH_DEVICE_TXP_SET_ACK_OP, BT_MESH_DEVICE_TXP_MSG_MAXLEN_SET)) }   \
	}

/** @def BT_MESH_MODEL_TXP_CLI
 *
 * @brief TXP Client model composition data entry.
 *
 * @param[in] _cli Pointer to a bt_mesh_txp_cli instance.
 */
#define BT_MESH_MODEL_TXP_CLI(_cli)                                                                \
	BT_MESH_MODEL_VND_CB(BT_MESH_NORDIC_SEMI_COMPANY_ID, BT_MESH_MODEL_ID_TXP_CLI,             \
			     _bt_mesh_txp_cli_op, &(_cli)->pub,                                    \
			     BT_MESH_MODEL_USER_DATA(struct bt_mesh_txp_cli, _cli),                \
			     &_bt_mesh_txp_cli_cb)

/**
 * TXP Client structure.
 *
 * Should be initialized with the BT_MESH_TXP_CLI_INIT macro.
 */
struct bt_mesh_txp_cli {
	/** @brief TXP status message handler.
	 *
	 * @param[in] cli Client that received the status message.
	 * @param[in] ctx Context of the incoming message.
	 * @param[in] status TXP Status of the Server that published the message.
	 */
	void (*const status_handler)(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
				     const struct bt_mesh_txp_status *status);
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/** Access model pointer. */
	struct bt_mesh_model *model;
};

/** @brief Get the status of the bound srv.
 *
 * Response will be passed to the
 * @ref bt_mesh_txp_cli::status_handler callback.
 *
 * @param[in] cli Client model to send on.
 * @param[in] ctx Message context, or NULL to use the configured publish
 * parameters.
 *
 * @retval 0 Successfully sent the message.
 * @retval -EADDRNOTAVAIL A message context was not provided and publishing is
 * not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_txp_cli_get(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx);

/** @brief Set the TX power state in the srv (acknowledged)
 *
 * Response will be passed to the
 * @ref bt_mesh_txp_cli::status_handler callback.
 *
 * @param[in] cli Client model to send on.
 * @param[in] ctx Message context, or NULL to use the configured publish
 * parameters.
 * @param[in] set Contains the new txp value to set.
 *
 * @retval 0 Successfully sent the message.
 * @retval -EADDRNOTAVAIL A message context was not provided and publishing is
 * not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_txp_cli_set_ack(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			const struct bt_mesh_txp_set *set);

/** @brief Set the TX power state in the srv (unacknowledged)
 *
 * @param[in] cli Client model to send on.
 * @param[in] ctx Message context, or NULL to use the configured publish
 * parameters.
 * @param[in] set Contains the new txp value to set.
 *
 * @retval 0 Successfully sent the message.
 * @retval -EADDRNOTAVAIL A message context was not provided and publishing is
 * not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_txp_cli_set_unack(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			const struct bt_mesh_txp_set *set);

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _bt_mesh_txp_cli_op[];
extern const struct bt_mesh_model_cb _bt_mesh_txp_cli_cb;
/** @endcond */

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* BT_MESH_TXP_CLI_H__ */
