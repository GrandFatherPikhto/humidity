#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include <inttypes.h>

#pragma once

#define ROTARY_PCNT_HIGH_LIMIT  100
#define ROTARY_PCNT_LOW_LIMIT  -100

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*RotaryCallback_t) (void);

typedef enum {
    ROTARY_ENCODER_DIRECTION_NONE = 0x0,
    ROTARY_ENCODER_DIRECTION_CW   = 0x1,
    ROTARY_ENCODER_DIRECTION_CCW  = 0x2,
} rotary_encoder_direction_t;

typedef enum {
    ROTARY_ENCODER_WATCH_POINT_ZERO = 0x0,
    ROTARY_ENCODER_WATCH_POINT_HIGH = 0x1,
    ROTARY_ENCODER_WATCH_POINT_LOW  = 0x2,
} rotary_encoder_watch_point_t;

typedef struct {
    int16_t  position;
    uint16_t positive;
    int8_t   distance;
} rotary_encoder_position_t;

typedef struct {
    rotary_encoder_position_t position;
    int prev_count;
    int pulse_count;
    int event_count;
    rotary_encoder_watch_point_t watch_point;
    rotary_encoder_direction_t direction; ///< 
    uint8_t watch_changed: 1, unused: 7;
} rotary_encoder_info_t;


void rotary_encoder_init                  (QueueHandle_t queue);
void get_rotary_encoder_position          (int *pulse_count);
void rotary_encoder_reset                 (void);
void rotary_encoder_set_position          (int16_t position, uint16_t positive);
void rotary_encoder_event                 (void);

#ifdef __cplusplus
}
#endif
