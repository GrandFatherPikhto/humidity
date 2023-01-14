#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#pragma once

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                  /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

typedef struct {
    i2c_port_t port;            // I2C port number
    uint8_t addr;               // I2C address
    gpio_num_t sda_io_num;      // GPIO number for I2C sda signal
    gpio_num_t scl_io_num;      // GPIO number for I2C scl signal
    uint32_t clk_speed;         // I2C clock frequency for master mode
} i2c_dev_t;

esp_err_t i2c_master_initialization (i2c_port_t port, 
                                     int sda, 
                                     int scl);

esp_err_t i2c_dev_read    (const i2c_dev_t *dev, 
                           const void *out_data, 
                           size_t out_size, 
                           void *in_data, 
                           size_t in_size);

esp_err_t i2c_dev_write   (const i2c_dev_t *dev, 
                           const void *out_reg, 
                           size_t out_reg_size, 
                           const void *out_data, 
                           size_t out_size);


inline esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg,
        void *in_data, size_t in_size)
{
    return i2c_dev_read(dev, &reg, 1, in_data, in_size);
}

inline esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg,
        const void *out_data, size_t out_size)
{
    return i2c_dev_write(dev, &reg, 1, out_data, out_size);
}
