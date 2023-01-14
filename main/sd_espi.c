#include "sd_espi.h"
#include "humidity.h"
#include "nvs_fields.h"

static const char *TAG = "SD SPI Card";

#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  CONFIG_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_PIN_CLK
#define PIN_NUM_CS    CONFIG_PIN_CS

extern humidity_data_t humidity_data[NUM_SENSORS];

static sdmmc_card_t *card = NULL;
// static sdmmc_host_t host  = { 0 };
static int host_slot = -1;
bool mounted = false;

    static esp_err_t _sd_espi_init (void);
static void _sd_espi_deinit    (void);

static void _sd_espi_write_humidity_data (uint32_t experiment, uint32_t point) {
    if (mounted) {
        char file_name[0x100] = { 0 };
        snprintf(file_name, 0x100, "/sdcard/data%04ld.csv", experiment);
        FILE *f_data = fopen(file_name, "a+");
        char strtime[0x20];
        if(f_data != NULL) {
            for (uint8_t sensor = 0; sensor < NUM_SENSORS; sensor++) {
                strftime(strtime, 0x20, "%d-%b-%Y %H:%M:%S", &humidity_data[sensor].time);
                fprintf(f_data, "%s;\t%"PRId64";\t%"PRIu32";%"PRIu32";\t%u;\t0x%02X;\t%3.3f;\t%3.3f;\n",
                    strtime,
                    mktime (&humidity_data[sensor].time),
                    experiment,
                    point,
                    sensor,
                    humidity_data[sensor].status,
                    humidity_data[sensor].temperature,
                    humidity_data[sensor].humidity);            
            }

            memset (strtime, 0, 0x10);
            fflush(f_data);
            fclose(f_data);
        }
    }
}

static esp_err_t _sd_espi_init (void) {
    esp_err_t ret;
    
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGD(__FUNCTION__, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGD(__FUNCTION__, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        spi_bus_free(host.slot);
        return ret;
    }

    host_slot = host.slot;
    ESP_LOGI(__FUNCTION__, "HostSlot: %d", host.slot);
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGD(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }

        mounted = false;
        return ret;
    }

    mounted = true;

    ESP_LOGI(__FUNCTION__, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

static void _sd_espi_deinit (void) {
    const char mount_point[] = MOUNT_POINT;

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host_slot);
    host_slot = -1;
    mounted = false;
}

esp_err_t sd_espi_init (void) {
    return _sd_espi_init ();
}

void sd_espi_deinit (void) {
    _sd_espi_deinit ();
}

void sd_espi_write_humidity_data (uint32_t experiment, uint32_t point) {
    _sd_espi_write_humidity_data (experiment, point);
}