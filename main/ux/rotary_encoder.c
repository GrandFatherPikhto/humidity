#include "ux/rotary_encoder.h"
#include "menu/menu_main.h"

#define ROTARY_EC11_GPIO_A CONFIG_EC11_GPIO_S1
#define ROTARY_EC11_GPIO_B CONFIG_EC11_GPIO_S2

#define ROTARY_ENCODER_EVENT_QUEUE_LENGTH 0x10

#define ESP_INTR_FLAG_DEFAULT 0

#define TICKS_TO_WAIT 10
#define REACH_QUEUE_SIZE 2

static rotary_encoder_info_t rotary_encoder_info = {
    .position    = { 0 },
    .pulse_count = 0,
    .prev_count  = 0,
    .event_count = 0,
    .direction = ROTARY_ENCODER_DIRECTION_NONE
};

static QueueHandle_t reach_queue    = NULL;
static QueueHandle_t events_queue   = NULL;
static pcnt_unit_handle_t pcnt_unit = NULL;

static bool _pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx) {
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    
    return (high_task_wakeup == pdTRUE);
    // return true;
}

void get_rotary_encoder_position(int *pulse_count) {
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, pulse_count));
}

static void _rotary_encoder_set_position (int16_t position, uint16_t positive) {
    rotary_encoder_info.position.position    = position;
    rotary_encoder_info.position.positive    = positive;
    rotary_encoder_info.position.distance    = 0;
    rotary_encoder_info.watch_point = ROTARY_ENCODER_WATCH_POINT_ZERO;
    rotary_encoder_info.prev_count  = 0;
    rotary_encoder_info.pulse_count = 0;
    rotary_encoder_info.event_count = 0;
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
}

void rotary_encoder_reset (void) {
    _rotary_encoder_set_position (0, 0);
}

void rotary_encoder_set_position (int16_t position, uint16_t positive) {
    _rotary_encoder_set_position (position, positive);
}

static void _rotary_encoder_watch_point (void) {
    ESP_LOGD(__FUNCTION__, "Watch point event, count: %d", rotary_encoder_info.event_count);
    if (rotary_encoder_info.event_count == ROTARY_PCNT_LOW_LIMIT) {
        rotary_encoder_info.watch_point = ROTARY_ENCODER_WATCH_POINT_LOW;
    } else if (rotary_encoder_info.event_count == ROTARY_PCNT_HIGH_LIMIT) {
        rotary_encoder_info.watch_point = ROTARY_ENCODER_WATCH_POINT_HIGH;
    } else if (rotary_encoder_info.event_count == 0) {
        // rotary_encoder_info.watch_point = ROTARY_ENCODER_WATCH_POINT_ZERO;
    }
}

static void _rotary_encoder_log_position(void) {
        ESP_LOGD(__FUNCTION__, "Pulse Count: %d, Prev Count: %d, Watch Point: %d", 
        rotary_encoder_info.pulse_count, 
        rotary_encoder_info.prev_count, 
        rotary_encoder_info.watch_point);
}

static void _rotary_encoder_watch_point_zero (void) {
    rotary_encoder_info.position.distance = (rotary_encoder_info.pulse_count - rotary_encoder_info.prev_count) / 4;
    rotary_encoder_info.position.position += rotary_encoder_info.position.distance;
    if (rotary_encoder_info.position.positive + rotary_encoder_info.position.distance >= 0) {
        rotary_encoder_info.position.positive = rotary_encoder_info.position.positive + rotary_encoder_info.position.distance;
    } else {
        rotary_encoder_info.position.positive = 0;
    }
}

static void _rotary_encoder_watch_point_high (void) {
    rotary_encoder_info.position.distance = rotary_encoder_info.pulse_count / 4;
    rotary_encoder_info.position.position += rotary_encoder_info.position.distance;
    rotary_encoder_info.position.positive += rotary_encoder_info.position.distance; 
}

static void _rotary_encoder_watch_point_low (void) {
    rotary_encoder_info.position.distance    = rotary_encoder_info.pulse_count / 4;
    rotary_encoder_info.position.position += rotary_encoder_info.position.distance;
    if (rotary_encoder_info.position.positive + rotary_encoder_info.position.distance >= 0) {
        rotary_encoder_info.position.positive = rotary_encoder_info.position.positive + rotary_encoder_info.position.distance;
    } else {
        rotary_encoder_info.position.positive = 0;
    }
}

/**
 * @brief  Рассчёт текущей позиции исходя из пройденных точек LOW, HIGH
 * @note   Рассчёт текущей позиции, перехват событий перехода ключевых точек, полученных при перехвате события
 * @param  void 
 * @retval None
 */
static void _rotary_encoder_calc_position(void) {
    
    _rotary_encoder_log_position();

    if (rotary_encoder_info.pulse_count != rotary_encoder_info.prev_count) {
        switch (rotary_encoder_info.watch_point)
        {
        case ROTARY_ENCODER_WATCH_POINT_ZERO:
            _rotary_encoder_watch_point_zero ();
            break;
        case ROTARY_ENCODER_WATCH_POINT_HIGH:
            _rotary_encoder_watch_point_high ();
            break;
        case ROTARY_ENCODER_WATCH_POINT_LOW:
            _rotary_encoder_watch_point_low  ();
            break;
        default:
            break;
        }

        rotary_encoder_info.prev_count = rotary_encoder_info.pulse_count;
        rotary_encoder_info.watch_point = ROTARY_ENCODER_WATCH_POINT_ZERO;

        if (events_queue != NULL) {
            rotary_encoder_position_t rotary_encoder_position = {
                .position = rotary_encoder_info.position.position,
                .positive = rotary_encoder_info.position.positive,
                .distance = rotary_encoder_info.position.distance,
            };

            xQueueSend (events_queue, &rotary_encoder_position, ( TickType_t ) 0);
        }
    }
}

static void _rotary_encoder_event (void) {
    // rotary_encoder_info_t *rotary_encoder_info = (rotary_encoder_info_t *) arg;
    if (xQueueReceive(reach_queue, &rotary_encoder_info.event_count, (TickType_t) 0) == pdTRUE) {
       _rotary_encoder_watch_point   ();
    } else {
        ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, & rotary_encoder_info.pulse_count));
        _rotary_encoder_calc_position ();
    }
}

static void _rotary_encoder_init (QueueHandle_t queue) {
    ESP_LOGD(__FUNCTION__, "install pcnt unit");
    
    events_queue = queue;

    pcnt_unit_config_t unit_config = {
        .high_limit = ROTARY_PCNT_HIGH_LIMIT,
        .low_limit  = ROTARY_PCNT_LOW_LIMIT,
    };
    // pcnt_unit_handle_t pcnt_unit = NULL;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    ESP_LOGD(__FUNCTION__, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };

    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    ESP_LOGD(__FUNCTION__, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ROTARY_EC11_GPIO_A,
        .level_gpio_num = ROTARY_EC11_GPIO_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num  = ROTARY_EC11_GPIO_B,
        .level_gpio_num = ROTARY_EC11_GPIO_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_LOGD(__FUNCTION__, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_LOGD(__FUNCTION__, "add watch points and register callbacks");
    int watch_points[] = {ROTARY_PCNT_LOW_LIMIT, -50, 0, 50, ROTARY_PCNT_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
    }

    pcnt_event_callbacks_t cbs = {
        .on_reach = _pcnt_on_reach,
    };

    reach_queue = xQueueCreate(REACH_QUEUE_SIZE, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, reach_queue));


    ESP_LOGD(__FUNCTION__, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_LOGD(__FUNCTION__, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGD(__FUNCTION__, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    ESP_LOGD(__FUNCTION__, "EC11 A: [%d]", ROTARY_EC11_GPIO_A);    
    ESP_LOGD(__FUNCTION__, "EC11 B: [%d]", ROTARY_EC11_GPIO_B);
}

void rotary_encoder_init(QueueHandle_t queue) {
    _rotary_encoder_init(queue);
}

void rotary_encoder_event (void) {
    _rotary_encoder_event ();
}
