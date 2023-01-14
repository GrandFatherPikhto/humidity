
#include <inttypes.h>
#include <string.h>
#include "menu/menu_rotary.h"
#include "ux/rotary_encoder.h"

#pragma once

#define MENU_MAIN_LEN 5

typedef enum {
    MENU_MAIN      = 0x0F, ///< Главное меню
    MENU_START     = 0x00, ///< Старт считывания данных
    MENU_RESET     = 0x01, ///< Инкремент счётчика экспериментов (влияет на имя файла, куда записываются данные)
    MENU_PERIOD    = 0x02, ///< Меню редактирования периода считывания данных
    MENU_DATE_TIME = 0x03, ///< Меню редактирования даты/времени
    MENU_SYNC      = 0x04, ///< Меня синхронизации даты/времени и отправки данных на сервер
} menu_main_item_t;

typedef enum {
    MENU_MAIN_FLAG_NONE   = 0x00,
    MENU_MAIN_FLAG_RESET  = 0x01,
    MENU_MAIN_FLAG_UPDATE = 0x02,
} menu_main_flag_t;

void menu_main_get_item        (char **title, uint8_t item);
void menu_main_set_items       (void);
void menu_main_reset           (uint8_t item);
void menu_main_select_menu     (void);
void menu_main_single_click    (void);
void menu_main_long_click      (void);
void menu_main_change_position (rotary_encoder_position_t *rotary_encoder_position);
void menu_main_selected        (void);
void menu_main_return          (void);