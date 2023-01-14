#include "menu/menu_sync.h"
#include "menu/menu_rotary.h"
#include "menu/menu_main.h"
#include "wifi_sta.h"

extern menu_info_t menu_info;

static void _menu_sync_init (void) {
    ESP_LOGI(__FUNCTION__, "Start Sync");
    wifi_sta_init   ();
    vTaskDelay     (1000);
    wifi_sta_deinit ();
    ESP_LOGI(__FUNCTION__, "End Sync");
    menu_main_return ();
}

static void _menu_sync_set_items (void) {
    for (uint8_t item = 0; item < MENU_ITEMS; item ++) {
        memset (menu_info.items[item], 0, MENU_ITEM_LEN);
    }

    menu_info.size = 6;
    menu_info.item = 0;
}

void menu_sync_init (void) {
    _menu_sync_init ();
}

void menu_sync_set_items (void) {
    _menu_sync_set_items ();
}