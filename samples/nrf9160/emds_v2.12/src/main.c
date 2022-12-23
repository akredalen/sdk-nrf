/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "emds/emds.h"

LOG_MODULE_REGISTER(mqtt_simple, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

#define EMDS_DEV_IRQ 23
#define EMDS_DEV_PRIO 0
#define EMDS_ISR_ARG 0
#define EMDS_IRQ_FLAGS 0

// Some fake data for EMDS
static uint8_t _some_data[702];
#define SHARED_DATA_EMDS_ID ((int) 0x100)
EMDS_STATIC_ENTRY_DEFINE(shared_data_store, SHARED_DATA_EMDS_ID, &_some_data, sizeof(_some_data));



static void app_emds_cb(void)
{
    NVIC_SystemReset();
}

static void isr_emds_cb(void *arg)
{
	ARG_UNUSED(arg);
	emds_store();
}

uint32_t shared_data_init(void)
{
 

#ifdef CONFIG_EMDS
	int err = emds_init(&app_emds_cb);
	if (err) {
		printk("Initializing emds failed (err %d)\n", err);
		return;
	}

	err = emds_load();
	if (err) {
		printk("Restore of emds data failed (err %d)\n", err);
		return 1;
	}

	err = emds_prepare();
	if (err) {
		printk("Preparation emds failed (err %d)\n", err);
		return 1;
	}

    LOG_INF("Recovered = 0x%02x", _some_data[2]);

	_some_data[2] = 0xAA;

    
	IRQ_CONNECT(EMDS_DEV_IRQ, EMDS_DEV_PRIO, isr_emds_cb,
		    EMDS_ISR_ARG, EMDS_IRQ_FLAGS);
	irq_enable(EMDS_DEV_IRQ);
#endif

    return 0;
}


uint32_t shared_data_shutdown(void)
{
#ifdef CONFIG_EMDS
    emds_store();
#endif
    return 0;
}


#if defined(CONFIG_DK_LIBRARY)
static void button_handler(uint32_t button_states, uint32_t has_changed)
{
	if (has_changed & button_states &
	    BIT(CONFIG_BUTTON_EVENT_BTN_NUM - 1)) {
		int ret;
		printk("BUTTON PUSHED: start shared data shutdown");
		shared_data_shutdown();
	}
}
#endif


void main(void)
{

	LOG_INF("EMDS Example Started");
	shared_data_init();

#if defined(CONFIG_DK_LIBRARY)
	dk_buttons_init(button_handler);
#endif

	while(1)
	{
		k_sleep(K_MSEC(1000));
	}
}
