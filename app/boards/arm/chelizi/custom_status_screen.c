/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */

#include <lvgl.h>

#include "widgets/battery_status.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

const lv_font_t *default_font = &lv_font_montserrat_18;

static struct zmk_widget_battery_status battery_status_widget;

LV_IMG_DECLARE(logo);

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *zenlogo_icon;
    zenlogo_icon = lv_img_create(screen);
    lv_img_set_src(zenlogo_icon, &logo);
    lv_obj_align(zenlogo_icon, LV_ALIGN_TOP_RIGHT, 0, 0);

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(battery_status_widget.obj, LV_ALIGN_TOP_LEFT, 0, 0);

    return screen;
}
