#ifndef FETCHER_H
#define FETCHER_H

#include "cJSON.h"
#include <curl/curl.h>

extern const char* TRIP_UPDATES;
extern const char* VEHICLE_POSITIONS;
extern const char* SCHEDULE;

cJSON* fetch_json(const char* url);
int fetch_file(const char* url, const char* dst);

#endif
