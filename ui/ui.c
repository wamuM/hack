/*
 * Simple SDL3 code to visualize the quad splitting and integrating process
 * Only one .poly file at a time
 */

#include "../fetcher.h"
#include <SDL3/SDL_render.h>
#include <math.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <stdio.h>

#define W 480
#define H 480

typedef struct Point Point;
struct Point
{
  float x; 
  float y;
};

Point bmin, bmax;

cJSON* obj;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;


float lats[4] = {0,0,1,1}; 
float lons[4] = {0,1,1,0}; 

float tx, ty, scl;
/*
float min(float* arr, int len)
{
  float z = arr[0];
  for(int i = 0; i<len; i++)
    if(arr[i] < z)
      z = arr[i];
  return z;
}


float max(float* arr, int len)
{
  float z = arr[0];
  for(int i = 0; i<len; i++)
    if(arr[i] > z)
      z = arr[i];
  return z;
}
*/
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example Quad", "1.0", "");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Example Quad", W, H, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(renderer, -1);

    SDL_SetRenderLogicalPresentation(renderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    obj = get_json("Catalunya", 7);

    float minlon = 1000;
    float minlat = 1000;
    float maxlon = -1000;
    float maxlat = -1000;

    cJSON* elements = cJSON_GetObjectItem(obj, "elements");
    int num_elms = cJSON_GetArraySize(elements);
    printf("%d\n", num_elms);

    for(int i = 0; i<num_elms; i++)
    {
      cJSON* item = cJSON_GetArrayItem(elements, i);
      cJSON_Print(item);
      cJSON* bounds =   cJSON_GetObjectItem(item, "bounds");
      float _minlat = atof(cJSON_GetStringValue(cJSON_GetObjectItem(bounds, "minlat")));
      float _maxlat = atof(cJSON_GetStringValue(cJSON_GetObjectItem(bounds, "maxlat")));
      float _minlon = atof(cJSON_GetStringValue(cJSON_GetObjectItem(bounds, "minlon")));
      float _maxlon = atof(cJSON_GetStringValue(cJSON_GetObjectItem(bounds, "maxlon")));
      if(_minlat < minlat) minlat = _minlat;
      if(_maxlat < maxlat) maxlat = _maxlat;
      if(_minlon < minlon) minlon = _minlon;
      if(_maxlon < maxlon) maxlon = _maxlon;
    }


    tx = - (minlon + maxlon)/2;
    ty = (minlat + maxlat)/2;

     scl = fmin(
        W / (maxlon-minlon),
        H / (maxlat-minlat)
        )*0.95;

     printf("%f, %f, %f\n", tx, ty, scl);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  
    SDL_RenderClear(renderer);  

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  


    cJSON* elements = cJSON_GetObjectItem(obj, "elements");
    int num_elms = cJSON_GetArraySize(elements);

    for(int i = 0; i<num_elms; i++)
    {
        cJSON* item = cJSON_GetArrayItem(elements, i);
        cJSON* members = cJSON_GetObjectItem(item, "members");
        int num_mems = cJSON_GetArraySize(members);

        for(int j = 0; j<num_mems; j++)
        {
          cJSON* member = cJSON_GetArrayItem(members, j);
          if(strcmp("way", cJSON_GetStringValue(cJSON_GetObjectItem(member, "type")))) continue;
          if(strcmp("outer", cJSON_GetStringValue(cJSON_GetObjectItem(member, "role")))) continue;
          cJSON* geometry = cJSON_GetObjectItem(member, "geometry");
          int num_pts = cJSON_GetArraySize(geometry);

          for(int k = 0; k<num_pts; k++)
          {
            cJSON* p1 = cJSON_GetArrayItem(geometry, k);
            cJSON* p2 = cJSON_GetArrayItem(geometry, (k+1) % num_pts);

            int x1 = W/2 + scl * (tx + atof(cJSON_GetStringValue(cJSON_GetObjectItem(p1, "lon"))));
            int x2 = W/2 + scl * (tx + atof(cJSON_GetStringValue(cJSON_GetObjectItem(p2, "lon"))));
            int y1 = H/2 - scl * (atof(cJSON_GetStringValue(cJSON_GetObjectItem(p1, "lat")))-ty);
            int y2 = H/2 - scl * (atof(cJSON_GetStringValue(cJSON_GetObjectItem(p2, "lat")))-ty);
            SDL_RenderLine(renderer, x1, y1, x2, y2);
          }
          
        }
    }


    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
  cJSON_Delete(obj);
}


