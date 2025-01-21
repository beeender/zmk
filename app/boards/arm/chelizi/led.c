#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>

#define LED_NUM_LOCK 0x01
#define LED_CAPS_LOCK 0x02
#define LED_SCROLL_LOCK 0x04

#define LED1_NODE DT_NODELABEL(led1)
#define LED2_NODE DT_NODELABEL(led2)
#define LED3_NODE DT_NODELABEL(led3)

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static int hid_indicators_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(led, hid_indicators_changed_listener);
ZMK_SUBSCRIPTION(led, zmk_hid_indicators_changed);

static int hid_indicators_changed_listener(const zmk_event_t *eh) {
#define CHECK_READY(led_dev) \
    { \
        if (!gpio_is_ready_dt(&led_dev)) { \
            LOG_ERR("GPIO " #led_dev " is not ready"); \
            return -1; \
        } \
    }

#define CONFIGURE_GPIO(led_dev) \
    { \
        if (gpio_pin_configure_dt(&led_dev, GPIO_OUTPUT_ACTIVE) < 0) { \
            LOG_ERR("Failed to configure" #led_dev); \
            return -1; \
        } \
    }

    static bool inited = false;
    if (!inited) {
        CHECK_READY(led1);
        CHECK_READY(led2);
        CHECK_READY(led3);
        CONFIGURE_GPIO(led1);
        CONFIGURE_GPIO(led2);
        CONFIGURE_GPIO(led3);
        inited = true;
    }
    struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);

    gpio_pin_set_dt(&led1, ev->indicators & LED_NUM_LOCK);
    gpio_pin_set_dt(&led2, ev->indicators & LED_CAPS_LOCK);
    gpio_pin_set_dt(&led3, ev->indicators & LED_SCROLL_LOCK);

    return 0;
}
