#define DT_DRV_COMPAT qiyun_qyeg0213

#include <zephyr/kernel.h>

#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(qyeg0213, CONFIG_DISPLAY_LOG_LEVEL);

#define PANEL_WIDTH DT_INST_PROP(0, width)
#define PANEL_HEIGHT DT_INST_PROP(0, height)

#define MAX_LINE_BYTES 16 // =128/8

struct qyeg0213_cfg {
    struct gpio_dt_spec dc;
    struct gpio_dt_spec cs;
    struct gpio_dt_spec sclk;
    struct gpio_dt_spec sdi;
    struct gpio_dt_spec reset;
    struct gpio_dt_spec busy;
};

#define CLEAR(gpio)                                                                                \
    {                                                                                              \
        int ret = gpio_pin_set_dt(&gpio, 0);                                                       \
        if (ret) {                                                                                 \
            LOG_ERR("Failed to clear gpio" #gpio ", error %d", ret);                               \
        }                                                                                          \
    }

#define SET(gpio)                                                                                  \
    {                                                                                              \
        int ret = gpio_pin_set_dt(&gpio, 1);                                                       \
        if (ret) {                                                                                 \
            LOG_ERR("Failed to set gpio" #gpio ", error %d", ret);                                 \
        }                                                                                          \
    }

#define EPD_W21_MOSI_0 CLEAR(cfg->sdi)
#define EPD_W21_MOSI_1 SET(cfg->sdi)

#define EPD_W21_CLK_0 CLEAR(cfg->sclk)
#define EPD_W21_CLK_1 SET(cfg->sclk)

#define EPD_W21_CS_0 CLEAR(cfg->cs)
#define EPD_W21_CS_1 SET(cfg->cs)

#define EPD_W21_DC_0 CLEAR(cfg->dc)
#define EPD_W21_DC_1 SET(cfg->dc)

#define EPD_W21_RST_0 CLEAR(cfg->reset)
#define EPD_W21_RST_1 SET(cfg->reset)

#define ALLSCREEN_GRAGHBYTES 4000
static bool blanking_on = true;
static uint8_t disp_buf_black[ALLSCREEN_GRAGHBYTES];

static inline void spi_write_byte(const struct qyeg0213_cfg *cfg, uint8_t TxData) {
    uint8_t TempData;
    uint8_t scnt;
    TempData = TxData;

    EPD_W21_CLK_0;
    k_busy_wait(1);
    for (scnt = 0; scnt < 8; scnt++) {
        if (TempData & 0x80)
            EPD_W21_MOSI_1
        else
            EPD_W21_MOSI_0

        EPD_W21_CLK_1;
        k_busy_wait(1);
        EPD_W21_CLK_0;
        TempData = TempData << 1;
    }
}

static inline void write_command(const struct qyeg0213_cfg *cfg, uint8_t cmd) {
    EPD_W21_CS_1;
    EPD_W21_CS_0;
    EPD_W21_DC_0; // D/C#   0:command  1:data

    spi_write_byte(cfg, cmd);
    EPD_W21_CS_1;
}

static inline void write_data(const struct qyeg0213_cfg *cfg, uint8_t data) {
    EPD_W21_CS_1;
    EPD_W21_CS_0;
    EPD_W21_DC_1; // D/C#   0:command  1:data

    spi_write_byte(cfg, data);
    EPD_W21_CS_1;
}

static inline void busy_wait(const struct qyeg0213_cfg *cfg) {
    int pin = gpio_pin_get_dt(&cfg->busy);
    __ASSERT(pin >= 0, "Failed to get pin level");

    while (pin > 0) {
        k_msleep(1);
        pin = gpio_pin_get_dt(&cfg->busy);
    }
}

static inline void update_screen(const struct qyeg0213_cfg *cfg) {
    write_command(cfg, 0x24); // write RAM for black(0)/white (1)
    for (size_t i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        write_data(cfg, disp_buf_black[i]);
    }

    write_command(cfg, 0x22);
    write_data(cfg, 0xF7);
    write_command(cfg, 0x20);
    busy_wait(cfg);
}

static int qyeg0213_write(const struct device *dev, const uint16_t x, const uint16_t y,
                          const struct display_buffer_descriptor *desc, const void *buf) {
    LOG_DBG("qyeg0213_write");
    const struct qyeg0213_cfg *cfg = dev->config;

    LOG_DBG("x %u, y %u, height %u, width %u, pitch %u, buf size %u", x, y, desc->height,
            desc->width, desc->pitch, desc->buf_size);
    __ASSERT(buf != NULL, "Buffer is not available");

    for (size_t buf_x = 0; buf_x < desc->width; buf_x++) {
        size_t dest_x = x + buf_x;
        for (size_t buf_y = 0; buf_y < desc->height; buf_y++) {
            size_t dest_y = y + buf_y;
            uint8_t *buf_xy = (uint8_t *)buf + buf_x / 8 + buf_y * desc->width / 8;
            uint8_t bit = buf_x % 8;

            size_t idx = dest_x * MAX_LINE_BYTES + dest_y / 8;

            if (*buf_xy & BIT(bit)) {
                disp_buf_black[idx] &= ~(0x80 >> (dest_y % 8));
            } else {
                disp_buf_black[idx] &= 0xFF;
            }
        }
    }

    if (blanking_on == false) {
        update_screen(cfg);
    }
    return 0;
}

static int qyeg0213_read(const struct device *dev, const uint16_t x, const uint16_t y,
                         const struct display_buffer_descriptor *desc, void *buf) {
    LOG_DBG("not supported");
    return -ENOTSUP;
}

static int qyeg0213_blanking_off(const struct device *dev) {
    blanking_on = false;
    LOG_DBG("qyeg0213_blanking_off");
    const struct qyeg0213_cfg *cfg = dev->config;
    update_screen(cfg);
    return 0;
}

static int qyeg0213_blanking_on(const struct device *dev) {
    blanking_on = true;
    return 0;
}

static void *qyeg0213_get_framebuffer(const struct device *dev) {
    LOG_DBG("not supported");
    return NULL;
}

static int qyeg0213_set_brightness(const struct device *dev, const uint8_t brightness) {
    LOG_WRN("not supported");
    return -ENOTSUP;
}

static int qyeg0213_set_contrast(const struct device *dev, uint8_t contrast) {
    LOG_WRN("not supported");
    return -ENOTSUP;
}

static void qyeg0213_get_capabilities(const struct device *dev, struct display_capabilities *caps) {
    memset(caps, 0, sizeof(struct display_capabilities));
    caps->x_resolution = PANEL_WIDTH;
    caps->y_resolution = PANEL_HEIGHT;
    caps->supported_pixel_formats = PIXEL_FORMAT_MONO01;
    caps->current_pixel_format = PIXEL_FORMAT_MONO01;
    caps->screen_info = SCREEN_INFO_EPD | SCREEN_INFO_X_ALIGNMENT_WIDTH;
}

static int qyeg0213_set_orientation(const struct device *dev,
                                    const enum display_orientation orientation) {
    LOG_ERR("Unsupported");
    return -ENOTSUP;
}

static int qyeg0213_set_pixel_format(const struct device *dev, const enum display_pixel_format pf) {
    if (pf == PIXEL_FORMAT_MONO01) {
        return 0;
    }

    LOG_ERR("not supported");
    return -ENOTSUP;
}

static int qyeg0213_controller_init(const struct qyeg0213_cfg *cfg) {
    LOG_DBG("Enter qyeg0213_controller_init");

    EPD_W21_RST_0;
    k_msleep(100);
    EPD_W21_RST_1; // hard reset
    k_msleep(100);

    busy_wait(cfg);
    write_command(cfg, 0x12); // soft reset
    busy_wait(cfg);

    write_command(cfg, 0x74); // set analog block control
    write_data(cfg, 0x54);
    write_command(cfg, 0x7E); // set digital block control
    write_data(cfg, 0x3B);

    write_command(cfg, 0x01); // Driver output control
    write_data(cfg, 0xF9);
    write_data(cfg, 0x00);
    write_data(cfg, 0x00);

    write_command(cfg, 0x11); // data entry mode
    write_data(cfg, 0x01);

    write_command(cfg, 0x44); // set Ram-X address start/end position
    write_data(cfg, 0x01);
    write_data(cfg, 0x10); // 0x0F-->(15+1)*8=128

    write_command(cfg, 0x45); // set Ram-Y address start/end position
    write_data(cfg, 0xF9);    // 0xF9-->(249+1)=250
    write_data(cfg, 0x00);
    write_data(cfg, 0x00);
    write_data(cfg, 0x00);

    write_command(cfg, 0x3C); // BorderWavefrom
    write_data(cfg, 0x01);

    write_command(cfg, 0x18);
    write_data(cfg, 0x80);

    write_command(cfg, 0x4E); // set RAM x address count to 0;
    write_data(cfg, 0x01);
    write_command(cfg, 0x4F); // set RAM y address count to 0xF9-->(249+1)=250;
    write_data(cfg, 0xF9);
    write_data(cfg, 0x00);
    busy_wait(cfg);

    // Init the red screen layer. Black one doesn't need this since it will be
    // written later.
    write_command(cfg, 0x26); // write RAM for red(1)/white (0)
    for (size_t i = 0; i < ALLSCREEN_GRAGHBYTES; i++) {
        write_data(cfg, 0);
    }

    memset(disp_buf_black, 0xff, ALLSCREEN_GRAGHBYTES);
    LOG_DBG("Exit qyeg0213_controller_init");
    return 0;
}

static int qyeg0213_init(const struct device *dev) {
#define CHECK_READY(gpio)                                                                          \
    if (!device_is_ready(gpio.port)) {                                                             \
        LOG_ERR("Could not get GPIO port for qyeg0213 " #gpio);                                    \
    }

    LOG_DBG("Enter qyeg0213_init");

    const struct qyeg0213_cfg *cfg = dev->config;

    CHECK_READY(cfg->dc);
    gpio_pin_configure_dt(&cfg->dc, GPIO_OUTPUT_HIGH);

    CHECK_READY(cfg->cs);
    gpio_pin_configure_dt(&cfg->cs, GPIO_OUTPUT_HIGH);

    CHECK_READY(cfg->sclk);
    gpio_pin_configure_dt(&cfg->sclk, GPIO_OUTPUT_HIGH);

    CHECK_READY(cfg->sdi);
    gpio_pin_configure_dt(&cfg->sdi, GPIO_OUTPUT_HIGH);

    CHECK_READY(cfg->reset);
    gpio_pin_configure_dt(&cfg->reset, GPIO_OUTPUT_HIGH);

    CHECK_READY(cfg->busy);
    gpio_pin_configure_dt(&cfg->busy, GPIO_INPUT | GPIO_PULL_DOWN);

    return qyeg0213_controller_init(cfg);
}

static const struct qyeg0213_cfg qyeg0213_config = {
    .dc = GPIO_DT_SPEC_INST_GET(0, dc_gpios),
    .cs = GPIO_DT_SPEC_INST_GET(0, cs_gpios),
    .sclk = GPIO_DT_SPEC_INST_GET(0, sclk_gpios),
    .sdi = GPIO_DT_SPEC_INST_GET(0, sdi_gpios),
    .reset = GPIO_DT_SPEC_INST_GET(0, reset_gpios),
    .busy = GPIO_DT_SPEC_INST_GET(0, busy_gpios),
};

static const struct display_driver_api qyeg0213_driver_api = {
    .blanking_on = qyeg0213_blanking_on,
    .blanking_off = qyeg0213_blanking_off,
    .write = qyeg0213_write,
    .read = qyeg0213_read,
    .get_framebuffer = qyeg0213_get_framebuffer,
    .set_brightness = qyeg0213_set_brightness,
    .set_contrast = qyeg0213_set_contrast,
    .get_capabilities = qyeg0213_get_capabilities,
    .set_pixel_format = qyeg0213_set_pixel_format,
    .set_orientation = qyeg0213_set_orientation,
};

DEVICE_DT_INST_DEFINE(0, qyeg0213_init, NULL, NULL, &qyeg0213_config, POST_KERNEL,
                      CONFIG_APPLICATION_INIT_PRIORITY, &qyeg0213_driver_api);
