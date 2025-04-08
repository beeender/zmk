#
# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT
#

# Suppresses duplicate unit-address warning at build time for power, clock, acl and flash-controller
# https://docs.zephyrproject.org/latest/build/dts/intro-input-output.html

list(APPEND EXTRA_DTC_FLAGS "-Wno-unique_unit_address_if_enabled")

if (CHELIZI_EINK)
    list(APPEND EXTRA_DTC_OVERLAY_FILE ${BOARD_DIR}/eink.overlay)
else ()
    list(APPEND EXTRA_DTC_OVERLAY_FILE ${BOARD_DIR}/led.overlay)
endif()
