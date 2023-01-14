#include "ssd1306_oled.h"

#define TAG "OLED"

static SSD1306_t dev;
static int center, top, bottom;

static void _oled_init() {

	dev._address = I2CAddress;
	dev._flip = false;

#if CONFIG_FLIP
	dev._flip = true;
	ESP_LOGW(TAG, "Flip upside down");
#endif

#if CONFIG_SSD1306_128x64
	ESP_LOGD(TAG, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x64
	top    = 1;
	center = 1;
	bottom = 4;
    ESP_LOGD(TAG, "SSD1306 128x64");
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text_x3(&dev, 0, "HUMID", MENU_DISPLAY_LEN, false);
#endif // CONFIG_SSD1306_128x64

	vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void oled_init (void) {
	_oled_init ();
}

void oled_close(void) {
    	// Fade Out
	ssd1306_fadeout(&dev);
}

void oled_display_menu_item(uint8_t page, char *title, bool selected) {
	ssd1306_display_text(&dev, page, title, MENU_DISPLAY_LEN, selected);
}

void oled_clear_menu_item(uint8_t page) {
	ssd1306_clear_line(&dev, page, false);
}

void oled_clear_screen(void) {
	ssd1306_clear_screen(&dev, false);
}

uint8_t oled_get_pages(void) {
	return ssd1306_get_pages(&dev);
}
