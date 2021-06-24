#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#
cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_sources(app PRIVATE
	src/main.c
	src/pca20036_dfu/pca20036_ethernet.c
	src/pca20036_dfu/ethernet_dfu.c
	src/pca20036_dfu/ethernet_command_system.c
	src/pca20036_dfu/ethernet_utils.c
	src/pca20036_dfu/hp_led.c
	../../../../../nrf/samples/bluetooth/mesh/settings_client/src/settings_cli.c
	../../../../../nrf/samples/bluetooth/mesh/settings_server/src/settings_srv.c
	../../../../../modules/hal/wiznet/Callback/dhcp_cb.c
	../../../../../modules/hal/wiznet/Ethernet/W5500/w5500.c
	../../../../../modules/hal/wiznet/Ethernet/socket.c
	../../../../../modules/hal/wiznet/Ethernet/wizchip_conf.c
	../../../../../modules/hal/wiznet/Internet/DHCP/dhcp.c
	../../../../../modules/hal/wiznet/Internet/TFTP/tftp.c
	../../../../../modules/hal/wiznet/Internet/TFTP/netutil.c)
target_include_directories(app PRIVATE include
	../../../../../nrf/samples/bluetooth/mesh/settings_client/src
	../../../../../nrf/samples/bluetooth/mesh/settings_client/include
	../../../../../nrf/samples/bluetooth/mesh/settings_server/src
	../../../../../nrf/samples/bluetooth/mesh/settings_server/include
	../../../../../modules/hal/wiznet/Callback
	../../../../../modules/hal/wiznet/Ethernet
	../../../../../modules/hal/wiznet/Ethernet/W5500
	../../../../../modules/hal/wiznet/Internet/DHCP
	../../../../../modules/hal/wiznet/Internet/TFTP)

zephyr_library_include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${NRF_DIR}/subsys/bluetooth/mesh)
