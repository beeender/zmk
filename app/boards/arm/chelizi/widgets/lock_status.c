#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

#include "lock_status.h"

#define LED_NUM_LOCK 0x01
#define LED_CAPS_LOCK 0x02
#define LED_SCROLL_LOCK 0x04

extern const lv_font_t *default_font;
static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct lock_status_state {
    zmk_hid_indicators_t indicators;
};

static void set_lock_symbol(lv_obj_t *label, struct lock_status_state state) {
    static zmk_hid_indicators_t prev_ind = { 0xff };

    if (prev_ind == state.indicators) {
        return;
    }

    char status_str[20];
    char *nums_symbol = LV_SYMBOL_CLOSE;
    char *caps_symbol = LV_SYMBOL_CLOSE;
    if (state.indicators & LED_NUM_LOCK) {
        nums_symbol = LV_SYMBOL_OK;
    }
    if (state.indicators & LED_CAPS_LOCK) {
        caps_symbol = LV_SYMBOL_OK;
    }

    snprintf(status_str, sizeof(status_str), "%s NUMS %s CAPS", nums_symbol, caps_symbol);
    lv_label_set_text(label, status_str);
}

static void lock_status_update_cb(struct lock_status_state state) {
    struct zmk_widget_lock_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_lock_symbol(widget->obj, state); }
}

static struct lock_status_state lock_status_get_state(const zmk_event_t *eh) {
    struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    return (struct lock_status_state){.indicators = ev->indicators};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_lock_status, struct lock_status_state, lock_status_update_cb,
                            lock_status_get_state)

ZMK_SUBSCRIPTION(widget_lock_status, zmk_hid_indicators_changed);

int zmk_widget_lock_status_init(struct zmk_widget_lock_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_style_text_font(widget->obj, default_font, LV_PART_MAIN);

    sys_slist_append(&widgets, &widget->node);

    widget_lock_status_init();
    return 0;
}

lv_obj_t *zmk_widget_lock_status_obj(struct zmk_widget_lock_status *widget) { return widget->obj; }
