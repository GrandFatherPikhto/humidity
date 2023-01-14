/*
 * nvs_fields.c
 *
 *  Created on: 4 янв. 2023 г.
 *      Author: grand
 */




#include "nvs_fields.h"

#define TAG "NVS Fields"

#define EXPERIMENT_COUNTER "EXPERIMENT"
#define RESTART_COUNTER    "RESTART"
#define PERIOD_VALUE       "PERIOD"

esp_err_t _nvs_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if( err != ESP_OK) {
        return err;
    }

    // err = print_what_saved();
    if (err != ESP_OK) printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));

    if(err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t _nvs_load_u32_value(uint32_t *value, const char *name) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    // Read
    err = nvs_get_u32(nvs_handle, name, value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Close
    nvs_close(nvs_handle);

    return ESP_OK;
}

static esp_err_t _nvs_save_u32_value(uint32_t value, const char *name) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;


    err = nvs_set_u32(nvs_handle, name, value);
    if (err != ESP_OK) return err;

    // Commit written value.
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(nvs_handle);

    return ESP_OK;
}

static esp_err_t _nvs_load_u16_value(uint16_t *value, const char *name) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    // Read
    err = nvs_get_u16(nvs_handle, name, value);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Close
    nvs_close(nvs_handle);

    return ESP_OK;
}

static esp_err_t _nvs_save_u16_value(uint16_t value, const char *name) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;


    err = nvs_set_u16(nvs_handle, name, value);
    if (err != ESP_OK) return err;

    // Commit written value.
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(nvs_handle);

    return ESP_OK;
}

esp_err_t nvs_init (void) {
    return _nvs_init ();
}

esp_err_t nvs_load_period_counter(uint16_t *counter) {
    return _nvs_load_u16_value (counter, PERIOD_VALUE);
}

esp_err_t nvs_save_period_counter(uint16_t counter) {
    return _nvs_save_u16_value (counter, PERIOD_VALUE);
}

esp_err_t nvs_load_restart_counter (uint32_t *counter) {
    return _nvs_load_u32_value(counter, RESTART_COUNTER);
}

esp_err_t nvs_save_restart_counter (uint32_t counter) {
    return _nvs_save_u32_value(counter, RESTART_COUNTER);
}

esp_err_t nvs_load_experiment_counter (uint32_t *counter) {
    return _nvs_load_u32_value (counter, EXPERIMENT_COUNTER);
}

esp_err_t nvs_save_experiment_counter (uint32_t counter) {
    return _nvs_save_u32_value(counter, EXPERIMENT_COUNTER);
}
