#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include "sdl_wrapper.h"
#include "scene.h"

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500
#define MS_PER_S 1e3

/**
 * The coordinate at the center of the screen.
 */
Vector center;
/**
 * The coordinate difference from the center to the top right corner.
 */
Vector max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
KeyHandler key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;
/**
 * Stores whether the mouse is currently pressed (held down).
 */
bool flag = false;

TTF_Font *font = NULL;

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        case SDLK_SPACE: return SPACE_BAR;
        case SDLK_y: return YES;
        case SDLK_n: return NO;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(Vector min, Vector max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max)),
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);

    // addition is to initialize ttf
    TTF_Init();

    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
}

Bool_Coords sdl_is_done(Scene *scene, Body *body) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event);
    int x = 145;
    int y = WINDOW_HEIGHT - 140;
    bool event_done = false;
    while (SDL_PollEvent(event)) {

        // printf("polling event\n");
        switch (event->type) {
            // http://lazyfoo.net/SDL_tutorials/lesson09/index.php
            case SDL_MOUSEBUTTONDOWN:
                flag = true;
                x = event->button.x;
                y = event->button.y;
                event_done = true;
                break;
            case SDL_MOUSEMOTION:
                if(flag) {
                    x = event->button.x;
                    y = event->button.y;
                    //printf("Mouse motion - (%d, %d)\n", x, y);
                }
                event_done = true;
                break;
            case SDL_MOUSEBUTTONUP:
                //printf("mouse up!\n");
                flag = false;
                x = -1;
                y = -1;
                event_done = true;
                break;
            case SDL_QUIT:
                free(event);
                return (Bool_Coords){.b = true, .x = 0, .y = 0};
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (!key_handler) break;
                char key = get_keycode(event->key.keysym.sym);
                if (!key) break;

                double timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time =
                    (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type, held_time, scene, body);
                break;
            //default:
              //event_done = true;
              //break;
        }
        if(event_done) break;
    }
    free(event);
    //printf("%d %d\n", x, y);
    return (Bool_Coords){.b = false, .x = x, .y = y};
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(List *points, RGBColor color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width);
    assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;

    // Convert each vertex to a point on screen
    short *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points);
    assert(y_points);
    for (size_t i = 0; i < n; i++) {
        Vector *vertex = list_get(points, i);
        Vector pos_from_center =
            vec_multiply(scale, vec_subtract(*vertex, center));
        // Flip y axis since positive y is down on the screen
        x_points[i] = round(center_x + pos_from_center.x);
        y_points[i] = round(center_y - pos_from_center.y);
    }

    // Draw polygon with the given color
    filledPolygonRGBA(
        renderer,
        x_points, y_points, n,
        color.r * 255, color.g * 255, color.b * 255, 255
    );
    free(x_points);
    free(y_points);
}

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

void sdl_render_scene(Scene *scene) {
    sdl_clear();
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        List *shape = body_get_shape(body);
        sdl_draw_polygon(shape, body_get_color(body));
        list_free(shape);
        char *text = body_get_text(body);
        /* IMPORTANT NOTE: in sdl_ttf (0, 0) is top left but
         * for all other sdl function (0, 0) is top right
         * so unfortunately we have to account for that. */
        Vector centroid = body_get_centroid(body);
        Vector adjusted_centroid = (Vector) {centroid.x, WINDOW_HEIGHT - centroid.y};
        sdl_draw_text(text, adjusted_centroid);
    }
    
    sdl_show();
}

void sdl_on_key(KeyHandler handler) {
    key_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

void sdl_draw_text(char *txt, Vector centroid) {
  sdl_draw_permanent_text(txt, 20, (SDL_Color) {0, 0, 0}, centroid);
}

void sdl_draw_permanent_text(char *txt, int ptsize, SDL_Color color, Vector centroid) {
  font = TTF_OpenFont("arial.ttf", ptsize);
  SDL_Surface *message = TTF_RenderText_Solid(font, txt, color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, message);
  int text_width;
  int text_height;
  // finds values of text_width and text_height
  SDL_QueryTexture(texture, NULL, NULL, &text_width, &text_height);

  double left_x = centroid.x - text_width / 2;
  double left_y = centroid.y - text_height / 2;
	// upper left x coordinate, upper left y coordinate, width, height
  SDL_Rect rect = {left_x, left_y, text_width, text_height};
  SDL_Rect *dstrect = (SDL_Rect *) malloc(sizeof(SDL_Rect));
  *dstrect = rect;

  SDL_RenderCopy(renderer, texture, NULL, dstrect);
  TTF_CloseFont(font);
}

void sdl_clean_up(Scene *scene) {
  scene_free(scene);
  SDL_DestroyRenderer(renderer);
  TTF_Quit();
  SDL_Quit();
}
