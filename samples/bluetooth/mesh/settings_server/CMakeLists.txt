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
	src/model_handler.c
	src/settings_srv.c)
target_include_directories(app PRIVATE include)

zephyr_library_include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${NRF_DIR}/subsys/bluetooth/mesh)
