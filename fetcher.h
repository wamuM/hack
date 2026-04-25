#ifndef FETCHER_H
#define FETCHER_H

#include "cJSON.h"
#include <curl/curl.h>

cJSON* fetch_json(const char* query);
int fetch_file(const char* query);

#endif
