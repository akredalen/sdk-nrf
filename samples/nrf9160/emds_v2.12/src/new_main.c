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
#include <zephyr/sys/printk.h>
#include "emds/emds.h"

LOG_MODULE_REGISTER(mqtt_simple, CONFIG_MQTT_SIMPLE_LOG_LEVEL);

#define EMDS_DEV_IRQ 24
#define EMDS_DEV_PRIO 0
#define EMDS_ISR_ARG 0
#define EMDS_IRQ_FLAGS 0

// Some fake data for EMDS
static uint8_t _some_data[702];
#define SHARED_DATA_EMDS_ID ((int) 0x100)
EMDS_STATIC_ENTRY_DEFINE(shared_data_store, SHARED_DATA_EMDS_ID, &_some_data, sizeof(_some_data));

static void button_handler_cb(uint32_t button_states, uint32_t has_changed)
{
	printk("Got to button handler\n");

	if (has_changed & button_states & DK_BTN1_MSK) {
		printk("BUTTON 1 PUSHED\n");
		NVIC_SetPendingIRQ(EMDS_DEV_IRQ);
	}
	
}

static void app_emds_cb(void)
{
	printk("SAMPLE HALTED!!!\n");
	// dk_set_leds(DK_LED2_MSK | DK_LED3_MSK | DK_LED4_MSK);
	// k_fatal_halt(K_ERR_CPU_EXCEPTION);
	// NVIC_SystemReset();
}

static void isr_emds_cb(void *arg)
{
	int err;
	ARG_UNUSED(arg);

	// /* Stop mpsl to reduce power usage. */
	// irq_disable(TIMER0_IRQn);
	// irq_disable(RTC0_IRQn);
	// irq_disable(RADIO_IRQn);

	// mpsl_uninit();

	printk("Interrupt callback activated\n");
	
	err = emds_store();
	if(err){
		printk("emds_store FAILED!\n");
	}
}

void main(void)
{
	int err;
	printk("EMDS example Started\n");

	printk("Initializing buttons...\n");
	dk_buttons_init(NULL);

#ifdef CONFIG_EMDS
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};

	dk_button_handler_add(&button_handler);

	err = emds_init(&app_emds_cb);
	if (err) {
		printk("Initializing emds failed (err %d)\n", err);
		return;
	}

	err = emds_load();
	if (err) {
		printk("Restore of emds data failed (err %d)\n", err);
		return;
	}

	err = emds_prepare();
	if (err) {
		printk("Preparation emds failed (err %d)\n", err);
		return;
	}

	IRQ_CONNECT(EMDS_DEV_IRQ, EMDS_DEV_PRIO, isr_emds_cb,
		    EMDS_ISR_ARG, EMDS_IRQ_FLAGS);
	irq_enable(EMDS_DEV_IRQ);


	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}
	#endif

}
