#include <string.h>
#include "settings_srv.h"
#include "model_utils.h"
#include <bluetooth/mesh/models.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <sys/byteorder.h>

#include "model_utils.h"

static void encode_status(struct net_buf_simple *buf,
			  const struct bt_mesh_settings_status *status)
{
	bt_mesh_model_msg_init(buf, BT_MESH_DEVICE_SETTINGS_STATUS_OP);
	net_buf_simple_add_u8(buf, status->present_txp);
	net_buf_simple_add_u8(buf, status->status);
}

static void rsp_status(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *rx_ctx,
		       const struct bt_mesh_settings_status *status)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DEVICE_SETTINGS_STATUS_OP,
				 BT_MESH_DEVICE_SETTINGS_MSG_MAXLEN_STATUS);

	encode_status(&msg, status);

	(void)bt_mesh_model_send(model, rx_ctx, &msg, NULL, NULL);
}

void handle_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		       struct net_buf_simple *buf)
{
	if (buf->len != BT_MESH_DEVICE_SETTINGS_MSG_LEN_GET) {
		printk("Error: Buffer length out of scope (1)");
		return;
	}

	struct bt_mesh_settings_srv *srv = model->user_data;
	struct bt_mesh_settings_status status = { 0 };

	srv->handlers->get(srv, ctx, &status);

	rsp_status(model, ctx, &status);
}

static void settings_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		      struct net_buf_simple *buf, bool ack)
{
	if (buf->len < BT_MESH_DEVICE_SETTINGS_MSG_MINLEN_SET ||
	    buf->len > BT_MESH_DEVICE_SETTINGS_MSG_MAXLEN_SET) {
		printk("Error: Buffer length out of scope (settings_set)");
		return;
	}

	struct bt_mesh_settings_srv *srv = model->user_data;
	struct bt_mesh_settings_status status = { 0 };
	struct bt_mesh_settings_set set;

	uint8_t msg = net_buf_simple_pull_u8(buf);
	uint8_t tid = net_buf_simple_pull_u8(buf);

	set.txp_value = msg;

	if (tid_check_and_update(&srv->prev_transaction, tid, ctx) != 0) {
		/* If this is the same transaction, we don't need to send it
		 * to the app, but we still have to respond with a status.
		 */
		srv->handlers->get(srv, NULL, &status);
		goto respond;
	}

	srv->handlers->set(srv, ctx, &set, &status);

	// (void)bt_mesh_settings_srv_pub(srv, NULL, &status);

respond:
	if (ack) {
		rsp_status(model, ctx, &status);
	}
}

/* Gives optional ack */
void handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		       struct net_buf_simple *buf)
{
	settings_set(model, ctx, buf, true);
}

extern const struct bt_mesh_model_op _bt_mesh_settings_srv_op[];

int32_t bt_mesh_settings_srv_pub(struct bt_mesh_settings_srv *srv,
			    struct bt_mesh_msg_ctx *ctx,
			    const struct bt_mesh_settings_status *status);

int32_t bt_mesh_settings_srv_pub(struct bt_mesh_settings_srv *srv,
			    struct bt_mesh_msg_ctx *ctx,
			    const struct bt_mesh_settings_status *status)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DEVICE_SETTINGS_STATUS_OP,
				 BT_MESH_DEVICE_SETTINGS_MSG_MAXLEN_STATUS);
	
	encode_status(&msg, status);
	return model_send(srv->model, ctx, &msg);
}

static int bt_mesh_settings_srv_init(struct bt_mesh_model *model)
{
	struct bt_mesh_settings_srv *srv = model->user_data;

	srv->model = model;
	net_buf_simple_init(model->pub->msg, 0);

	return 0;
}

const struct bt_mesh_model_cb _bt_mesh_settings_srv_cb = {
	.init = bt_mesh_settings_srv_init
};

const struct bt_mesh_model_op _bt_mesh_settings_srv_op[] = {
    { BT_MESH_DEVICE_SETTINGS_GET_OP,    BT_MESH_DEVICE_SETTINGS_MSG_LEN_GET,    handle_get },
    { BT_MESH_DEVICE_SETTINGS_SET_OP,    BT_MESH_DEVICE_SETTINGS_MSG_MINLEN_SET,    handle_set },

    BT_MESH_MODEL_OP_END,
};