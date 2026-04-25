/*
 * Simple SDL3 code to visualize the quad splitting and integrating process
 * Only one .poly file at a time
 */


#include "../fetcher.h"
#include <SDL3/SDL_render.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>

#define GAP 10

int* state;

static TTF_Font *font = NULL;
static SDL_Texture *text_texture = NULL;
static int text_w = 0, text_h = 0;

void UpdateTextTexture(const char* text);


#define INPUT_BUFFER_SIZE 128
static char input_text[INPUT_BUFFER_SIZE] = {0};

#define W 1400
#define H 900

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

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Generalized Travler", "1.0", "");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Example Quad", W, H, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    SDL_StartTextInput(window);

    SDL_SetRenderVSync(renderer, -1);

    SDL_SetRenderLogicalPresentation(renderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX);
  
    obj = get_json("'ISO3166-2'='ES-CT'", 4, 7);

    if(obj == NULL)
    {
      printf("Invalid file\n");
      exit(1);
    }

    float minlon = 1000;
    float minlat = 1000;
    float maxlon = -1000;
    float maxlat = -1000;

    cJSON* elements = cJSON_GetObjectItem(obj, "elements");
    int num_elms = cJSON_GetArraySize(elements);
    printf("%d\n", num_elms);

    state = (int *) calloc(num_elms, sizeof(int));
    state[0] = 1;
    state[28] = 3;

    for(int i = 0; i<num_elms; i++)
    {
      cJSON* item = cJSON_GetArrayItem(elements, i);
      cJSON* bounds =   cJSON_GetObjectItem(item, "bounds");
      float _minlat = cJSON_GetNumberValue(cJSON_GetObjectItem(bounds, "minlat"));
      float _maxlat = cJSON_GetNumberValue(cJSON_GetObjectItem(bounds, "maxlat"));
      float _minlon = cJSON_GetNumberValue(cJSON_GetObjectItem(bounds, "minlon"));
      float _maxlon = cJSON_GetNumberValue(cJSON_GetObjectItem(bounds, "maxlon"));
      if(_minlat < minlat) minlat = _minlat;
      if(_maxlat > maxlat) maxlat = _maxlat;
      if(_minlon < minlon) minlon = _minlon;
      if(_maxlon > maxlon) maxlon = _maxlon;
    }


    tx = - (minlon + maxlon)/2;
    ty = (minlat + maxlat)/2;

     scl = fmin(
        W / (maxlon-minlon),
        H / (maxlat-minlat)
        )*0.95;

     printf("%f, %f, %f\n", tx, ty, scl);

     if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Load a font (ensure this file exists in your directory!)
    font = TTF_OpenFont("../font.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    // Handle Text Input
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        // Concatenate new characters to your buffer
        size_t current_len = strlen(input_text);
        if (current_len < INPUT_BUFFER_SIZE - 1) {
            strncat(input_text, event->text.text, INPUT_BUFFER_SIZE - current_len - 1);
            // printf("Current input: %s\n", input_text); // Debugging
        }
        UpdateTextTexture(input_text);
    }

    // Handle Backspace (or other special keys)
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_BACKSPACE) {
            size_t len = strlen(input_text);
            if (len > 0) {
                input_text[len - 1] = '\0';
                // printf("Current input: %s\n", input_text); // Debugging
            }
          UpdateTextTexture(input_text);
        }
        // You can add logic here to trigger your fetcher when Enter is pressed
        if (event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER) {
            printf("Submitting: %s\n", input_text);
            // Call your logic to re-fetch with the new string here
        }
    }

    return SDL_APP_CONTINUE;
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
      if(!state[i]) continue;

      SDL_SetRenderDrawColor(renderer, 48 + 157 * (state[i] == 3), 48 + 157 * (state[i] == 1), 48, SDL_ALPHA_OPAQUE);
      if(state[i] == 1 || state[i] == 3)
      {
        SDL_SetRenderScale(renderer, 1.5f, 1.5f); 
      }

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

        for(int k = 0; k<num_pts; k+=GAP)
        {
          cJSON* p1 = cJSON_GetArrayItem(geometry, k);
          cJSON* p2 = cJSON_GetArrayItem(geometry, k+GAP >= num_pts ? num_pts-1 : k+GAP);

          int x1 = W/2 + scl * (tx + cJSON_GetNumberValue(cJSON_GetObjectItem(p1, "lon")));
          int x2 = W/2 + scl * (tx + cJSON_GetNumberValue(cJSON_GetObjectItem(p2, "lon")));
          int y1 = H/2 - scl * (cJSON_GetNumberValue(cJSON_GetObjectItem(p1, "lat"))-ty);
          int y2 = H/2 - scl * (cJSON_GetNumberValue(cJSON_GetObjectItem(p2, "lat"))-ty);


          //printf("(%d, %d) - (%d, %d)\n", x1,y1,x2,y2);
          SDL_RenderLine(renderer, x1, y1, x2, y2);
        } 
      }
    }

    if (text_texture) {
        SDL_FRect dst = { 50, 50, (float)text_w, (float)text_h };
        SDL_RenderTexture(renderer, text_texture, NULL, &dst);
    }


    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
  cJSON_Delete(obj);
  if (text_texture) SDL_DestroyTexture(text_texture);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
}

void UpdateTextTexture(const char* text)
{
    if (text_texture) SDL_DestroyTexture(text_texture);
    if(strlen(text) == 0) return;
    // Create a surface from text
    SDL_Color white = {0, 0, 0, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, white);
    
    // Create texture from surface
    text_texture = SDL_CreateTextureFromSurface(renderer, surf);
    text_w = surf->w;
    text_h = surf->h;
    
    SDL_DestroySurface(surf);
}
