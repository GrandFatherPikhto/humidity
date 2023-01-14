#include "menu/menu_reset.h"
#include "nvs_fields.h"

extern menu_info_t menu_info;

static void _menu_reset_set_items(void) {
    strncpy(menu_info.items[0], "Cancel",  MENU_ITEM_LEN);
    strncpy(menu_info.items[1], "Reset",   MENU_ITEM_LEN);
    menu_info.size = 2;
    menu_info.item = 0;
}

static void _menu_reset_single_click (void) {
    if (menu_info.item == 1) {
        nvs_save_experiment_counter (0);
        nvs_save_restart_counter    (0);
    }
}

void menu_reset_set_items(void) {
    _menu_reset_set_items();
}

void menu_reset_single_click (void) {
    _menu_reset_single_click ();
}
