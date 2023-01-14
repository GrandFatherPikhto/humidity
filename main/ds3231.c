#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ds3231.h"

#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)

static i2c_dev_t ds3231_dev = {
    .port = I2C_MASTER_NUM,
    .addr = DS3231_ADDR,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .clk_speed  = I2C_MASTER_FREQ_HZ,
};

static uint8_t _bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

static uint8_t _dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

static esp_err_t _ds3231_set_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* time/date data */
    data[0] = _dec2bcd(time->tm_sec);
    data[1] = _dec2bcd(time->tm_min);
    data[2] = _dec2bcd(time->tm_hour);
    /* The week data must be in the range 1 to 7, and to keep the start on the
     * same day as for tm_wday have it start at 1 on Sunday. */
    data[3] = _dec2bcd(time->tm_wday + 1);
    data[4] = _dec2bcd(time->tm_mday);
    data[5] = _dec2bcd(time->tm_mon + 1);
    data[6] = _dec2bcd(time->tm_year);

    return i2c_dev_write_reg(dev, DS3231_ADDR_TIME, data, 7);
}

static esp_err_t _ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp)
{
    CHECK_ARG(dev);
    CHECK_ARG(temp);

    uint8_t data[2];

    esp_err_t res = i2c_dev_read_reg (dev, DS3231_ADDR_TEMP, data, sizeof(data));
    if (res == ESP_OK)
        *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

    return res;
}

static esp_err_t _ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = _ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = t_int >> 2;

    return res;
}

static esp_err_t _ds3231_get_temp_float(i2c_dev_t *dev, float *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = _ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = t_int * 0.25;

    return res;
}

static esp_err_t _ds3231_get_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* read time */
    esp_err_t res = i2c_dev_read_reg (dev, DS3231_ADDR_TIME, data, 7);
        if (res != ESP_OK) return res;

    /* convert to unix time structure */
    time->tm_sec = _bcd2dec(data[0]);
    time->tm_min = _bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG)
    {
        /* 12H */
        time->tm_hour = _bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        /* AM/PM? */
        if (data[2] & DS3231_PM_FLAG) time->tm_hour += 12;
    }
    else time->tm_hour = _bcd2dec(data[2]); /* 24H */
    time->tm_wday      = _bcd2dec(data[3]) - 1;
    time->tm_mday      = _bcd2dec(data[4]);
    time->tm_mon       = _bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    time->tm_year      = _bcd2dec(data[6]);
    time->tm_isdst     = 0;

    // ESP_LOGI(__FUNCTION__, "Time Zone: %d", CONFIG_TIMEZONE);
    // apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
    // applyTZ(time);

    return ESP_OK;
}

esp_err_t ds3231_get_dev_time(i2c_dev_t *dev, struct tm *time) {
    return _ds3231_get_time(dev, time);
}

esp_err_t ds3231_set_dev_time(i2c_dev_t *dev, struct tm *time) {
    return _ds3231_set_time(dev, time);
}

esp_err_t ds3231_get_dev_temp_float(i2c_dev_t *dev, float *temp) {
    return _ds3231_get_temp_float(dev, temp);
}

esp_err_t ds3231_get_dev_temp_integer(i2c_dev_t *dev, int8_t *temp) {
    return _ds3231_get_temp_integer(dev, temp);
}

esp_err_t ds3231_get_dev_raw_temp(i2c_dev_t *dev, int16_t *temp) {
    return _ds3231_get_raw_temp(dev, temp);
}

esp_err_t ds3231_get_time (struct tm *time) {
    return _ds3231_get_time(&ds3231_dev, time);
}

esp_err_t ds3231_set_time (struct tm *time) {
    return _ds3231_set_time(&ds3231_dev, time);
}

esp_err_t ds3231_get_raw_temp (int16_t *temp) {
    return _ds3231_get_raw_temp(&ds3231_dev, temp);
}

esp_err_t ds3231_get_temp_float (float *temp) {
    return _ds3231_get_temp_float(&ds3231_dev, temp);
}

esp_err_t ds3231_get_temp_integer(int8_t *temp) {
    return _ds3231_get_temp_integer(&ds3231_dev, temp);
}

