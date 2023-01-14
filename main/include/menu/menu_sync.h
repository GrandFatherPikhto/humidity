#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"

#pragma once

void menu_sync_init (void);
void menu_sync_set_items (void);