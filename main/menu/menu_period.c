#include "menu/menu_period.h"
#include "menu/menu_rotary.h"
#include "ux/rotary_encoder.h"
#include "nvs_fields.h"

extern menu_info_t menu_info;

static period_info_t period_info = {
    .counter = 0,
	.value   = 0,
};

static void _menu_period_save_value   (void);
static void _menu_period_cancel_value (void);
static void _menu_period_set_items    (void);
static void _menu_period_edit_item    (uint16_t position);
static void _menu_period_start_edit   (void);
static void _menu_period_stop_edit    (void);

static void _menu_period_init (void) {
	nvs_load_period_counter(&period_info.value);
    if (!period_info.value) { 
        period_info.value = 1;
        nvs_save_period_counter(period_info.value);
    }
	period_info.counter = period_info.value;
}

static void _menu_period_set_items (void) {
    // snprintf(menu_info.items[0], MENU_ITEM_LEN, "%u sec", 30);
    rotary_encoder_set_position (period_info.counter, period_info.counter);
    menu_info.flags |= MENU_FLAG_EDIT_ITEM | MENU_FLAG_UPDATE_ITEM;
    menu_info.size = 1;
    menu_info.item = 0;
}

static void _menu_period_set_item (void) {
    snprintf(menu_info.items[0], MENU_ITEM_LEN, "Period: %u min", period_info.counter);
}

static void _menu_period_edit_item ( uint16_t position) {
    // period_info.counter += (next - prev) / 4;
    // if (period_info.counter < 0) period_info.counter = 0;
    period_info.counter = position;
    _menu_period_set_item();
}

static void _menu_period_start_edit (void) {
	nvs_load_period_counter(&period_info.value);
	period_info.counter = period_info.value;
    rotary_encoder_set_position (period_info.counter, period_info.counter);
    _menu_period_set_item();
}

static void _menu_period_stop_edit (void) {
	_menu_period_cancel_value();
}

static void _menu_period_save_value (void) {
	period_info.value = period_info.counter;
	nvs_save_period_counter(period_info.value);
}

static void _menu_period_cancel_value(void) {
	period_info.counter = period_info.value;
}

void menu_period_set_items (void) {
    _menu_period_set_items ();
}

void menu_period_edit_item ( uint16_t position) {
    _menu_period_edit_item ( position);
}

void menu_period_start_edit (void) {
    _menu_period_start_edit ();
}

void menu_period_stop_edit (void) {
    _menu_period_stop_edit ();
}

void menu_period_save_value(void) {
	_menu_period_save_value ();
}

void menu_period_cancel_value(void) {
	_menu_period_cancel_value ();
}

void menu_period_get_value (uint16_t *value) {
    *value = period_info.value;
}

void menu_period_init (void) {
    _menu_period_init ();
}
