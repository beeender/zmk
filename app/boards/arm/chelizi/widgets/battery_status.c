/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/battery.h>
#include <zmk/usb.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>

#include "battery_status.h"

extern const lv_font_t *default_font;

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

static void set_battery_symbol(lv_obj_t *label, struct battery_status_state state) {
    uint8_t level = state.level;

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    char *icon_str;
    if (level > 95) {
        icon_str = LV_SYMBOL_BATTERY_FULL;
    } else if (level > 75) {
        icon_str = LV_SYMBOL_BATTERY_3;
    } else if (level > 45) {
        icon_str = LV_SYMBOL_BATTERY_2;
    } else if (level > 15) {
        icon_str = LV_SYMBOL_BATTERY_1;
    } else {
        icon_str = LV_SYMBOL_BATTERY_EMPTY;
    }
    char bat_str[20];
    if (state.usb_present) {
        snprintf(bat_str, 20, "%s BAT %s %d%%", icon_str, LV_SYMBOL_CHARGE, level);
    } else {
        snprintf(bat_str, 20, "%s BAT %d%%", icon_str, level);
    }
    LOG_DBG("Battery change");
    lv_label_set_text(label, bat_str);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
}

void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_battery_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_symbol(widget->obj, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_style_text_font(widget->obj, default_font, LV_PART_MAIN);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();

    return 0;
}

lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
    return widget->obj;
}
