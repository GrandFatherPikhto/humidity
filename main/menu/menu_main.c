#include "ux/rotary_encoder.h"
#include "menu/menu_main.h"
#include "menu/menu_rotary.h"
#include "menu/menu_start.h"
#include "menu/menu_main.h"
#include "menu/menu_period.h"
#include "menu/menu_reset.h"
#include "menu/menu_date_time.h"
#include "menu/menu_sync.h"
#include "menu/menu_titles.h"
#include "flags.h"

extern menu_info_t menu_info;

static char *menu_items[MENU_ITEM_LEN] = {
    "Start",
    "New Experiment",
    "Period",
    "Date/Time",
    "Sync",
    0
    };

static void _menu_main_set_items(void);
static void _menu_main_reset(menu_main_item_t item);
static void _menu_main_get_item(char **title, uint8_t item);
static void _menu_main_select_menu(void);
static void _menu_main_single_click(void);
static void _menu_main_long_click(void);
static void _menu_main_change_position(rotary_encoder_position_t *rotary_position);
static void _menu_main_reset_edit(void);
static void _menu_main_start_edit(void);
static void _menu_main_stop_edit(void);
static void _menu_main_set_update (menu_main_flag_t *flags);

static void _menu_main_set_items(void)
{
    uint8_t cnt = 0;
    for (cnt = 0; cnt < MENU_ITEMS && menu_items[cnt] != 0; cnt++)
    {
        strncpy(menu_info.items[cnt], menu_items[cnt], MENU_ITEM_LEN);
    }

    menu_info.size = cnt;
    menu_info.item = 0;
    menu_info.prev = 0;
    menu_info.flags |= MENU_FLAG_UPDATE_ITEMS;
}

static void _menu_main_reset(menu_main_item_t item)
{
    menu_info.current = item;
    menu_info.item = menu_info.prev;
    menu_info.prev = 0;
    menu_info.flags = MENU_FLAG_NONE;
    rotary_encoder_reset();
}

static void _menu_main_get_item(char **title, uint8_t item)
{
    if (item < MENU_MAIN_LEN)
    {
        *title = menu_items[item];
    }
}

static void _menu_main_return (void) {
    menu_main_flag_t flags = MENU_MAIN_FLAG_RESET | MENU_MAIN_FLAG_UPDATE;
    menu_info.current = MENU_MAIN;
    menu_info.item = 0;
    menu_titles_reset     ();
    menu_titles_set_items ();
    menu_titles_update    ();
    _menu_main_set_items  ();
    _menu_main_set_update (&flags);
    menu_rotary_update_items ();   
}

static void _menu_main_change_position(rotary_encoder_position_t *rotary_position)
{
    if ((menu_info.flags & MENU_FLAG_EDIT_ITEM) == MENU_FLAG_EDIT_ITEM)
    {
        switch (menu_info.current)
        {
        case MENU_PERIOD:
            menu_period_edit_item(rotary_position->positive);
            menu_info.flags |= MENU_FLAG_UPDATE_ITEM;
            break;
        case MENU_DATE_TIME:
            menu_date_time_edit_item(rotary_position->positive);
            menu_info.flags |= MENU_FLAG_UPDATE_ITEM;
            break;
        default:
            break;
        }
    }
}

static void _menu_main_select_menu(void)
{
    menu_info.flags |= MENU_FLAG_UPDATE_ITEMS;

    switch (menu_info.current)
    {
    case MENU_MAIN:
        _menu_main_set_items();
        break;
    case MENU_START:
        menu_start_set_items();
        break;
    case MENU_RESET:
        menu_reset_set_items();
        break;
    case MENU_PERIOD:
        SET_FLAG(menu_info.flags, MENU_FLAG_EDIT_ITEM);
        _menu_main_start_edit();
        menu_period_set_items();
        break;
    case MENU_DATE_TIME:
        menu_date_time_set_items();
        break;
    case MENU_SYNC:
        menu_sync_set_items ();
        // menu_sync_init ();
        break;
    default:
        break;
    }

    menu_titles_set_items ();
    menu_titles_update    ();
}

static void _menu_main_start_edit(void)
{
    switch (menu_info.current)
    {
    case MENU_PERIOD:
        menu_period_start_edit();
        break;
    case MENU_DATE_TIME:
        menu_date_time_start_edit();
        break;
    default:
        break;
    }
}

static void _menu_main_stop_edit(void)
{
    switch (menu_info.current)
    {
    case MENU_PERIOD:
        menu_period_stop_edit();
        break;
    case MENU_DATE_TIME:
        menu_date_time_stop_edit();
        break;
    default:
        break;
    }
}

static void _menu_main_reset_edit(void)
{
    BLINK_FLAG(menu_info.flags, MENU_FLAG_EDIT_ITEM);
    SET_FLAG(menu_info.flags, MENU_FLAG_UPDATE_ITEM);
    if (IS_FLAG(menu_info.flags, MENU_FLAG_EDIT_ITEM))
    {
        _menu_main_start_edit();
    }
    else
    {
        _menu_main_stop_edit();
    }

    SET_FLAG(menu_info.flags, MENU_FLAG_UPDATE_ITEM);
}

static void _menu_main_set_update(menu_main_flag_t *flags)
{
    SET_FLAG(*flags, MENU_MAIN_FLAG_RESET);
    SET_FLAG(*flags, MENU_MAIN_FLAG_UPDATE);
}

static void _menu_main_reset_update(menu_main_flag_t *flags)
{
    RESET_FLAG(*flags, MENU_MAIN_FLAG_RESET);
    RESET_FLAG(*flags, MENU_MAIN_FLAG_UPDATE);
}

static void _menu_main_selected (void) {
    switch (menu_info.current)
    {
    case MENU_SYNC:
        menu_sync_init ();        
        break;
    
    default:
        break;
    }
}

static void _menu_main_single_click(void)
{
    ESP_LOGD(__FUNCTION__, "Click %u", menu_info.current);
    uint8_t next = MENU_MAIN;
    menu_main_flag_t flags = MENU_MAIN_FLAG_NONE;

    switch (menu_info.current)
    {
    case MENU_MAIN:
        next = menu_info.item;
        _menu_main_set_update(&flags);
        break;
    case MENU_START:
        ESP_LOGD(__FUNCTION__, "Menu Start");
        _menu_main_reset_update(&flags);
        break;
    case MENU_RESET:
        menu_reset_single_click ();
        _menu_main_set_update(&flags);
        break;
    case MENU_PERIOD:
        _menu_main_reset_edit();
        _menu_main_set_update(&flags);
        break;
    case MENU_DATE_TIME:
        _menu_main_reset_edit();
        _menu_main_reset_update(&flags);
        break;
    case MENU_SYNC:
        
        break;
    default:
        break;
    }

    if (IS_FLAG(flags, MENU_MAIN_FLAG_RESET))
    {
        _menu_main_reset(next);
    }

    if (IS_FLAG(flags, MENU_MAIN_FLAG_UPDATE))
    {
        _menu_main_select_menu();
    }
}

static void _menu_main_long_click(void)
{
    ESP_LOGD(__FUNCTION__, "Long Click");
    uint8_t next = MENU_MAIN;
    menu_main_flag_t flags = MENU_MAIN_FLAG_NONE;
    switch (menu_info.current)
    {
    case MENU_MAIN:
        _menu_main_set_update(&flags);
        break;
    case MENU_START:
        _menu_main_set_update(&flags);
        break;
    case MENU_RESET:
        _menu_main_set_update(&flags);
        break;
    case MENU_PERIOD:
        menu_period_save_value ();
        _menu_main_set_update(&flags);
        break;
    case MENU_DATE_TIME:
        _menu_main_set_update(&flags);
        break;
    case MENU_SYNC:
        _menu_main_set_items ();
        _menu_main_set_update(&flags);
        menu_rotary_update_items ();   
        break;
    default:
        break;
    }

    if (IS_FLAG(flags, MENU_MAIN_FLAG_RESET))
    {
        _menu_main_reset(next);
    }

    if (IS_FLAG(flags, MENU_MAIN_FLAG_UPDATE))
    {
        _menu_main_select_menu();
    }
}

void menu_main_get_item(char **title, uint8_t item)
{
    _menu_main_get_item(title, item);
}

void menu_main_set_items(void)
{
    _menu_main_set_items();
}

void menu_main_reset(uint8_t item)
{
    _menu_main_reset(item);
}

void menu_main_select_menu(void)
{
    _menu_main_select_menu();
}

void menu_main_single_click(void)
{
    _menu_main_single_click();
}

void menu_main_long_click(void)
{
    _menu_main_long_click();
}

void menu_main_change_position(rotary_encoder_position_t *rotary_position)
{
    _menu_main_change_position(rotary_position);
}

void menu_main_selected (void) {
    _menu_main_selected ();
}

void menu_main_return (void) {
    _menu_main_return ();
}