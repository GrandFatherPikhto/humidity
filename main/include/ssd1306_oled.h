#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"

#pragma once

#define DISPLAY_LINES 4
#define MENU_DISPLAY_LEN 0x10

typedef struct {
    uint8_t item:4, page: 4;
} MENU_ITEM;


void    oled_init              (void);
void    oled_close             (void);
void    oled_display_menu_item (uint8_t item, char *title, bool selected);
void    oled_clear_menu_item   (uint8_t item);
void    oled_clear_screen      (void);
uint8_t oled_get_pages         (void);
