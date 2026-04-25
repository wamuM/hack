#include <stdio.h>

#include "cJSON.h"
#include "fetcher.h"



int main(void) {

    const char *query = "[out:json];area['name'='Catalunya']->.a;relation['boundary'='administrative']['admin_level'='6'](area.a);out geom;";

    fetch_file(query);

    return 0;
}
