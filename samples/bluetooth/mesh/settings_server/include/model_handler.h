/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <bluetooth/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif

const struct bt_mesh_comp *model_handler_init(void);

void hci_set_tx_power(uint8_t handle_type, uint16_t handle, uint8_t txp_lvl);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */