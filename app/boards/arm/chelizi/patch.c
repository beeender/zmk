#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>

#define I2C0_NODE DT_NODELABEL(ioexpander)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

static int usb_conn_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(patch, usb_conn_changed_listener);
ZMK_SUBSCRIPTION(patch, zmk_usb_conn_state_changed);

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int usb_conn_changed_listener(const zmk_event_t *eh) {
    struct zmk_usb_conn_state_changed *ev = as_zmk_usb_conn_state_changed(eh);

    if (ev->conn_state == ZMK_USB_CONN_HID) {
        if (!device_is_ready(dev_i2c.bus)) {
            LOG_ERR("Failed to get I2C device binding\\n");
            return 0;
        }

        // Reset the I2C.
        // I have seen the keyboard loosing response, highly doubted that is
        // caused by the expander.
        i2c_reg_write_byte_dt(&dev_i2c, 0x06, 0x00);
        i2c_reg_write_byte_dt(&dev_i2c, 0x07, 0x00);
        i2c_reg_write_byte_dt(&dev_i2c, 0x03, 0x00);
        i2c_reg_write_byte_dt(&dev_i2c, 0x02, 0x00);
    }

    return ZMK_EV_EVENT_BUBBLE;
}
