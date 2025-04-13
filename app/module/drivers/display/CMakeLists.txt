# Copyright (c) 2021 The ZMK Contributors
# SPDX-License-Identifier: MIT

zephyr_library_amend()

zephyr_library_sources_ifdef(CONFIG_IL0323 il0323.c)
zephyr_library_sources_ifdef(CONFIG_QYEG0213 qyeg0213.c)
