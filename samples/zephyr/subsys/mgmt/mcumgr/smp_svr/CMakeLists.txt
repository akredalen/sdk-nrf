# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smp_svr)

target_sources(app PRIVATE ${ZEPHYR_BASE}/samples/subsys/mgmt/mcumgr/smp_svr/src/main.c)
target_sources_ifdef(CONFIG_MCUMGR_TRANSPORT_BT app PRIVATE ${ZEPHYR_BASE}/samples/subsys/mgmt/mcumgr/smp_svr/src/bluetooth.c)
