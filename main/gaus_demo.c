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

//Tag for logging
static const char *TAG = "gaus-demo";


/* The examples use simple configuration that you can set via
   'make menuconfig'.
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define GAUS_SERVER_URL "https://gaus.incubation.io/"
*/
#define GAUS_SERVER_URL CONFIG_GAUS_SERVER_URL
#define GAUS_PRODUCT_ACCESS CONFIG_GAUS_PRODUCT_ACCESS
#define GAUS_PRODUCT_SECRET CONFIG_GAUS_PRODUCT_SECRET

//FIXME: Use mac address or something
//Should be unique to this device (MAC or similar)
#define GAUS_DEVICE_ID CONFIG_GAUS_DEVICE_ID

/*
 * End of configuration
 *
 * */

void gaus_communication_task(void *taskData) {
  char *device_access;
  char *device_secret;
  unsigned int poll_interval;
  gaus_session_t session;

  unsigned int filterCount = 1;
  gaus_header_filter_t filters[1] = {
      {
          strdup("firmware-version"),
          strdup("0.0.0")
      }
  };
  unsigned int updateCount = 0;
  gaus_update_t *updates = NULL;

  for (int i = 5; i >= 0; i--) {
    ESP_LOGI(TAG, "Starting test in %d seconds...", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  //Print gaus version and initialize library
  gaus_version_t version = gaus_client_library_version();
  ESP_LOGI(TAG, "Gaus Client Library Version: v%d.%d.%d", version.major, version.minor, version.patch);
  gaus_error_t *err = gaus_global_init(GAUS_SERVER_URL, NULL);
  if (err) {
    ESP_LOGE(TAG, "An error occurred initializing!");
    goto FAIL;
  } else {
    ESP_LOGI(TAG, "Gaus library initialized!");
  }

  //Fixme: Should only register if not registered before.
  //Register device:
  err = gaus_register(GAUS_PRODUCT_ACCESS, GAUS_PRODUCT_SECRET, GAUS_DEVICE_ID,
                      &device_access, &device_secret, &poll_interval);
  if (err) {
    ESP_LOGE(TAG, "An error occurred registering!");
    ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
             err->description);
    goto FAIL;
  } else {
    ESP_LOGI(TAG, "Gaus registering!");
  }

  //Fixme: Should load access/secret from storage.
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
    err = gaus_check_for_updates(&session, filterCount, filters, &updateCount, &updates);
    if (err) {
      ESP_LOGE(TAG, "An error occurred checking for updates!");
      ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
               err->description);
      goto FAIL;
    } else {
      ESP_LOGI(TAG, "Gaus check for update successful!");
    }

    for (int i = 120; i >= 0; i--) {
      ESP_LOGI(TAG, "Sleeping for %d seconds...\n", i);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }

  FAIL:
  freeUpdates(updateCount, &updates);
  free(filters[0].filter_name);
  free(filters[0].filter_value);
  if (err) {
    free(err->description);
  }
  free(device_access);
  free(device_secret);
  free(session.device_guid);
  free(session.product_guid);
  free(session.token);
  free(err);
  esp_restart();
}

void app_main() {
  //Setup wifi
  ESP_ERROR_CHECK(nvs_flash_init());
  initialise_wifi();
  if (!wait_on_wifi()) {
    ESP_LOGE(TAG, "Failed to connect to wifi!");
  }

  //Spin up a new task to handle gaus communications
  xTaskCreatePinnedToCore(&gaus_communication_task, "gaus_communication_task", 10 * 1024, NULL, 5, NULL,
                          tskNO_AFFINITY);
}
