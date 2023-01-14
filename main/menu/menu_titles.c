#include "menu/menu_titles.h"
#include "menu/menu_rotary.h"
#include "menu/menu_main.h"
#include "ssd1306_oled.h"
#include "ds3231.h"

#define TIME_UPDATE 6000000

static struct tm time_info;
static int64_t current_time;
extern menu_info_t menu_info;

static void _menu_titles_update_time (void) {
    ds3231_get_time (&time_info);
    strftime(menu_info.titles[0], 
        MENU_ITEM_LEN, 
        "%d %b %y %H:%M",
        &time_info
    );
}

static void _menu_titles_update_time_title (void) {
    if (esp_timer_get_time () - current_time >= TIME_UPDATE)  {
        _menu_titles_update_time ();
        oled_clear_menu_item     (0);
        oled_display_menu_item   (0, menu_info.titles[0], false);
        current_time = esp_timer_get_time ();
    }
}

static void _menu_titles_set_items (void) {
    char *out = NULL;
    _menu_titles_update_time ();
    if (menu_info.current == MENU_MAIN) {
        strncpy(menu_info.titles[1], "Main", MENU_ITEM_LEN);
    } else {
        menu_main_get_item(&out, menu_info.current);
        if (out != NULL) {
            strncpy(menu_info.titles[1], out, MENU_ITEM_LEN);
        }
    }
}

static void _menu_titles_reset (void) {
    for(uint8_t i = 0; i < MENU_TITLE_ITEMS; i++) {
        memset(menu_info.titles, 0, MENU_ITEM_LEN);
    }
}

static void _menu_titles_update (void) {
    for (uint8_t i = 0; i < MENU_TITLE_ITEMS && menu_info.titles[i][0] != 0; i++) {
        oled_clear_menu_item(i);
        oled_display_menu_item(i, menu_info.titles[i], false);
    }
}

void menu_titles_set_items (void) {
    _menu_titles_set_items ();
}

void menu_titles_reset (void) {
    _menu_titles_reset ();
}

void menu_titles_update (void) {
    _menu_titles_update ();
}

void menu_titles_update_time_title (void) {
    _menu_titles_update_time_title ();
}
