#ifndef ESP32_FREERTOS_DEMO_SNTP_H
#define ESP32_FREERTOS_DEMO_SNTP_H

#include <esp_err.h>

//Fetch the time from network and initialize sntp
//Blocks until time is set or too many retrys
esp_err_t obtain_time(void);

#endif //ESP32_FREERTOS_DEMO_SNTP_H
