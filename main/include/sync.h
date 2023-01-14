#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_attr.h"
#include "esp_sleep.h"

#include <time.h>
#include <sys/time.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#pragma once

void sync_clock (void);
void get_clock  (void);
