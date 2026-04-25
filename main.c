#include <stdio.h>

#include "cJSON.h"
#include "fetcher.h"



int main(void) {

  cJSON* obj = get_json("'ISO3166-1'='ES'", 2, 7);



  return 0;
}
