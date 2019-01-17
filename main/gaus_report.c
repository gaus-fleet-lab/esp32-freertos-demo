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
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <esp_log.h>
#include <gaus/gaus_client.h>

#include "gaus_report.h"
#include "gaus_helpers.h"

#define TAG "gaus_report"

void send_update_status_report(gaus_session_t *session, char *phase, char *status, char *logLine, char *updateId) {

  time_t now = 0;
  time(&now);
  char time[25];  //Time is always 25 chars with null (Format: 2018-11-15T12:00:22.000Z)
  strftime(time, sizeof(time), "%FT%T.000Z", gmtime(&now));

  gaus_report_header_t header = {
      strdup(time)
  };

  unsigned int reportCount = 1;
  gaus_v_string_t vstrings[4] = {
      {
          strdup("phase"),
          strdup(phase)
      },
      {
          strdup("status"),
          strdup(status)
      },
      {
          strdup("logLine"),
          strdup(logLine)
      },
      {
          strdup("updateId"),
          strdup(updateId)
      }
  };

  gaus_report_t report[1] = {
      {
          .report = {
              .update_status = {
                  .type = strdup("Status"),
                  .ts = strdup(time),
                  .v_int_count = 0,
                  .v_ints = NULL,
                  .v_float_count = 0,
                  .v_floats = NULL,
                  .v_string_count = 4,
                  .v_strings = vstrings,
                  .tag_count = 0,
                  .tags = NULL
              }
          },
          .report_type = GAUS_REPORT_UPDATE
      }
  };

  gaus_error_t *err = gaus_report(session, 0, NULL, &header, reportCount, report);
  if (err) {
    ESP_LOGE(TAG, "An error occurred making a report!");
    ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
             err->description);
    free(err->description);
  } else {
    ESP_LOGI(TAG, "Report made successfully!");
  }
  freeReports(reportCount, report);
  free(header.ts);
}

void send_update_temperature_and_humidity_report(gaus_session_t *session, float temperature, float humidity) {

  time_t now = 0;
  time(&now);
  char time[25];  //Time is always 25 chars with null (Format: 2018-11-15T12:00:22.000Z)
  strftime(time, sizeof(time), "%FT%T.000Z", gmtime(&now));

  gaus_report_header_t header = {
      strdup(time)
  };

  unsigned int reportCount = 2;
  gaus_v_float_t temperature_data[1] = {
      {
          strdup("temperature"),
          temperature
      }
  };

  gaus_v_float_t humidity_data[1] = {
      {
          strdup("humidity"),
          humidity
      }
  };

  gaus_report_t report[2] = {
      {
          .report = {
              .generic = {
                  .type = strdup("Temperature"),
                  .ts = strdup(time),
                  .v_int_count = 0,
                  .v_ints = NULL,
                  .v_float_count = 1,
                  .v_floats = temperature_data,
                  .v_string_count = 0,
                  .v_strings = NULL,
                  .tag_count = 0,
                  .tags = NULL
              }
          },
          .report_type = GAUS_REPORT_GENERIC
      },
      {
          .report = {
              .generic = {
                  .type = strdup("Humidity"),
                  .ts = strdup(time),
                  .v_int_count = 0,
                  .v_ints = NULL,
                  .v_float_count = 1,
                  .v_floats = humidity_data,
                  .v_string_count = 0,
                  .v_strings = NULL,
                  .tag_count = 0,
                  .tags = NULL
              }
          },
          .report_type = GAUS_REPORT_GENERIC
      }
  };

  gaus_error_t *err = gaus_report(session, 0, NULL, &header, reportCount, report);
  if (err) {
    ESP_LOGE(TAG, "An error occurred making a temperature and humidity report!");
    ESP_LOGE(TAG, "error_type: %d, http_error_code: %d, description: %s", err->error_type, err->http_error_code,
             err->description);
    free(err->description);
  } else {
    ESP_LOGI(TAG, "Report made successfully!");
  }
  freeReports(reportCount, report);
  free(header.ts);
}
