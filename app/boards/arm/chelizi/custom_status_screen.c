/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */

#include <lvgl.h>

#include <zephyr/random/random.h>

#include "widgets/battery_status.h"
#include "widgets/output_status.h"
#include "widgets/layer_status.h"
#include "widgets/lock_status.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

const lv_font_t *default_font = &lv_font_montserrat_18;

static struct zmk_widget_battery_status battery_status_widget;
static struct zmk_widget_output_status output_status_widget;
static struct zmk_widget_layer_status layer_status_widget;
static struct zmk_widget_lock_status lock_status_widget;

LV_IMG_DECLARE(logo);

struct zmk_widget_lucky_status {
    uint8_t red[6];
    uint8_t blue[1];
    lv_obj_t *obj;
};
static struct zmk_widget_lucky_status lucky_status_widget;

int zmk_widget_lucky_status_init(struct zmk_widget_lucky_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_12, LV_PART_MAIN);

    uint8_t *red = widget->red;
    for (int i = 0; i < 6;) {
        uint8_t r = sys_rand32_get() % 33 + 1;
        int j;
        for (j = 0; j < i; j++) {
            if (red[j] == r) {
                break;
            }
        }
        if (j >= i) {
            for (j = 0; j <= i; j++) {
                if (j == i || red[j] == 0) {
                    red[i] = r;
                    break;
                }
                if (red[j] > r) {
                    memmove(&red[j + 1], &red[j], 6 - j);
                    red[j] = r;
                    break;
                }
            }
            i++;
        }
    }

    widget->blue[0] = sys_rand32_get() % 16 + 1;

    char status_str[40];
    snprintf(status_str, sizeof(status_str),
             LV_SYMBOL_AUDIO " %d %d %d %d %d %d  %d " LV_SYMBOL_AUDIO, red[0], red[1], red[2],
             red[3], red[4], red[5], widget->blue[0]);
    lv_label_set_text(widget->obj, status_str);
    return 0;
}

lv_obj_t *zmk_display_status_screen() {

    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *zenlogo_icon;
    zenlogo_icon = lv_img_create(screen);
    lv_img_set_src(zenlogo_icon, &logo);
    lv_obj_align(zenlogo_icon, LV_ALIGN_TOP_RIGHT, 0, 0);

    zmk_widget_battery_status_init(&battery_status_widget, screen);
    lv_obj_align(battery_status_widget.obj, LV_ALIGN_TOP_LEFT, 0, 0);

    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align_to(output_status_widget.obj, battery_status_widget.obj, LV_ALIGN_OUT_BOTTOM_LEFT,
                    0, 2);

    zmk_widget_layer_status_init(&layer_status_widget, screen);
    lv_obj_align_to(layer_status_widget.obj, output_status_widget.obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                    2);

    zmk_widget_lock_status_init(&lock_status_widget, screen);
    lv_obj_align_to(lock_status_widget.obj, layer_status_widget.obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                    2);

    zmk_widget_lucky_status_init(&lucky_status_widget, screen);
    lv_obj_align_to(lucky_status_widget.obj, lock_status_widget.obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    return screen;
}
