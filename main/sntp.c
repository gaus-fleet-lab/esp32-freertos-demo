#include "sntp.h"
#include <lwip/err.h>
#include <apps/sntp/sntp.h>
#include <nvs_flash.h>
#include <esp_log.h>

#define TAG "sntp"

#define RETRY_TIMEOUT 2000

static void initialize_sntp(void) {
  ESP_LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();
}

esp_err_t obtain_time(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  initialize_sntp();

  // wait for time to be set
  time_t now = 0;
  struct tm timeinfo = {0};
  int retry = 0;
  const int max_retries = 10;
  //This was written in 2018, but will be valid going forward too...
  //localtime_r gives years since 1900
  while (timeinfo.tm_year < (2018 - 1900) && ++retry < max_retries) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, max_retries);
    vTaskDelay(RETRY_TIMEOUT / portTICK_PERIOD_MS);
    time(&now);
    localtime_r(&now, &timeinfo);
  }

  if (retry >= max_retries) {
    ESP_LOGE(TAG, "Failed to set system time in %d attempts", retry);
    return ESP_FAIL;
  }
  return ESP_OK;
}
