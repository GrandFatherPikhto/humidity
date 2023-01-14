#include "menu/menu_start.h"
#include "nvs_fields.h"

extern menu_info_t menu_info;

static uint32_t experiment = 0;

static void _menu_start_init (void) {
    nvs_load_experiment_counter (&experiment);
}

static void _menu_start_increment (void) {
    esp_err_t res = ESP_OK;
    char error[0x100];
    res = nvs_load_experiment_counter (&experiment);
    if (res != ESP_OK) {
        esp_err_to_name_r (res, error, 0x100);
        ESP_LOGE(__FUNCTION__, "Error: %d, %s", res, error);
    }
    experiment ++;
    res = nvs_save_experiment_counter (experiment);
    if (res != ESP_OK) {
        esp_err_to_name_r (res, error, 0x100);
        ESP_LOGE(__FUNCTION__, "Error: %d, %s", res, error);
    }
}

static void _menu_start_set_items (void) {
    _menu_start_increment ();
    for (uint8_t item = 0; item < 8; item++) {
        // snprintf(menu_info.items[item], MENU_ITEM_LEN, "%u T:%2.1f H:%2.1f", item + 1, 20.1, 22.2);
        memset (menu_info.items[item], 0, MENU_ITEM_LEN);
    }
    menu_info.size = 8;
    menu_info.item = 0;
}

void menu_start_set_items (void) {
    _menu_start_set_items ();
}

void menu_start_init (void) {
    _menu_start_init ();
}

void menu_start_get_counter (uint32_t *counter) {
    *counter = experiment;
}