#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

// Callback function to handle the data received from the API
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    printf("%.*s", (int)total_size, (char *)ptr);
    return total_size;
}

int main(void) {
    CURL *curl;
    CURLcode res;

    // The Overpass query: get administrative relations in Catalonia
    const char *query = "[out:json];area['name'='Catalunya']->.a;relation['boundary'='administrative']['admin_level'='6'](area.a);out geom;";
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        // 1. URL encode the query string
        char *encoded_query = curl_easy_escape(curl, query, 0);
        
        // 2. Construct the full URL
        char url[2048];
        snprintf(url, sizeof(url), "https://overpass-api.de/api/interpreter?data=%s", encoded_query);
        // 3. Set up the request
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // 4. Perform the request
        printf("Fetching data from OpenStreetMap...\n\n");
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup
        curl_free(encoded_query);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
