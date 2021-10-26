/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#include <string.h>
#include <stdlib.h>
#include <bluetooth/mesh/models.h>
#include "model_utils.h"

#include "txp_settings_cli.h"

static void handle_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
	if (buf->len < BT_MESH_DEVICE_TXP_MSG_MINLEN_STATUS ||
	    buf->len > BT_MESH_DEVICE_TXP_MSG_MAXLEN_STATUS) {
		printk("Error: Buffer length out of scope (handle_status)");
		return;
	}

	struct bt_mesh_txp_cli *cli = model->user_data;
	struct bt_mesh_txp_status status;

	status.present_txp = net_buf_simple_pull_u8(buf);

	if (buf->len == 2) {
		status.status = net_buf_simple_pull_u8(buf);
	}

	if (model_ack_match(&cli->ack_ctx, BT_MESH_DEVICE_TXP_STATUS_OP, ctx)) {
		struct bt_mesh_txp_status *rsp =
			(struct bt_mesh_txp_status *)cli->ack_ctx.user_data;

		*rsp = status;
		model_ack_rx(&cli->ack_ctx);
	}

	if (cli->status_handler) {
		cli->status_handler(cli, ctx, &status);
	}
}

const struct bt_mesh_model_op _bt_mesh_txp_cli_op[] = {
	{ BT_MESH_DEVICE_TXP_STATUS_OP, BT_MESH_DEVICE_TXP_MSG_MINLEN_STATUS, handle_status },
	BT_MESH_MODEL_OP_END,
};

static int bt_mesh_txp_cli_init(struct bt_mesh_model *model)
{
	struct bt_mesh_txp_cli *cli = model->user_data;

	cli->model = model;
	net_buf_simple_init(cli->pub.msg, 0);
	model_ack_init(&cli->ack_ctx);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_txp_cli_cb = {
	.init = bt_mesh_txp_cli_init,
};

int bt_mesh_txp_cli_get(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			struct bt_mesh_txp_status *rsp)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DEVICE_TXP_GET_OP, BT_MESH_DEVICE_TXP_MSG_LEN_GET);

	bt_mesh_model_msg_init(&msg, BT_MESH_DEVICE_TXP_GET_OP);

	return model_ackd_send(cli->model, ctx, &msg, rsp ? &cli->ack_ctx : NULL,
			       BT_MESH_DEVICE_TXP_STATUS_OP, rsp);
}

int bt_mesh_txp_cli_set(struct bt_mesh_txp_cli *cli, struct bt_mesh_msg_ctx *ctx,
			const struct bt_mesh_txp_set *set, struct bt_mesh_txp_status *rsp)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DEVICE_TXP_SET_OP, BT_MESH_DEVICE_TXP_MSG_MAXLEN_SET);
	bt_mesh_model_msg_init(&msg, BT_MESH_DEVICE_TXP_SET_OP);

	net_buf_simple_add_u8(&msg, set->txp_value);
	net_buf_simple_add_u8(&msg, cli->tid++);

	return model_ackd_send(cli->model, ctx, &msg, rsp ? &cli->ack_ctx : NULL,
			       BT_MESH_DEVICE_TXP_STATUS_OP, rsp);
}
