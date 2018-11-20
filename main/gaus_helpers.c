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
#include "gaus_helpers.h"
#include <esp_log.h>
#include <stdlib.h>

#define TAG "gaus-helpers"

void freeUpdate(gaus_update_t update) {
  for (int i = 0; i < update.metadata_count; i++) {
    free(update.metadata[i].key);
    free(update.metadata[i].value);
  }
  update.metadata_count = 0;
  update.size = 0;
  free(update.metadata);
  free(update.update_type);
  free(update.package_type);
  free(update.md5);
  free(update.update_id);
  free(update.version);
  free(update.download_url);
}

void freeUpdates(unsigned int updateCount, gaus_update_t **updates) {
  for (int i = 0; i < updateCount; i++) {
    freeUpdate((*updates)[i]);
  }
  free(*updates);
}

void freeReport(gaus_report_t report) {
  switch (report.report_type) {
    case GAUS_REPORT_UPDATE:
      for (int i = 0; i < report.report.update_status.v_string_count; i++) {
        free(report.report.update_status.v_strings[i].name);
        free(report.report.update_status.v_strings[i].value);
      }
      free(report.report.update_status.type);
      free(report.report.update_status.ts);
      break;
    case GAUS_REPORT_GENERIC:
      for (int i = 0; i < report.report.generic.v_int_count; i++) {
        free(report.report.generic.v_ints[i].name);
      }
      for (int i = 0; i < report.report.generic.v_float_count; i++) {
        free(report.report.generic.v_floats[i].name);
      }
      for (int i = 0; i < report.report.generic.v_string_count; i++) {
        free(report.report.generic.v_strings[i].name);
        free(report.report.generic.v_strings[i].value);
      }
      free(report.report.generic.type);
      free(report.report.generic.ts);
      break;
    default:

      break;
  }
}


void freeReports(unsigned int reportCount, gaus_report_t *reports) {
  for (int i = 0; i < reportCount; i++) {
    freeReport(reports[i]);
  }
}
