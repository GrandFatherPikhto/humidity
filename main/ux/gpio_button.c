#include "ux/gpio_button.h"
#include "ux/rotary_encoder.h"
#include "menu/menu_rotary.h"

static QueueHandle_t gpio_evt_queue = NULL;
static QueueHandle_t events_queue   = NULL;
static gpio_button_info_t button_info = { 0 };

#define ROTARY_TASK_NAME      "GPIO Button"
#define ROTARY_BUTTON_GPIO    CONFIG_ROTARY_BUTTON_GPIO
#define STACK_SIZE            CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE
#define ESP_INTR_FLAG_DEFAULT 0
#define KEY_PRESS_TIME        1000000 ///< Время ожидания долгого нажатия кнопки в микросекундах
#define ISR_QUEUE_SIZE        2
#define TAG                   "Button GPIO"

static void IRAM_ATTR _gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void _gpio_button_event_pressed (void) {
    /* Кнопка нажата. Фиксируем факт и время нажатия кнопки */
    button_info.time = esp_timer_get_time ();
    button_info.event_prev = button_info.event;
    button_info.event = GPIO_BUTTON_EVENT_NONE;
}

static void _gpio_button_event_released (void) {
    /* Кнопка отпущена и не удерживалась больше, чем KEY_PRESS_TIME
    * можно инициализировать событие GPIO_BUTTON_SINGLE_CLICK */
    if (button_info.event != GPIO_BUTTON_EVENT_LONG_CLICK) {
        button_info.event  = GPIO_BUTTON_EVENT_SINGLE_CLICK;
        gpio_button_event_t event = GPIO_BUTTON_EVENT_SINGLE_CLICK;
        xQueueSend(events_queue, &event, 0);
        button_info.event_prev = button_info.event;
    }
}

/** Если кнопка нажата, событие LONG_CLICK ещё не настало
 * но время ожидания превышено, инициализируем состояние LONG_CLICK
 */
static void _gpio_button_pressed (void) {
    if (button_info.state    == GPIO_BUTTON_STATE_PRESSED
        && button_info.event == GPIO_BUTTON_EVENT_NONE
        && ((esp_timer_get_time () - button_info.time) >= KEY_PRESS_TIME)) {
            // Кнопка нажата более KEY_PRESS_TIME
            button_info.event_prev    = button_info.event;
            button_info.event         = GPIO_BUTTON_EVENT_LONG_CLICK;
            gpio_button_event_t event = GPIO_BUTTON_EVENT_LONG_CLICK;
            xQueueSend(events_queue, &event, 0);
    }
}

static void _gpio_button_event (void) {
    uint32_t io_num;
    if(xQueueReceive(gpio_evt_queue, &io_num, (TickType_t) 0) == pdTRUE) {
        button_info.state = gpio_get_level(io_num) == 0 ? GPIO_BUTTON_STATE_PRESSED : GPIO_BUTTON_STATE_RELEASED;
        if(button_info.state != button_info.state_prev) {
            /* Кнопка отпущена */
            if (button_info.state == GPIO_BUTTON_STATE_PRESSED) {
                _gpio_button_event_pressed  ();
            } else if (button_info.state == GPIO_BUTTON_STATE_RELEASED){
                _gpio_button_event_released ();
            }
            /* Запоминаем состояние кнопки после его изменения */
            button_info.state_prev = button_info.state;
        }
    } else {
        _gpio_button_pressed ();
    }
}

static void _gpio_button_init (QueueHandle_t queue /* gpio_button_event_t */)
{
    events_queue = queue;
    gpio_config_t io_conf = { 0 };
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = ROTARY_BUTTON_GPIO;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
        //change gpio interrupt type for one pin
    gpio_set_intr_type(ROTARY_BUTTON_GPIO, GPIO_INTR_ANYEDGE);
    
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(ISR_QUEUE_SIZE, sizeof(uint32_t));
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(ROTARY_BUTTON_GPIO, _gpio_isr_handler, (void*) ROTARY_BUTTON_GPIO);
}

void gpio_button_init (QueueHandle_t queue /* gpio_button_event_t */) {
    _gpio_button_init(queue);
}

void gpio_button_event (void) {
    _gpio_button_event ();
}
