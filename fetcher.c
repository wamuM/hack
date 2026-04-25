#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetcher.h"
#include "cJSON.h"

const char* TRIP_UPDATES = "https://gtfsrt.renfe.com/trip_updates.json";
const char* VEHICLE_POSITIONS = "https://gtfsrt.renfe.com/vehicle_positions.json";
const char* SCHEDULE = "https://ssl.renfe.com/ftransit/Fichero_CER_FOMENTO/fomento_transit.zip";

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

cJSON* fetch_json(const char* url)
{
  CURL *curl = curl_easy_init();
  if(!curl) 
  {
    printf("error initializing curl\n");
    exit(-1);
  }

  struct string s;
  init_string(&s);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  CURLcode result = curl_easy_perform(curl);
  if(result)
  {
    printf("error fetching json\n");
    return NULL;
  }

  curl_easy_cleanup(curl);
  cJSON* json = cJSON_Parse(s.ptr);
  free(s.ptr);
  return json;
}

int fetch_file(const char* url, const char* dst)
{
  CURL *curl;
  FILE *fp;
  CURLcode res;
  curl = curl_easy_init();
  
  if (!curl) return 1;

  printf("Fetching %s -> %s\n", url, dst);

  fp = fopen(dst,"wb");
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  res = curl_easy_perform(curl);
  if(res)
    return 1;

  curl_easy_cleanup(curl);
  fclose(fp);
  return 0;
}
