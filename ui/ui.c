#include "../fetcher.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <linux/limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "../path.h" 
#include "../graph.h"
#include "../auto_completion.h"
#include "../challenge_generator.h"
#include "../deviation.h"

#define INPUT_BUFFER_SIZE 128
#define MAX_AUTOCOMPLETE 10

void mk_text(const char* txt, int x, int y, int align_left, int r, int g, int b);
void on_subtmitted_answer();

typedef struct Text Text;
struct Text 
{ 
  SDL_Texture *text_texture;
  int text_w, text_h;
  char text[INPUT_BUFFER_SIZE];
};

void update_texture(Text* text, int r, int g, int b);
void draw_text_no_bg(Text* text, int x, int y);

#define GAP 10

int get_deviation(int index);
void win();
static bool won = false;

int* state;
graph grph; 
int start_node_index;
int goal_node_index;
Path solution;

int* sent_regions;
int* deviation;

int num_sent = 0;

static TTF_Font *font = NULL;
static TTF_Font *win_font = NULL;
static Text input_text; 
// static Text tmp_txt;

void destroy_text(Text* text);
void draw_text(Text* text, int x, int y);
void on_input_changed();

static Text suggestions[MAX_AUTOCOMPLETE];
static int suggestion_len;
static char* suggestion_str[MAX_AUTOCOMPLETE];
static int suggestion_index[MAX_AUTOCOMPLETE];

#define W 1500
#define H 900

float minlon = 1000;
float minlat = 1000;
float maxlon = -1000;
float maxlat = -1000;

cJSON* obj;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

float tx, ty, scl;

char title[300];

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    srand ( time(NULL) );

    SDL_SetAppMetadata("Generalised Travle", "1.0", "");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Generalised Travle", W, H, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    SDL_StartTextInput(window);

    SDL_SetRenderVSync(renderer, -1);

    SDL_SetRenderLogicalPresentation(renderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX);
 
    char buff[200];
    sprintf(buff, "'%s'='%s'", argv[1], argv[2]);
     obj = get_json(buff, atoi(argv[3]), atoi(argv[4]));
    // obj = get_json("'ISO3166-1'='ES'", 2, 4);
    // obj = get_json("'ISO3166-2'='ES-CT'", 4, 7);

    if(obj == NULL)
    {
      printf("Invalid file\n");
      exit(1);
    }

    cJSON* elements = cJSON_GetObjectItem(obj, "elements");
    int num_elms = cJSON_GetArraySize(elements);
    printf("%d\n", num_elms);

    state = (int *) calloc(num_elms, sizeof(int));
    if(graph_create_from_cjson(obj, &grph, argv[5]))
    {
      printf("Graph creation failed!");
      exit(1);
    }
    generate_random_start_goal(&grph,3, &start_node_index, &goal_node_index, &solution);
    sprintf(title, "From %s to %s", grph.nodes[start_node_index].name, grph.nodes[goal_node_index].name);
    state[start_node_index] = 1;
    state[goal_node_index] = 3;

    sent_regions = (int *) calloc(num_elms-2, sizeof(int));
    deviation = (int *) calloc(num_elms-2, sizeof(int));

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
        W / (maxlon-minlon) * 0.95,
        H / (maxlat-minlat) * 0.8
        );

     printf("%f, %f, %f\n", tx, ty, scl);

     if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Load a font (ensure this file exists in your directory!)
    font = TTF_OpenFont("../font2.ttf", 24);
    win_font = TTF_OpenFont("../font.ttf", 84);
    if (!font || !win_font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    memset(&input_text, 0, sizeof(Text));
 //   memset(&tmp_txt, 0, sizeof(Text));
    for (int i = 0; i < MAX_AUTOCOMPLETE; i++)
      memset(&suggestions[i], 0, sizeof(Text));

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if(won)
    {
      if (event->type == SDL_EVENT_KEY_DOWN) 
        if (event->key.key == SDLK_SPACE) 
          return SDL_APP_SUCCESS; 

      return SDL_APP_CONTINUE;
    }

    // Handle Text Input
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        // Concatenate new characters to your buffer
        size_t current_len = strlen(input_text.text);
        if (current_len < INPUT_BUFFER_SIZE - 1) {
            strncat(input_text.text, event->text.text, INPUT_BUFFER_SIZE - current_len - 1);
            // printf("Current input: %s\n", input_text); // Debugging
            on_input_changed();
        }
    }

    // Handle Backspace (or other special keys)
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_BACKSPACE) {
            size_t len = strlen(input_text.text);
            if (len > 0) {
                input_text.text[len - 1] = '\0';
                // printf("Current input: %s\n", input_text); // Debugging
              on_input_changed();
            }
        }
        // You can add logic here to trigger your fetcher when Enter is pressed
        if (event->key.key == SDLK_RETURN || event->key.key == SDLK_KP_ENTER) {
          on_subtmitted_answer();
            // Call your logic to re-fetch with the new string here
        }
    }

    return SDL_APP_CONTINUE;
}

int max(int a, int b)
{
  return a < b ? b : a;
}

int red(int dv)
{
  if(dv == 0) return 200;
  if(dv == 1) return 128;
  return 0;
}

int green(int dv)
{
  if(dv == 0) return 0;
  if(dv == 1) return 128;
  return 200;
}

int blue(int dv)
{
  return 0;
}


/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  
    SDL_RenderClear(renderer);  

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  

    mk_text(title, 20, 20, 1, 0, 0, 0);

    cJSON* elements = cJSON_GetObjectItem(obj, "elements");
    int num_elms = cJSON_GetArraySize(elements);

    for(int i = 0; i<num_elms; i++)
    {
      if(!state[i]) continue;

      if(state[i] != 2)
        SDL_SetRenderDrawColor(renderer, 48 + 157 * (state[i] == 3), 48+ 157 * (state[i] == 3), 48 + 157 * (state[i] == 1), SDL_ALPHA_OPAQUE);
      else
      {
        int dv = get_deviation(i);
        SDL_SetRenderDrawColor(renderer, red(dv),green(dv),blue(dv), SDL_ALPHA_OPAQUE);
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
    
    int j = 0;
    for(int i = num_sent -1 ; i>=0 && j <= 20; i--, j++)
    {
      int id = sent_regions[i];
      int dv = deviation[i];
      char buf[255];
      sprintf(buf, "%d. %s", (i+1), grph.nodes[id].name);

      mk_text(buf, W - 50, H - 40 * j-60, 0, red(dv),green(dv),blue(dv));

    }


    draw_text(&input_text, 50, 50);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderLine(renderer, 50, 76, 50+max(input_text.text_w,50), 76);


    for(int i = 0; i<suggestion_len; i++)
      draw_text(&suggestions[i], 50, 50 + 50 * (i+1));

    if(won)
    {

     /* sprintf(tmp_txt.text, "");
      update_texture(&tmp_txt, 0, 0, 0);
      draw_text(&tmp_txt, W/2-tmp_txt.text_w/2, H/2);*/
      mk_text("Press <space> to exit", W/2, H/2, -1, 0,0,0);


      Text tmp_txt;
      sprintf(tmp_txt.text, "You WON!!!");
      SDL_Color black = {0, 0, 0, 255};
    if (tmp_txt.text_texture) SDL_DestroyTexture(tmp_txt.text_texture);

    // Create a surface from text
    SDL_Surface *surf = TTF_RenderText_Blended(win_font, tmp_txt.text, 0, black);
    
    // Create texture from surface
    tmp_txt.text_texture = SDL_CreateTextureFromSurface(renderer, surf);
    tmp_txt.text_w = surf->w;
    tmp_txt.text_h = surf->h;
    
    SDL_DestroySurface(surf);
      draw_text_no_bg(&tmp_txt, W/2-tmp_txt.text_w/2, H/2-100);
      
    }


    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
  cJSON_Delete(obj);
  destroy_text(&input_text);
//  destroy_text(&tmp_txt);
  for(int i = 0; i<MAX_AUTOCOMPLETE; i++)
    destroy_text(&suggestions[i]);
  
  if (font) TTF_CloseFont(font);
  if (win_font) TTF_CloseFont(win_font);
  TTF_Quit();
}

void update_texture(Text* text, int r, int g, int b)
{
  if (text->text_texture) {
    SDL_DestroyTexture(text->text_texture);
    text->text_texture = NULL;
}

if (strlen(text->text) == 0) {
    text->text_w = text->text_h = 0;
    return;
}

    SDL_Color black = {r, g, b, 255};

    // Create a surface from text
    SDL_Surface *surf = TTF_RenderText_Blended(font, text->text, 0, black);
    
    // Create texture from surface
    text->text_texture = SDL_CreateTextureFromSurface(renderer, surf);
    text->text_w = surf->w;
    text->text_h = surf->h;
    
    SDL_DestroySurface(surf);
}

void update_complition()
{
  suggestion_len = generate_suggestions(input_text.text, &grph, suggestion_str, suggestion_index, state, MAX_AUTOCOMPLETE);

  for(int i = 0; i<suggestion_len; i++)
    sprintf(suggestions[i].text, "%s" , suggestion_str[i]);
}

void on_input_changed()
{
  update_complition();
  update_texture(&input_text,0,0,0);
  for(int i = 0; i<suggestion_len; i++)
    update_texture(&suggestions[i],0,0,0);
}

void destroy_text(Text* text)
{
  if(text->text_texture) 
    SDL_DestroyTexture(text->text_texture);
}

void draw_text(Text* text, int x, int y)
{ 
  if (!text->text_texture) return;
  if(text->text[0] == '\0') return;

  SDL_FRect rect;
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  rect.x = x-5;
  rect.y = y-5;
  rect.w = text->text_w+10;
  rect.h = text->text_h+10;
  SDL_RenderFillRect(renderer, &rect);

  SDL_FRect dst = { x, y, (float)text->text_w, (float)text->text_h };
  SDL_RenderTexture(renderer, text->text_texture, NULL, &dst);
  
}


void draw_text_no_bg(Text* text, int x, int y)
{ 
  if (!text->text_texture) return;

  SDL_FRect dst = { x, y, (float)text->text_w, (float)text->text_h };
  SDL_RenderTexture(renderer, text->text_texture, NULL, &dst);
  
}

void on_selected_region(int region)
{
  state[region] = 2;
  sent_regions[num_sent] = region;
  deviation[num_sent++] = classify_node(&grph, region, solution.nodes, solution.len);

  // check if winning
  
  Path user_solution;
  masked_bfs(&user_solution, &grph, start_node_index, goal_node_index, state); 
  if(user_solution.len != 0)win();

}

void on_subtmitted_answer()
{
  if(suggestion_len > 0) // is good answer
  {
    // Clear input
    input_text.text[0] = '\0';
    input_text.text_w = 0;
    on_input_changed();
    // Select region

    on_selected_region(suggestion_index[0]);
  }
}

void mk_text(const char* txt, int x, int y, int align_left, int r, int g, int b)
{
  Text  tmp_txt = {0};
  snprintf(tmp_txt.text, INPUT_BUFFER_SIZE, "%s", txt);
  update_texture(&tmp_txt, r, g, b);
  if(align_left == 1)
    draw_text(&tmp_txt, x, y);
  else if(align_left == 0)
    draw_text(&tmp_txt, x-tmp_txt.text_w, y);
  else
    draw_text(&tmp_txt, x-tmp_txt.text_w/2, y);

  destroy_text(&tmp_txt);

}

void win()
{
  won = true;
}

int get_deviation(int index)
{
  for(int i = 0; i<grph.node_len-2; i++) 
  {
    if(sent_regions[i] == index)
      return deviation[i];
  }
  return -1;
}
