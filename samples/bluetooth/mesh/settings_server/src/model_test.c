#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"

///////////////// OP CODES AND LENGHTS /////////////////
#define BT_MESH_ONOFF_OP_GET BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_ONOFF_OP_SET BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_ONOFF_OP_STATUS BT_MESH_MODEL_OP_2(0x82, 0x04)

#define BT_MESH_ONOFF_MSG_LEN_GET 0
#define BT_MESH_ONOFF_MSG_MINLEN_SET 2
#define BT_MESH_ONOFF_MSG_MAXLEN_SET 4
#define BT_MESH_ONOFF_MSG_MINLEN_STATUS 1
#define BT_MESH_ONOFF_MSG_MAXLEN_STATUS 3

/////////////////// DEFINITIONS /////////////////////

struct bt_mesh_onoff_srv {
	/** Transaction ID tracker. */
	struct bt_mesh_tid_ctx prev_transaction;
	/** Handler function structure. */
	const struct bt_mesh_onoff_srv_handlers *handlers;
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/* Publication buffer */
	struct net_buf_simple pub_buf;
	/* Publication data */
	uint8_t pub_data[BT_MESH_MODEL_BUF_LEN(
		BT_MESH_ONOFF_OP_STATUS, BT_MESH_ONOFF_MSG_MAXLEN_STATUS)];
	/* Scene entry */
	struct bt_mesh_scene_entry scene;
};

/** Mandatory parameters for the Generic OnOff Set message. */
struct bt_mesh_onoff_set {
	/** State to set. */
	bool on_off;
	/** Transition parameters. */
	const struct bt_mesh_model_transition *transition;
};


struct bt_mesh_onoff_srv_handlers {
	/** @brief Set the OnOff state.
	 *
	 * @note This handler is mandatory.
	 *
	 * @param[in] srv Server instance to set the state of.
	 * @param[in] ctx Message context for the message that triggered the
	 * change, or NULL if the change is not coming from a message.
	 * @param[in] set Parameters of the state change.
	 * @param[out] rsp Response structure to be filled.
	 */
	void (*const set)(struct bt_mesh_onoff_srv *srv,
			  struct bt_mesh_msg_ctx *ctx,
			  const struct bt_mesh_onoff_set *set,
			  struct bt_mesh_onoff_status *rsp);
};



#define BT_MESH_MODEL_ONOFF_SRV(_srv)                                          \
	BT_MESH_MODEL_CB(BT_MESH_MODEL_ID_GEN_ONOFF_SRV,                       \
			 _bt_mesh_onoff_srv_op, &(_srv)->pub,                  \
			 BT_MESH_MODEL_USER_DATA(struct bt_mesh_onoff_srv,     \
						 _srv),                        \
			 &_bt_mesh_onoff_srv_cb)


const struct bt_mesh_model_op _bt_mesh_onoff_srv_op[] = {
	{ BT_MESH_ONOFF_OP_SET, BT_MESH_ONOFF_MSG_MAXLEN_SET, handle_set}, 
    // sier hvilken "handler" som skal kjøre når den gitte OP koden er mottatt
	BT_MESH_MODEL_OP_END,
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// FUNCTIONALITY ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

static void handle_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		       struct net_buf_simple *buf)
{
	if (buf->len != BT_MESH_ONOFF_MSG_MAXLEN_SET) { // sjekker lengde
		return;
	}

	struct bt_mesh_onoff_srv *srv = model->user_data;
	struct bt_mesh_onoff_status status = { 0 };

	srv->handlers->set(srv, ctx, &status); // kjører: .set ---> led_set()


static void led_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp){

                printk("Hello!");
                dk_set_led(1, true);

                // Do some work ....
            }

static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
    .set = led_set
};

/******************************************************************/
/************************* SNURRE I GANG MESH *********************/
/******************************************************************/

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */
static struct k_delayed_work attention_blink_work;

static void attention_blink(struct k_work *work)
{
	static int idx;
	const uint8_t pattern[] = {
		BIT(0) | BIT(1),
		BIT(1) | BIT(2),
		BIT(2) | BIT(3),
		BIT(3) | BIT(0),
	};
	dk_set_leds(pattern[idx++ % ARRAY_SIZE(pattern)]);
	k_delayed_work_submit(&attention_blink_work, K_MSEC(30));
}

static void attention_on(struct bt_mesh_model *mod)
{
	k_delayed_work_submit(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(struct bt_mesh_model *mod)
{
	k_delayed_work_cancel(&attention_blink_work);
	dk_set_leds(DK_NO_LEDS_MSK);
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(
		1, BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV,
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			BT_MESH_MODEL_ONOFF_SRV(&led_ctx[0].srv)), // endre kun denne linjen!!
		BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	k_delayed_work_init(&attention_blink_work, attention_blink);

	for (int i = 0; i < ARRAY_SIZE(led_ctx); ++i) {
		k_delayed_work_init(&led_ctx[i].work, led_work);
	}

	return &comp;
}

//////////////////////////////// MAIN.C FILE /////////////////////////////////////

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

void main(void)
{
	int err;

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}