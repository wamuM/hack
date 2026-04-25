#ifndef FETCHER_H
#define FETCHER_H

#include "cJSON.h"
#include <curl/curl.h>

cJSON* get_json(const char* region, int admin_level, int admin_level2);
int fetch_file(const char* query, const char* region, int admin_level, int admin_level2);

#endif
