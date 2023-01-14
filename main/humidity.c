#include "humidity.h"
#include "menu/menu_period.h"
#include "menu/menu_start.h"
#include "menu/menu_main.h"
#include "i2cdev.h"
#include "ds3231.h"
#include "sd_espi.h"
#include "flags.h"

#define TCA9548A_RESET_GPIO CONFIG_TCA9548A_RESET_GPIO
#define STACK_SIZE    CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE
#define TASK_NAME     "Humidity"
#define TAG           "Humidity"
#define QUEUE_LENGTH  0x10
#define TASK_CORE     1

extern menu_info_t menu_info;

humidity_data_t humidity_data[NUM_SENSORS] = { 0 };
static humidity_info_t humidity_info = {
    .write   = 1,
    .log     = 0,
    .display = 1,
    .json    = 1,
};

static uint64_t time_counter = 0;
static uint16_t period       = 30;
static uint32_t point        = 0;
static uint32_t experiment   = 0;

/**
 * @brief Read a sequence of bytes from a HIH6020 sensor registers
 */
static esp_err_t _hih6020_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(
        I2C_MASTER_NUM, 
        HIH6020_SENSOR_ADDR, 
        &reg_addr, 
        1, 
        data, 
        len, 
        I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a HIH6020 sensor register
 */
static esp_err_t _hih6020_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(
        I2C_MASTER_NUM, 
        HIH6020_SENSOR_ADDR, 
        write_buf, 
        sizeof(write_buf), 
        I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief Write a byte to a HIH6020 sensor register
 */
static esp_err_t _tca9548a_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(
        I2C_MASTER_NUM, 
        TCA9548A_ADDR, 
        write_buf, 
        sizeof(write_buf), 
        I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief Calc Humidity
 */
static float _calc_humidity(const uint8_t *data) {
    uint16_t rawHumidity = ((((uint16_t)data[0] & 0x3F) << 8) | (uint16_t)data[1]);
    return (float)(((float)rawHumidity * 100.0f) / 16382.0f);
}

/**
 * @brief Calc Temperature
 */
static float _calc_temperature(const uint8_t *data) {
    uint16_t rawTemp = ((uint16_t)data[2] << 6) | ((uint16_t)data[3] >> 2);
    return (float)(((float)rawTemp * 165.0f) / 16382.0f) - 40.0f;
}

/**
 * @brief Calc Status
 */
static uint8_t _calc_status(const uint8_t *data) {
    return (data[0] >> 6);
}

/**
 * @brief Переводит I2C данные в Human Readable (status, temperature C, Humidity %)
 **/
static void _create_humidity_data(uint8_t sensor, const uint8_t *data) {
    humidity_data[sensor].status      = _calc_status(data);
    humidity_data[sensor].temperature = _calc_temperature(data);
    humidity_data[sensor].humidity    = _calc_humidity(data);
}

/**
 * @brief Распечатка данных в формате ESP_LOGI
 **/
static void _humidity_print_log_sensor(uint8_t sensor) {
    char datetime[0x20];
    strftime(datetime, 0x20, "%x %X", &humidity_data[sensor].time);
    menu_start_get_counter (&experiment);
    ESP_LOGI("Sensor",
        "Datetime: %s; Timestamp: %"PRId64"; Experiment: %"PRId32"; Point: %"PRIu32"; Sensor: %u; Status: 0x%02X; Temperature: %3.2f°C; Humidity: %3.2f%%;",
        datetime,
        mktime(&humidity_data[sensor].time),
        experiment,
        point, 
        sensor, 
        humidity_data[sensor].status,
        humidity_data[sensor].temperature, 
        humidity_data[sensor].humidity);        
}

static void _humidity_print_json_sensor (uint8_t sensor) {
    menu_start_get_counter (&experiment);
    printf ("JSON:\t{\"timestamp\": \"%"PRId64"\", \"experiment\": \"%"PRIu32"\", \"point\": \"%"PRIu32"\", \"sensor\": \"%u\", \"status\": \"0x%02X\", \"temperature\": \"%3.2f\", \"humidity\": \"%3.2f\"}\n",
        mktime(&humidity_data[sensor].time),
        experiment,
        point,
        sensor,
        humidity_data[sensor].status,
        humidity_data[sensor].temperature,
        humidity_data[sensor].humidity
    );
}

static void _humidity_set_error (uint8_t sensor, uint8_t status) {
    humidity_data[sensor].status      = status;
    humidity_data[sensor].temperature = 0.0;
    humidity_data[sensor].humidity    = 0.0;
}

static void _humidity_read_sensor (uint8_t sensor /*, humidity_data_t *humidity_data */) {
    uint8_t data[HIH6020_DATA_SIZE] = { 0 };

    esp_err_t err = 0;

    err = _tca9548a_register_write_byte(0, 1 << sensor);

    err = _hih6020_register_write_byte(0xFF, 1);

    vTaskDelay(HIH6020_CONVERSION_DELAY / portTICK_PERIOD_MS);

    err = _hih6020_register_read(0, data, 4);
    if(err == ESP_OK) {
        _create_humidity_data(sensor, data);        
    } else {
        _humidity_set_error(sensor, HIH6020_STATUS_READ_ERROR);
    }

    ds3231_get_time(&(humidity_data[sensor].time));
}

static void _humidity_print_log_data (void) {
    for (uint8_t sensor = 0; sensor < NUM_SENSORS; sensor ++) {
        _humidity_print_log_sensor(sensor);
    }
}

static void _humidity_print_json_data (void) {
    for (uint8_t sensor = 0; sensor < NUM_SENSORS; sensor ++) {
        _humidity_print_json_sensor(sensor);
    }
}

static void _humidity_display_data (void) {
    if (menu_info.current == MENU_START) {
        for (uint8_t sensor = 0; sensor < NUM_SENSORS; sensor ++) {
            if (humidity_data[sensor].status == 1) {
            snprintf(menu_info.items[sensor], MENU_ITEM_LEN, 
                "%u %02.1fC %02.1f%%", 
                sensor + 1, 
                humidity_data[sensor].temperature,
                humidity_data[sensor].humidity);
            } else {
                snprintf(menu_info.items[sensor], MENU_ITEM_LEN,
                    "%u Error: 0x%02X",
                    sensor + 1,
                    humidity_data[sensor].status
                );
            }
        }
        SET_FLAG(menu_info.flags, MENU_FLAG_UPDATE_ITEMS);
        menu_rotary_update_items ();
    }
}

/**
 * @brief  
 * @note   
 * @param  *: 
 * @retval None
 */
static void _humidity_read_data (void) {
    if (esp_timer_get_time() - time_counter > period * 60000000) {
    // if (esp_timer_get_time() - time_counter > 5000000) {

        for(uint8_t sensor = 0; sensor < NUM_SENSORS; sensor++) {
            _humidity_read_sensor (sensor);
        }

        menu_start_get_counter (&experiment);
        time_counter = esp_timer_get_time ();
        menu_period_get_value (&period);

        if (humidity_info.write) {
            sd_espi_init ();
            sd_espi_write_humidity_data (experiment, point);
            sd_espi_deinit ();
        }

        if (humidity_info.log) {
            _humidity_print_log_data ();
        }

        if (humidity_info.json) {
            _humidity_print_json_data ();
        }

        if (humidity_info.display) {
            _humidity_display_data ();
        }

        point ++;
    }    
}

static void _humidity_reader_init (void) {
    menu_period_get_value (&period);
}

void humidity_reader_init (void) {
    _humidity_reader_init ();
}

void humidity_read_data (void) {
    _humidity_read_data ();
}