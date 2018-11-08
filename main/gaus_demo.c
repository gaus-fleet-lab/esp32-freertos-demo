/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

//Tag for logging
static const char *TAG = "gaus-demo";

void app_main()
{
    char *serverUrl = strdup("https://gaus-test.dev.gaus.sonymobile.com");

    //Setup wifi
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    if(!wait_on_wifi()) {
      goto FAIL;
    }

    //Print gaus version and initialize library
    gaus_version_t version = gaus_client_library_version();
    ESP_LOGI(TAG, "Gaus Client Library Version: v%d.%d.%d", version.major, version.minor, version.patch);
    gaus_error_t *err = gaus_global_init(serverUrl, NULL);
    if(err) {
      ESP_LOGE(TAG, "An error occurred initializing!");
    } else {
      ESP_LOGI(TAG, "Gaus initialized!");
    }

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 120; i >= 0; i--) {
        ESP_LOGI(TAG, "Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    FAIL:
    ESP_LOGW(TAG, "Restarting now.");
    free(serverUrl);
    fflush(stdout);
    esp_restart();

}
