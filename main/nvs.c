//The MIT License (MIT)
//
//Copyright 2018, Sony Mobile Communications Inc.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "nvs.h"
#include "esp_log.h"

//Which page in nvs to store in
#define PAGE "gaus"

#define TAG "gaus-nvs-helper"

esp_err_t set_nvs_u32(const char *name, uint32_t value) {
  nvs_handle my_handle;
  esp_err_t err = nvs_open(PAGE, NVS_READWRITE, &my_handle);
  if (err == ESP_OK) {
    err = nvs_set_u32(my_handle, name, value);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred setting %s to %d", err, name, value);
    }
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred committing changes to page:name %s:%s", err, PAGE, name);
    }
    nvs_close(my_handle);
  } else {
    ESP_LOGE(TAG, "An error (%d) opening page %s", err, PAGE);
  }
  return err;
}

esp_err_t get_nvs_u32(const char *name, uint32_t *value) {
  nvs_handle my_handle;
  esp_err_t err = nvs_open(PAGE, NVS_READWRITE, &my_handle);
  if (err == ESP_OK) {
    err = nvs_get_u32(my_handle, name, value);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred getting %s", err, name);
    }
    nvs_close(my_handle);
  } else {
    ESP_LOGE(TAG, "An error (%d) opening page %s", err, PAGE);
  }
  return err;
}

esp_err_t set_nvs_str(const char *name, const char *value) {
  nvs_handle my_handle;
  esp_err_t err = nvs_open(PAGE, NVS_READWRITE, &my_handle);
  if (err == ESP_OK) {
    err = nvs_set_str(my_handle, name, value);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred setting %s to %s", err, name, value);
    }
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred committing changes to page:name %s:%s", err, PAGE, name);
    }
    nvs_close(my_handle);
  } else {
    ESP_LOGE(TAG, "An error (%d) opening page %s", err, PAGE);
  }
  return err;
}


//Expects a pointer to a char * that will be allocated with the required size and then filled
//Does not free any existing contents so that should be done before calling if there is existing
//contents
esp_err_t get_nvs_str(const char *name, char **value) {
  nvs_handle my_handle;
  esp_err_t err = nvs_open(PAGE, NVS_READWRITE, &my_handle);
  if (err == ESP_OK) {
    size_t required_size;
    nvs_get_str(my_handle, name, NULL, &required_size);
    *value = malloc(required_size);
    err = nvs_get_str(my_handle, name, *value, &required_size);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "An error (%d) occurred getting %s", err, name);
    }
    nvs_close(my_handle);
  } else {
    ESP_LOGE(TAG, "An error (%d) opening page %s", err, PAGE);
  }
  return err;
}
