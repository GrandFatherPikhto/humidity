#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <string.h>
#include "esp_timer.h"

#pragma once

typedef void (*gpio_button_callback) (void);

typedef enum {
    GPIO_BUTTON_STATE_NONE     = 0x00,
    GPIO_BUTTON_STATE_RELEASED = 0x01,
    GPIO_BUTTON_STATE_PRESSED  = 0x02,
} gpio_button_state_t;

typedef enum {
    GPIO_BUTTON_EVENT_NONE         = 0x00,
    GPIO_BUTTON_EVENT_SINGLE_CLICK = 0x04,
    GPIO_BUTTON_EVENT_LONG_CLICK   = 0x08,
} gpio_button_event_t;

typedef struct {
    int64_t time;
    gpio_button_state_t state;
    gpio_button_state_t state_prev;
    gpio_button_event_t event;
    gpio_button_event_t event_prev;
} gpio_button_info_t;


void gpio_button_init  (QueueHandle_t queue /* menu_task_info_t */);
void gpio_button_event (void);
