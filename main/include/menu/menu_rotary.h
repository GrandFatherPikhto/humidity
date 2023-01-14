#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"

#include "ux/gpio_button.h"
#include "ux/rotary_encoder.h"

#pragma once

#define MENU_ITEMS       8
#define MENU_TITLE_ITEMS 2
#define MENU_PAGE_SIZE   6
#define MENU_ITEM_LEN    0x12

typedef enum {
    MENU_FLAG_NONE            = 0x00, ///< Ничего не делать           
    MENU_FLAG_UPDATE_ITEM     = 0x01, ///< Обновить одну строку меню (.item)
    MENU_FLAG_UPDATE_ITEMS    = 0x02, ///< Обновить все строки меню
    MENU_FLAG_EDIT_ITEM       = 0x04, ///< Редактировать текущую строку меню (.item)
    MENU_FLAG_UPDATE_TITLES   = 0x08, ///< Обновить все строки заголовка
    MENU_FLAG_UPDATE_TITLE    = 0x10, ///< Обновить строку заголовка с текущим меню [1]
    MENU_FLAG_UPDATE_DATETIME = 0x20, ///< Обновить время/дату
} menu_flag_t;

typedef struct {
    uint8_t current;   ///< Текущее выбранное меню
    uint8_t prev;      ///< Предыдущий выбранный пункт меню
    uint8_t item;      ///< Текущий выбранный пункт меню
    uint8_t size;      ///< Количество активных пунктов меню
    uint8_t page;      ///< Текущая страница
    menu_flag_t flags; ///< Флаги состояний
    char items[MENU_ITEMS][MENU_ITEM_LEN]; ///< Элементы меню
    char titles [MENU_TITLE_ITEMS][MENU_ITEM_LEN]; ///< Элементы заголовка
} menu_info_t;

typedef struct {
    rotary_encoder_info_t rotary;
    gpio_button_info_t button;
} menu_rotary_event_t;

void menu_rotary_init              (void);
void menu_rotary_encoder_event     (void);
void menu_rotary_gpio_button_event (void);
void menu_rotary_update_items      (void);
