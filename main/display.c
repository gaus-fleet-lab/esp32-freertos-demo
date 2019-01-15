//The MIT License (MIT)
//
//Copyright 2018, Sony Mobile Communications Inc.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
//rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
//persons to whom the Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
//Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
//COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "miniTFTWing.h"

#include "esp_log.h"

//Display headers:
#include "tft.h"
#include "freertos/task.h"
#include "display.h"

//Tag for logging
static const char *TAG = "display";

// Define which spi bus to use TFT_VSPI_HOST or TFT_HSPI_HOST
#define SPI_BUS TFT_HSPI_HOST
#define DISPLAY_MAX_LEN 21

void initialize_display() {
  esp_err_t ret;

  // Setup the miniTFTWing gpio's and reset display
  Adafruit_miniTFTWing_init();
  if (!Adafruit_miniTFTWing_begin()) {
    ESP_LOGE(TAG, "Failed to connect to miniTFTWing!");
  } else {
    ESP_LOGI(TAG, "Connected to miniTFTWing");
  }

  ESP_LOGI(TAG, "Resetting miniTFTWing");

  Adafruit_miniTFTWing_tftReset(true);

  //Initialize the display:
  max_rdclock = 8000000; //Max read clock (used before display init, after uses find_rd_speed())
  TFT_PinsInit(); //Must be done before SPI interface init

  //Configure spi device(s)
  spi_lobo_device_handle_t spi;

  spi_lobo_bus_config_t buscfg = {
      .miso_io_num=PIN_NUM_MISO,        // set SPI MISO pin
      .mosi_io_num=PIN_NUM_MOSI,        // set SPI MOSI pin
      .sclk_io_num=PIN_NUM_CLK,        // set SPI CLK pin
      .quadwp_io_num=-1,
      .quadhd_io_num=-1,
      .max_transfer_sz = 6 * 1024,
  };
  spi_lobo_device_interface_config_t devcfg = {
      .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
      .mode=0,                                // SPI mode 0
      .spics_io_num=-1,                       // we will use external CS pin
      .spics_ext_io_num=PIN_NUM_CS,           // external CS pin
      .flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
  };

  vTaskDelay(500 / portTICK_RATE_MS);
  ESP_LOGI(TAG, "SPI Bus configured! pins used: miso=%d, mosi=%d, sck=%d, cs=%d", PIN_NUM_MISO, PIN_NUM_MOSI,
           PIN_NUM_CLK, PIN_NUM_CS);

  //Initialize the SPI bus and attach the LCD to the SPI bus
  ret = spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
  assert(ret == ESP_OK);
  ESP_LOGI(TAG, "Display device added to spi bus (%d)", SPI_BUS);
  disp_spi = spi;

  //Test select/deselect
  ret = spi_lobo_device_select(spi, 1);
  assert(ret == ESP_OK);
  ret = spi_lobo_device_deselect(spi);
  assert(ret == ESP_OK);

  ESP_LOGI(TAG, "Attached display device, speed=%u", spi_lobo_get_speed(spi));
  ESP_LOGI(TAG, "Bus uses native pins: %s", spi_lobo_uses_native_pins(spi) ? "true" : "false");

  //Initialize the Display

  ESP_LOGI(TAG, "Starting display init...");
  TFT_display_init();
  ESP_LOGI(TAG, "Display init completed");

  //Detect maximum read speed
  max_rdclock = find_rd_speed();
  ESP_LOGI(TAG, "Display max rd speed: %u", max_rdclock);

  // ==== Set SPI clock used for display operations ====
  spi_lobo_set_speed(spi, 5000000);
  ESP_LOGI(TAG, "Changed spi bus speed to: %u", spi_lobo_get_speed(spi));

  font_rotate = 0;
  text_wrap = 0;
  font_transparent = 0;
  font_forceFixed = 0;
  gray_scale = 0;
  TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
  TFT_setRotation(LANDSCAPE);
  TFT_setFont(DEFAULT_FONT, NULL);
  TFT_resetclipwin();

  TFT_fillScreen(TFT_BLACK);
}

void clear_screen() {
  TFT_fillScreen(TFT_BLACK);
}

void display_text_small(int x, int y, color_t color, char *fmt, ...) {
  va_list ap;
  char str[DISPLAY_MAX_LEN];

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  ESP_LOGI(TAG, "Displaying small text %s", str);
  _fg = color;
  _bg = TFT_BLACK;
  TFT_setFont(SMALL_FONT, NULL);
  TFT_clearStringRect(x, y, str);
  TFT_print(str, x, y);
}

void display_text_big(int x, int y, color_t color, char *fmt, ...) {
  va_list ap;
  char str[DISPLAY_MAX_LEN];

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  ESP_LOGI(TAG, "Displaying big text %s", str);
  _fg = color;
  _bg = TFT_BLACK;
  TFT_setFont(DEJAVU18_FONT, NULL);
  TFT_clearStringRect(x, y, str);
  TFT_print(str, x, y);
}
