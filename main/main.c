#include "common.h"

QueueHandle_t rotary_encoder_queue;
QueueHandle_t gpio_button_queue;

static void _set_time_zone (void) {
  setenv("TZ", CONFIG_TIMEZONE, 1);
  tzset();
}

void app_main(void) {
    _set_time_zone ();

    nvs_init             ();

    uint32_t restart_counter = 0;
    nvs_load_restart_counter  (&restart_counter);
    restart_counter ++;
    nvs_save_restart_counter  (restart_counter);

    // sd_espi_init         ();
    i2c_master_initialization (I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);

    oled_init            ();

    humidity_reader_init ();

    rotary_encoder_queue = xQueueCreate (2, sizeof(rotary_encoder_position_t));
    gpio_button_queue    = xQueueCreate (2, sizeof(gpio_button_event_t));

    rotary_encoder_init  (rotary_encoder_queue);
    gpio_button_init     (gpio_button_queue);
    menu_rotary_init     ();

    for (;;) {
        rotary_encoder_event ();
        gpio_button_event    ();

        menu_rotary_encoder_event     ();
        menu_rotary_gpio_button_event ();

        humidity_read_data   ();

        menu_titles_update_time_title ();

        vTaskDelay (10);
    }
}
