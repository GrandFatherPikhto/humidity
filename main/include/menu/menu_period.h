#include <inttypes.h>
#include <string.h>
#include "menu/menu_rotary.h"

#pragma once

typedef struct {
    int16_t  counter;
    uint16_t value;
} period_info_t;

void menu_period_set_items    (void);
void menu_period_edit_item    (uint16_t position);
void menu_period_start_edit   (void);
void menu_period_stop_edit    (void);
void menu_period_save_value   (void);
void menu_period_cancel_value (void);
void menu_period_get_value    (uint16_t *value);
void menu_period_init         (void);
