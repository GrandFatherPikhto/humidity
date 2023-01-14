#include "i2cdev.h"

#define TAG "I2C_Master"

#define I2CDEV_TIMEOUT              1000                       /*!<  > */ 

static esp_err_t _i2c_master_initialization (i2c_port_t port, int sda, int scl)
{
    esp_err_t err = ESP_OK;

    i2c_config_t i2c_config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = sda,
            .scl_io_num = scl,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 1000000
    };  
    //i2c_param_config(I2C_NUM_0, &i2c_config);
    //i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    err = i2c_param_config(port, &i2c_config);

    if (err != ESP_OK) {
        return err;
    }

	ESP_LOGD(TAG, "INTERFACE is i2c");
	ESP_LOGD(TAG, "CONFIG_SDA_GPIO=%d", CONFIG_I2C_MASTER_SDA);
	ESP_LOGD(TAG, "CONFIG_SCL_GPIO=%d", CONFIG_I2C_MASTER_SCL);
	ESP_LOGD(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    return i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
}

static esp_err_t _i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
{
    if (!dev || !in_data || !in_size) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | 1, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

static esp_err_t _i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size)
{
    if (!dev || !out_data || !out_size) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    if (out_reg && out_reg_size)
        i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);
    i2c_master_write(cmd, (void *)out_data, out_size, true);
    i2c_master_stop(cmd);
    esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d", dev->addr, dev->port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

esp_err_t i2c_master_initialization (i2c_port_t port, int sda, int scl) {
    return _i2c_master_initialization (port, sda, scl);
}

esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size) {
    return _i2c_dev_read (dev, out_data, out_size, in_data, in_size);
}

esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size) {
    return _i2c_dev_write (dev, out_reg, out_reg_size, out_data, out_size);
}