#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <inttypes.h>
#include <string.h>
#include <time.h>

#pragma once

#include "menu/menu_rotary.h"

#define MENU_DATE_TIME_PATTERN_LEN 0x0B
#define MENU_DATE_TIME_LEN 5

typedef struct {
    char item[MENU_DATE_TIME_PATTERN_LEN];
    int *value;
    uint8_t delim;
} date_time_info_t;

void menu_date_time_set_item      ( uint8_t item);
void menu_date_time_set_items     (void);
void menu_date_time_change_regime (void);
void menu_date_time_edit_item     ( uint16_t position);
void menu_date_time_start_edit    (void);
void menu_date_time_stop_edit     (void);