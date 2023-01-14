#include "menu/menu_rotary.h"
#include "menu/menu_main.h"
#include "menu/menu_titles.h"
#include "ux/gpio_button.h"
#include "ux/rotary_encoder.h"
#include "menu/menu_period.h"
#include "menu/menu_reset.h"
#include "menu/menu_start.h"
#include "ssd1306_oled.h"
#include "flags.h"

#define STACK_SIZE    CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE
#define TASK_NAME     "Menu Rotary"
#define TAG           "Menu Rotary"

#define RESET_AT         0      // Set to a positive non-zero number to reset the position if this value is exceeded
#define ENABLE_HALF_STEPS true  // Set to true to enable tracking of rotary encoder at half step resolution
#define FLIP_DIRECTION    false  // Set to true to reverse the clockwise/counterclockwise sense


menu_info_t menu_info = { 
    .current = 0xF,
    .item    = 0x0,
    .prev    = 0x0,
    .size    = 0x0,
    .page    = 0x0,
    .flags   = MENU_FLAG_NONE
 };

extern QueueHandle_t rotary_encoder_queue;
extern QueueHandle_t gpio_button_queue;

static void _menu_rotary_reset_items     (void);
static void _menu_rotary_update_items    (void);
static void _menu_rotary_update_item     (uint8_t item, bool selected);
static void _menu_rotary_change_position (rotary_encoder_position_t *rotary_encoder_position);

static void _menu_rotary_reset_items(void) {
    for(uint8_t i = 0; i < MENU_ITEMS; i++) {
        memset(menu_info.items[i], 0, MENU_ITEM_LEN);
    }
}

/**
 * @brief  Обновляет все пункты меню на текущей странице (MENU_PAGE_SIZE)
 * @note   Если выставлен menu_info.update == 1, обновляет все пункты меню, принадлежащие текущей странице.
 *         Текущая страница расчитывается из значения menu_info.item. 
 * @retval void
 */
static void _menu_rotary_update_items(void) {
    uint8_t line = 0, item = 0;
    if ((menu_info.flags & MENU_FLAG_UPDATE_ITEMS) == MENU_FLAG_UPDATE_ITEMS) {
        menu_info.page = menu_info.item / MENU_PAGE_SIZE;
        for (uint8_t i = 0; i < MENU_PAGE_SIZE; i++) {
            item = menu_info.page * MENU_PAGE_SIZE + i;
            line = i + MENU_TITLE_ITEMS;
            oled_clear_menu_item (line);
            if (item < menu_info.size && menu_info.items[item][0] != 0) {
                oled_display_menu_item (line, menu_info.items[item], item == menu_info.item);
            }
        }        
        menu_info.prev   = menu_info.item;
        menu_info.flags &= ~MENU_FLAG_UPDATE_ITEMS;
        ESP_LOGD(__FUNCTION__, "Update: %d", (menu_info.flags & MENU_FLAG_UPDATE_ITEMS) == MENU_FLAG_UPDATE_ITEMS);
    }
}


/**
 * @brief  Перерисовка пунтка меню с номером item.
 * @note   Перерисовка пункта меню с номером item активное/не активное (инвертированное/не инвертированное)
 * @param  item: Номер пункта меню из массива menu_info.items 
 * @param  selected: true/false -- инвертированные/не инвертированные цвета
 * @retval void
 */
static void _menu_rotary_update_item (uint8_t item, bool selected) {
    uint8_t line = item - menu_info.page * MENU_PAGE_SIZE + MENU_TITLE_ITEMS;
    oled_clear_menu_item   (line);
    oled_display_menu_item (line, menu_info.items[item], selected);
    ESP_LOGD(TAG, "_update_menu_item(item: %u, line: %u, selected: %d)", item, line, selected);
}

static void _menu_rotary_update_current_item(void) {
    if (IS_FLAG(menu_info.flags, MENU_FLAG_UPDATE_ITEM)) {
        _menu_rotary_update_item (menu_info.item, true);
        RESET_FLAG(menu_info.flags, MENU_FLAG_UPDATE_ITEM);
    }
}

/**
 * @brief  Смена активного пункта меню
 * @note   Меняет один пункт меню на другой. Дизактивирует menu_info.prev и активирует menu_info.item, с учётом текущей страницы
 * @retval void
 */
static void _menu_rotary_change_item (void) {
    uint8_t page = 0;
    if (menu_info.item != menu_info.prev) {
        page = menu_info.item / MENU_PAGE_SIZE;

        if (menu_info.page != page) {
            menu_info.flags |= MENU_FLAG_UPDATE_ITEMS;
            _menu_rotary_update_items();
        } else {
            _menu_rotary_update_item(menu_info.prev, false);
            _menu_rotary_update_item(menu_info.item, true);
            menu_info.prev = menu_info.item;
        }
    }
}

static void _menu_rotary_change_position (rotary_encoder_position_t *rotary_encoder_position) {
    int8_t count = 0;
    if ((menu_info.flags & MENU_FLAG_EDIT_ITEM) == MENU_FLAG_EDIT_ITEM) {
        menu_main_change_position (rotary_encoder_position);
    } else {
        count = rotary_encoder_position->position % menu_info.size;
        if (count < 0) count = menu_info.size + count;
        menu_info.prev = menu_info.item;
        menu_info.item = count;
    }
}

/**
 * @brief  Считывает события rotary encoder и нажатий кнопки rotary encoder
 * @note   
 * @param  *args: ничего не передаём. Просто нужно для совместимости с шаблоном указателя на функцию
 * @retval void
 */
static void _rotary_encoder_event (void) {
    rotary_encoder_position_t rotary_encoder_position = { 0 };

    if (xQueueReceive(rotary_encoder_queue, & rotary_encoder_position, (TickType_t) 0) == pdTRUE)
    {
        ESP_LOGD(__FUNCTION__, "Position: %d, Positive: %u, Distance: %d", 
            rotary_encoder_position.position, 
            rotary_encoder_position.positive, 
            rotary_encoder_position.distance);

        _menu_rotary_change_position      (&rotary_encoder_position);
        _menu_rotary_update_current_item  ();
        _menu_rotary_change_item  ();
        _menu_rotary_update_items ();
    }
}

static void _gpio_button_event (void) {
    gpio_button_event_t gpio_button_event = GPIO_BUTTON_EVENT_NONE;
    if (xQueueReceive(gpio_button_queue, &gpio_button_event, (TickType_t) 0) == pdTRUE) {
        switch (gpio_button_event) {
            case GPIO_BUTTON_EVENT_SINGLE_CLICK:
                menu_main_single_click ();
                _menu_rotary_update_items        ();
                _menu_rotary_update_current_item ();
                menu_main_selected ();
                ESP_LOGD(__FUNCTION__, "Single Click");
            break;
            case GPIO_BUTTON_EVENT_LONG_CLICK:
                menu_main_long_click   ();
                _menu_rotary_update_items        ();
                _menu_rotary_update_current_item ();
                ESP_LOGD(__FUNCTION__, "Long Click");
            break;
            case GPIO_BUTTON_EVENT_NONE:
                ESP_LOGD(__FUNCTION__, "None");
            break;
            default:
                ESP_LOGD(__FUNCTION__, "Unknown Event");
            break;
        }
    }
}

/**
 * @brief  Инициализация меню
 * @note   
 * @retval None
 */
static void _menu_rotary_init (void) {
    menu_start_init           ();
    _menu_rotary_reset_items  ();
    menu_titles_reset         ();
    menu_titles_set_items     ();
    menu_main_select_menu     ();
    menu_titles_update        ();
    _menu_rotary_update_items ();
}

void menu_rotary_init (void) {
    menu_period_init  ();
    _menu_rotary_init ();
}

void menu_rotary_encoder_event (void) {
    _rotary_encoder_event ();
}

void menu_rotary_gpio_button_event (void) {
    _gpio_button_event ();
}

void menu_rotary_update_items (void) {
    _menu_rotary_update_items ();
}
