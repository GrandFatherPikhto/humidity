#include <stdio.h>
#include "time.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2cdev.h"

#pragma once

#define NUM_SENSORS 8

#define HIH6020_REQUEST_ADDR        0xFF        /*!< Register addresses of the power managment register */
#define HIH6020_SENSOR_ADDR         0x27        /*!< Slave address of the HIH6020 sensor */
#define TCA9548A_ADDR               0x70        /*!< Slave address of TCA9845A mutliplexer */
#define TCA9548A_ADDR_NUM           0x00        /*!< Set number fo TCA9845A multiplexer */

#define HIH6020_POWER_UP_DELAY      0x75
#define HIH6020_CONVERSION_DELAY    0x60
#define HIH6020_DATA_SIZE           4

#define HIH6020_STATUS_READ_ERROR   0xFF

typedef struct {
    struct tm time;
    uint8_t status;
    float temperature;
    float humidity;
} humidity_data_t;

typedef struct {
    uint64_t period;
    uint8_t 
        write: 1,
        log: 1,
        display: 1,
        json: 1,
        unused: 3;
} humidity_info_t;

void humidity_reader_init (void);
void humidity_read_data   (void);