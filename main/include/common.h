#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "sdkconfig.h"

#include "ux/gpio_button.h"
#include "ux/rotary_encoder.h"

#include "menu/menu_main.h"
#include "menu/menu_rotary.h"
#include "menu/menu_titles.h"
#include "menu/menu_period.h"
#include "menu/menu_start.h"
#include "menu/menu_sync.h"
#include "menu/menu_date_time.h"
#include "menu/menu_reset.h"

#include "i2cdev.h"
#include "wifi_sta.h"
#include "sd_espi.h"
#include "ssd1306_oled.h"
#include "humidity.h"
#include "nvs_fields.h"

#pragma once

