/*
 * Simple SDL3 code to visualize the quad splitting and integrating process
 * Only one .poly file at a time
 */

#include "../struct_node.h"
#include <SDL3/SDL_render.h>
#include <math.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <stdio.h>

#define W 480
#define H 480

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;


float lats[4] = {0,0,1,1}; 
float lons[4] = {0,1,1,0}; 

float tx, ty, scl;

node n;

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

    n.len = 4;
    n.lats = lats;
    n.lons = lons;

    float mlat = min(lats, n.len);
    float mlon = min(lons, n.len);

    float Mlat = max(lats, n.len);
    float Mlon = max(lons, n.len);

    tx = - (mlon + Mlon)/2;
    ty = (mlat + Mlat)/2;

     scl = fmin(
        W / (Mlon-mlon),
        H / (Mlat-mlat)
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

    for(int i = 0; i<n.len; i++)
    {
      int x1 = W/2 + scl * (tx + n.lons[i]);
      int x2 = W/2 + scl * (tx + n.lons[(i+1)%n.len]);
      int y1 = H/2 - scl * (n.lats[i]-ty);
      int y2 = H/2 - scl * (n.lats[(i+1)%n.len]-ty);
      SDL_RenderLine(renderer, x1, y1, x2, y2);
    }

    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}


