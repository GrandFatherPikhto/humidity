/*
 * nvs_fields.h
 *
 *  Created on: 4 янв. 2023 г.
 *      Author: grand
 */

#ifndef MAIN_INCLUDE_NVS_FIELDS_H_
#define MAIN_INCLUDE_NVS_FIELDS_H_

#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#define STORAGE_NAMESPACE "storage"

esp_err_t nvs_init                    (void);

esp_err_t nvs_load_period_counter     (uint16_t *counter);
esp_err_t nvs_save_period_counter     (uint16_t  counter);

esp_err_t nvs_load_restart_counter    (uint32_t  *counter);
esp_err_t nvs_save_restart_counter    (uint32_t   counter);

esp_err_t nvs_load_experiment_counter (uint32_t *counter);
esp_err_t nvs_save_experiment_counter (uint32_t  counter);

#endif /* MAIN_INCLUDE_NVS_FIELDS_H_ */
