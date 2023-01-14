#include "menu/menu_date_time.h"
#include "ux/rotary_encoder.h"
#include "ds3231.h"
#include "flags.h"

static struct tm time_info = { 0 };

extern menu_info_t menu_info;

static date_time_info_t date_time_items [] = {
    { .item = "Day:    %d", .value = &time_info.tm_mday, .delim = 31 },
    { .item = "Mon:    %b", .value = &time_info.tm_mon,  .delim = 12 },
    { .item = "Year:   %Y", .value = &time_info.tm_year, .delim = 0  },
    { .item = "Hour:   %H", .value = &time_info.tm_hour, .delim = 24 },
    { .item = "Min:    %M", .value = &time_info.tm_min,  .delim = 60 },
};

static void _get_current_date_time (void) {
    ds3231_get_time (&time_info);
    // time_info.tm_year -= 1900;
} 

static void _menu_date_time_set_item ( uint8_t item) {
    memset(menu_info.items[item], 0, MENU_ITEM_LEN);
    if (menu_info.flags & MENU_FLAG_EDIT_ITEM && menu_info.item == item ) {
        menu_info.items[item][0] = '>';
        menu_info.items[item][1] = ' ';
        strftime(&menu_info.items[item][2], MENU_ITEM_LEN - 2, date_time_items[item].item, &time_info);
        int value = (int)(*(date_time_items[item].value));
        rotary_encoder_set_position (value, value);
    } else {
        strftime(menu_info.items[item], MENU_ITEM_LEN, date_time_items[item].item, &time_info);
    }
} 

static void _menu_date_time_start_edit (void) {
    _get_current_date_time ();
    int value = (int)(*(date_time_items[menu_info.item].value));
    rotary_encoder_set_position(value, value);
    _menu_date_time_set_item ( menu_info.item);
}

static void _menu_date_time_stop_edit (void) {
    _menu_date_time_set_item ( menu_info.item);
}

static void _menu_date_time_set_items (void) {
    _get_current_date_time ();
    for (uint8_t item = 0; item < 5; item++) {
        _menu_date_time_set_item ( item);
    }

    menu_info.size = 5;
    menu_info.item = 0;
}

static void _menu_date_time_change_regime (void) {
    menu_info.flags ^= MENU_FLAG_EDIT_ITEM;
    menu_date_time_set_item ( menu_info.item);
    // _menu_rotary_update_item (menu_info.item, true);
}

static void _menu_date_time_edit_item ( uint16_t position) {
    if (IS_FLAG(menu_info.flags,MENU_FLAG_EDIT_ITEM)) {
        if (date_time_items[menu_info.item].delim) {
            *(date_time_items[menu_info.item].value) 
                = position % (date_time_items[menu_info.item].delim);
        } else {
            *(date_time_items[menu_info.item].value)
                = position;
        }
       
        _menu_date_time_set_item ( menu_info.item);
    }
}

void menu_date_time_set_item ( uint8_t item) {
    _menu_date_time_set_item ( item);
}

void menu_date_time_set_items (void) {
    _menu_date_time_set_items ();
}

void menu_date_time_change_regime (void) {
    _menu_date_time_change_regime();
}

void menu_date_time_edit_item ( uint16_t position) {
    _menu_date_time_edit_item ( position);
}

void menu_date_time_start_edit (void) {
    _menu_date_time_start_edit();
}

void menu_date_time_stop_edit (void) {
    _menu_date_time_stop_edit();
}