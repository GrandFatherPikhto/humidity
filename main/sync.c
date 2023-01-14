#include "sync.h"
#include "i2cdev.h"
#include "ds3231.h"

#define TAG "Sync"

// static int s_retry_num = 0;

static void _time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void _initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    //sntp_setservername(0, "pool.ntp.org");
    ESP_LOGI(TAG, "Your NTP Server is %s", CONFIG_NTP_SERVER);
    sntp_setservername(0, CONFIG_NTP_SERVER);
    sntp_set_time_sync_notification_cb(_time_sync_notification_cb);
    sntp_init();
}

static bool _obtain_time(void)
{
    ESP_ERROR_CHECK( esp_netif_init() );
    _initialize_sntp();

    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (retry == retry_count) return false;

    return true;
}

static void _sync_clock(void)
{
    // obtain time over NTP
    ESP_LOGI(__FUNCTION__, "Connecting to WiFi and getting time over NTP.");
    if(!_obtain_time()) {
        ESP_LOGE(__FUNCTION__, "Fail to getting time over NTP.");
        while (1) { vTaskDelay(1); }
    }

    // update 'now' variable with current time
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    time(&now);
    // now = now + (CONFIG_TIMEZONE*60*60);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(__FUNCTION__, "The current date/time is: %s", strftime_buf);

    // Initialize RTC
    i2c_dev_t dev;
    if (ds3231_init_desc(&dev, I2C_NUM_0, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO) != ESP_OK) {
        ESP_LOGE(__FUNCTION__, "Could not init device descriptor.");
        while (1) { vTaskDelay(1); }
    }

    ESP_LOGI(__FUNCTION__, "timeinfo.tm_sec=%d",timeinfo.tm_sec);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_min=%d",timeinfo.tm_min);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_hour=%d",timeinfo.tm_hour);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_wday=%d",timeinfo.tm_wday);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_mday=%d",timeinfo.tm_mday);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_mon=%d",timeinfo.tm_mon);
    ESP_LOGI(__FUNCTION__, "timeinfo.tm_year=%d",timeinfo.tm_year);

        struct tm time = {
        .tm_year = timeinfo.tm_year,
        .tm_mon  = timeinfo.tm_mon,  // 0-based
        .tm_mday = timeinfo.tm_mday,
        .tm_hour = timeinfo.tm_hour,
        .tm_min  = timeinfo.tm_min,
        .tm_sec  = timeinfo.tm_sec
    };

    if (ds3231_set_time(&dev, &time) != ESP_OK) {
        ESP_LOGE(__FUNCTION__, "Could not set time.");
        while (1) { vTaskDelay(1); }
    }
    
    ESP_LOGI(__FUNCTION__, "Set initial date time done");
    
    i2c_driver_delete(I2C_NUM_0);
}

static void _get_clock(void)
{
    // Initialize RTC
    i2c_dev_t dev;
    if (ds3231_init_desc(&dev, I2C_NUM_0, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO) != ESP_OK) {
        ESP_LOGE(pcTaskGetName(0), "Could not init device descriptor.");
        while (1) { vTaskDelay(1); }
    }

    // Initialise the xLastWakeTime variable with the current time.
    // TickType_t xLastWakeTime = xTaskGetTickCount();

    // Get RTC date and time
    float temp;
    struct tm rtcinfo;

    if (ds3231_get_temp_float(&dev, &temp) != ESP_OK) {
        ESP_LOGE(__FUNCTION__, "Could not get temperature.");
        while (1) { vTaskDelay(1); }
    }

    if (ds3231_get_time(&dev, &rtcinfo) != ESP_OK) {
        ESP_LOGE(__FUNCTION__, "Could not get time.");
        while (1) { vTaskDelay(1); }
    }

    ESP_LOGI(__FUNCTION__, "%04d-%02d-%02d %02d:%02d:%02d, %.2f deg Cel", 
        rtcinfo.tm_year, rtcinfo.tm_mon + 1,
        rtcinfo.tm_mday, rtcinfo.tm_hour, rtcinfo.tm_min, rtcinfo.tm_sec, temp);
    
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_sec=%d",rtcinfo.tm_sec);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_min=%d",rtcinfo.tm_min);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_hour=%d",rtcinfo.tm_hour);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_wday=%d",rtcinfo.tm_wday);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_mday=%d",rtcinfo.tm_mday);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_mon=%d",rtcinfo.tm_mon);
    ESP_LOGI(__FUNCTION__, "rtcinfo.tm_year=%d",rtcinfo.tm_year);

    char strtime[0x30];
    // rtcinfo.tm_year -= 1900;
    strftime(strtime, 0x30, "%d/%b/%Y %H:%M:%S", &rtcinfo);
    ESP_LOGI(__FUNCTION__, "%"PRId64", %s", mktime(&rtcinfo), strtime);

    i2c_driver_delete(I2C_NUM_0);	    
}

void sync_clock (void) {
    // xTaskCreate(& _sync_clock, "setClock", 1024*4, NULL, 2, NULL);
    _sync_clock ();
}

void get_clock (void) {
    _get_clock ();
}