#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "fetcher.h"
#include "cJSON.h"

struct string {
  char *ptr;
  size_t len;
};

cJSON* load_json_file(const char* filename);

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

cJSON* get_json(const char* region, int admin_level)
{
  CURL *curl = curl_easy_init();
  if(!curl) 
  {
    printf("error initializing curl\n");
    exit(-1);
  }

  char query[300];
  sprintf(query,  "[out:json];area['name'='%s']->.a;relation['boundary'='administrative']['admin_level'='%d'](area.a);out geom;", region, admin_level);
    
  char *encoded_query = curl_easy_escape(curl, query, 0);
  char cache_file[2048];
  sprintf(cache_file, "cache/%s.json", encoded_query);

  if (access(cache_file, F_OK) != 0) 
    fetch_file(query);
 
  return load_json_file(cache_file);
}

int fetch_file(const char* query)
{
  CURL *curl;
  FILE *fp;
  CURLcode res;
  curl = curl_easy_init();
  
  if (!curl) return 1;


  char *encoded_query = curl_easy_escape(curl, query, 0);
  char url[2048];
  snprintf(url, sizeof(url), "https://overpass-api.de/api/interpreter?data=%s", encoded_query);

  char dst[2048];
  mkdir("cache", 0755);
  sprintf(dst, "cache/%s.json", encoded_query);

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

cJSON* load_json_file(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Unable to open file");
        return NULL;
    }

    // Determine file size
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer
    char* data = (char*)malloc(length + 1);
    if (!data) {
        fclose(fp);
        return NULL;
    }

    // Read file into buffer
    fread(data, 1, length, fp);
    fclose(fp);
    data[length] = '\0'; // Null-terminate

    // Parse the JSON
    cJSON* json = cJSON_Parse(data);
    
    // Free the buffer (cJSON creates its own copy of the data)
    free(data);

    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
    }

    return json;
}
