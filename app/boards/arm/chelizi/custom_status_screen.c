/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */


#include <lvgl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

LV_IMG_DECLARE(logo);

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *zenlogo_icon;
    zenlogo_icon = lv_img_create(screen);
    lv_img_set_src(zenlogo_icon, &logo);
    lv_obj_align(zenlogo_icon, LV_ALIGN_TOP_RIGHT, 0, 0);

    return screen;
}
