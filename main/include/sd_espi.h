#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#pragma once

void sd_espi_write_humidity_data(uint32_t experiment, uint32_t point);
esp_err_t sd_espi_init   (void);
void sd_espi_deinit (void);