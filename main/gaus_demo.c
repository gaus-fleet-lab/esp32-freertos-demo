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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "gaus/gaus_client.h"
#include "curl/curl.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "wifi.h"
#include "gaus_helpers.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "ota.h"
#include "gaus_report.h"
#include "sntp.h"
#include "display.h"
#include "dht11.h"

//Tag for logging
static const char *TAG = "gaus-demo";

#define BLINK_GPIO 13
//Signal pin for DH11
#define PIN_NUM_DH11 4

#define MODEM_VERSION_MAJOR 1
#define MODEM_VERSION_MINOR 0
#define MODEM_VERSION_PATCH 0

#define DSP_VERSION_MAJOR 1
#define DSP_VERSION_MINOR 1
#define DSP_VERSION_PATCH 1

/* The examples use simple configuration that you can set via
   'make menuconfig'.
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define GAUS_SERVER_URL "https://gaus.incubation.io/"
*/
#define GAUS_SERVER_URL CONFIG_GAUS_SERVER_URL
#define GAUS_PRODUCT_ACCESS CONFIG_GAUS_PRODUCT_ACCESS
#define GAUS_PRODUCT_SECRET CONFIG_GAUS_PRODUCT_SECRET

//Returns a strong pointer to a null terminated version string
static char *version_string(void);
static char *dsp_version_string(void);

//Returns a strong pointer to a null terminated id string
static char *get_device_id(void);

//Returns a strong pointer to a null terminated location string
static char *get_device_location(void);

//FIXME: Use mac address or something
//Should be unique to this device (MAC or similar)
#define GAUS_DEVICE_ID CONFIG_GAUS_DEVICE_ID
#define GAUS_DEVICE_LOCATION CONFIG_GAUS_DEVICE_LOCATION

#define ID_COLOR TFT_RED
#define DETAILS_COLOR TFT_YELLOW
#define STATUS_COLOR TFT_GREEN

/*
 * End of configuration
 *
 * */

void gaus_communication_task(void *taskData) {
  char *device_access;
  char *device_secret;
  char *device_id = get_device_id();
  char *device_location = get_device_location();

  uint32_t poll_interval;
  gaus_session_t session;

  unsigned int filterCount = 3;
  gaus_header_filter_t filters[3] = {
      {
          strdup("modem-version"),
          version_string()
      },
      {
          strdup("location"),
          strdup(device_location)
      }, {
          strdup("dsp-version"),
          dsp_version_string()
      }
  };
  unsigned int updateCount = 0;
  gaus_update_t *updates = NULL;

  //Setup LED to blink:
  gpio_pad_select_gpio(BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  //Print Gaus library version
  gaus_version_t version = gaus_client_library_version();
  ESP_LOGI(TAG, "Gaus Client Library Version: v%d.%d.%d", version.major, version.minor, version.patch);

  // GAUS LIBRARY STEP 1: Initalize library
  // Only required if using library
  display_text_small(0, BOTTOM, STATUS_COLOR, "Init library...\r");
  gaus_error_t *err = gaus_global_init(GAUS_SERVER_URL, NULL);
  if (err) {
    ESP_LOGE(TAG, "An error occurred initializing!");
    goto FAIL;
  } else {
    ESP_LOGI(TAG, "Gaus library initialized!");
  }
  //Retrieve device access, device secret, poll interval from NonVolatileStorage (NVS)
  esp_err_t pi_error = get_nvs_u32("poll_interval", &poll_interval);
  esp_err_t da_error = get_nvs_str("device_access", &device_access);
  esp_err_t ds_error = get_nvs_str("device_secret", &device_secret);

  //Check if we've previously registered this device.
  if (pi_error == ESP_OK && da_error == ESP_OK && ds_error == ESP_OK) {
    ESP_LOGI(TAG, "Skipping registration as this device has previously been registered!");
    ESP_LOGI(TAG, "poll_interval: %d, device_access: %s, device_secret: %s", poll_interval, device_access,
             device_secret);
  } else {
    // GAUS LIBRARY STEP 2: Register device
    // Only required if device is unregistered.  The results of this should be persisted to the device.
    display_text_small(0, BOTTOM, STATUS_COLOR, "Register device...\r");
    err = gaus_register(GAUS_PRODUCT_ACCESS, GAUS_PRODUCT_SECRET, device_id,
                        &device_access, &device_secret, &poll_interval);
    if (err) {
      ESP_LOGE(TAG, "An error occurred registering!");
      ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
               err->description);
      goto FAIL;
    } else {
      ESP_LOGI(TAG, "Gaus registered!");
    }

    //Persist register results in NVS
    set_nvs_u32("poll_interval", poll_interval);
    set_nvs_str("device_access", device_access);
    set_nvs_str("device_secret", device_secret);
  }

  // GAUS LIBRARY STEP 3: Authenticate device
  // We need to collect a "session" to use for all future communications with Gaus system.
  display_text_small(0, BOTTOM, STATUS_COLOR, "Authenticate device...\r");
  err = gaus_authenticate(device_access, device_secret, &session);
  if (err) {
    ESP_LOGE(TAG, "An error occurred authenticating!");
    ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
             err->description);
    goto FAIL;
  } else {
    ESP_LOGI(TAG, "Gaus authenticated!");
  }

  //Main Loop
  while (1) {
    // GAUS LIBRARY STEP 4: Check for updates
    // Use session to check for updates.  If session has expired, we should aquire a new session
    // by calling gaus_authenticate() again.
    display_text_small(0, BOTTOM, STATUS_COLOR, "Check for updates...\r");
    err = gaus_check_for_updates(&session, filterCount, filters, &updateCount, &updates);
    if (err) {
      ESP_LOGE(TAG, "An error occurred checking for updates!");
      ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
               err->description);
      goto FAIL;
    } else {
      ESP_LOGI(TAG, "Gaus check for update successful!");
      if (updateCount > 0) {
        display_text_small(0, BOTTOM, STATUS_COLOR, "Found %d Updates!\r", updateCount);
        ESP_LOGI(TAG, "Found %d updates!", updateCount);
        for (int i = 0; i < updateCount; i++) {
          ESP_LOGI(TAG, "Updates: %d has updateId: %s!", updateCount, updates[i].update_id);
        }

        //On this system we can only handle 1 update at a time due to system restart
        //So just take the first one, subsequent will be handled on restart.
        ESP_LOGI(TAG, "Beginning update!");
        ESP_LOGW(TAG, "Beginning update with url %s!", updates[0].download_url);
        send_update_status_report(&session, "download", "starting", "Starting download", updates[0].update_id);
        esp_err_t upgrade_error = do_firmware_upgrade(updates[0].download_url);
        if (upgrade_error != ESP_OK) {
          ESP_LOGE(TAG, "Update failed... restarting!");
          send_update_status_report(&session, "install", "failed", "Downloading and installing update failed",
                                    updates[0].update_id);
          goto FAIL;
        } else {
          send_update_status_report(&session, "install", "success", "Installed new modem version.",
                                    updates[0].update_id);
          ESP_LOGW(TAG, "New modem version installed.  Restarting device.");
          goto INSTALL_SUCCESS;
        }
      } else {
        display_text_small(0, BOTTOM, STATUS_COLOR, "No updates found!\r");
        ESP_LOGI(TAG, "No Updates: %d!", updateCount);
      }
    }

    for (int i = 60; i >= 0; i--) {
      ESP_LOGI(TAG, "Next update check in %d seconds...", i);
      gpio_set_level(BLINK_GPIO, i % 2);
      if (i % 2) {
        //Measure temperature and report:
        float temperature = dht11_readTemperature(false, false);
        //Measure humidity and report:
        float humidity = dht11_readHumidity(false);
        if (!isnan(temperature) && !isnan(humidity)) {
          ESP_LOGI(TAG, "Reporting: Temperature: %.2f   Humidity: %.2f", temperature, humidity);
          send_update_temperature_and_humidity_report(&session, temperature, humidity);
          display_text_small(0, BIG_FONT_HEIGHT + SMALL_FONT_HEIGHT * 2 + LINE_SPACING * 3, STATUS_COLOR,
                             "T: %.1fC   H: %.1f\r", temperature, humidity);
        }
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }

  INSTALL_SUCCESS:
  FAIL:
  freeUpdates(updateCount, &updates);
  free(filters[0].filter_name);
  free(filters[0].filter_value);
  free(filters[1].filter_name);
  free(filters[1].filter_value);
  free(filters[2].filter_name);
  free(filters[2].filter_value);
  if (err) {
    free(err->description);
  }
  free(device_access);
  free(device_secret);
  free(device_id);
  free(device_location);
  free(session.device_guid);
  free(session.product_guid);
  free(session.token);
  free(err);
  esp_restart();
}

void app_main() {
  uint32_t reset_count = 0;
  //Setup wifi
  ESP_ERROR_CHECK(nvs_flash_init());

  get_nvs_u32("reset_count", &reset_count);
  ESP_LOGI(TAG, "Reset count is now %d!", reset_count);

  reset_count++;
  set_nvs_u32("reset_count", reset_count);

  dht11_init(PIN_NUM_DH11);

  initialize_display();

  display_text_small(0, BIG_FONT_HEIGHT + LINE_SPACING, DETAILS_COLOR, "Modem v: v%d.%d.%d\r",
                     MODEM_VERSION_MAJOR,
                     MODEM_VERSION_MINOR, MODEM_VERSION_PATCH);

  char *device_id = get_device_id();
  char *device_location = get_device_location();
  display_text_big(CENTER, 0, ID_COLOR, "%s\r", device_id);
  display_text_small(0, BIG_FONT_HEIGHT + SMALL_FONT_HEIGHT + LINE_SPACING * 2, DETAILS_COLOR, "Location: %s\r",
                     device_location);
  display_text_small(0, BIG_FONT_HEIGHT + SMALL_FONT_HEIGHT * 2 + LINE_SPACING * 2, DETAILS_COLOR, "dsp v: v%d.%d.%d\r",
                     DSP_VERSION_MAJOR,
                     DSP_VERSION_MINOR,
                     DSP_VERSION_PATCH);
  free(device_id);
  free(device_location);

  display_text_small(0, BOTTOM, STATUS_COLOR, "Init wifi...\r");
  initialise_wifi();
  if (!wait_on_wifi()) {
    ESP_LOGE(TAG, "Failed to connect to wifi!");
  }

  display_text_small(0, BOTTOM, STATUS_COLOR, "Sync time...\r");
  esp_err_t err = obtain_time();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize time!");
  }

  //Spin up a new task to handle Gaus communications
  xTaskCreatePinnedToCore(&gaus_communication_task, "gaus_communication_task", 10 * 1024, NULL, 5, NULL,
                          tskNO_AFFINITY);
}

static char *version_string(void) {
  char *version;
  size_t len =
      snprintf(NULL, 0, "%d.%d.%d", MODEM_VERSION_MAJOR, MODEM_VERSION_MINOR, MODEM_VERSION_PATCH) + 1;
  version = malloc(len);
  snprintf(version, len, "%d.%d.%d", MODEM_VERSION_MAJOR, MODEM_VERSION_MINOR, MODEM_VERSION_PATCH);
  return version;
}


static char *dsp_version_string(void) {
  char *version;
  size_t len =
      snprintf(NULL, 0, "%d.%d.%d", DSP_VERSION_MAJOR, DSP_VERSION_MINOR, DSP_VERSION_PATCH) + 1;
  version = malloc(len);
  snprintf(version, len, "%d.%d.%d", DSP_VERSION_MAJOR, DSP_VERSION_MINOR, DSP_VERSION_PATCH);
  return version;
}
//Returns a strong pointer to a string with device id
static char *get_device_id(void) {
  char *device_id;

  //Retrieve device id from NonVolatileStorage (NVS)
  esp_err_t id_error = get_nvs_str("device_id", &device_id);

  //Set id if not set.
  if (id_error != ESP_OK) {
    device_id = strdup(GAUS_DEVICE_ID);
    ESP_LOGW(TAG, "Did not find id!  Setting to default %s", device_id);
    set_nvs_str("device_id", device_id);
  }
  return device_id;
}

//Returns a strong pointer to a string with device location
static char *get_device_location(void) {
  char *device_location;

  //Retrieve device location from NonVolatileStorage (NVS)
  esp_err_t location_error = get_nvs_str("device_location", &device_location);

  //Set location if not set.
  if (location_error != ESP_OK) {
    device_location = strdup(GAUS_DEVICE_LOCATION);
    ESP_LOGW(TAG, "Did not find location!  Setting to default %s", device_location);
    set_nvs_str("device_location", device_location);
  }
  return device_location;
}
