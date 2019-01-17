#ifndef ESP32_TFT_LIBRARY_DH11_H
#define ESP32_TFT_LIBRARY_DH11_H

#include <esp_types.h>
void dht11_init(int pin_num);
float dht11_readTemperature(bool S, bool force);
float dht11_readHumidity(bool force);

#endif
