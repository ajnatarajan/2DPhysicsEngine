#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include <stdbool.h>
#include "color.h"
#include "list.h"
#include "scene.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

// Values passed to a key handler when the given arrow key is pressed
#define LEFT_ARROW 1
#define UP_ARROW 2
#define RIGHT_ARROW 3
#define DOWN_ARROW 4
#define SPACE_BAR 5
#define YES 6
#define NO 7

#define WINDOW_HEIGHT 500



typedef struct bool_coords Bool_Coords;

struct bool_coords {
    bool b;
    int x;
    int y;
};

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum {
    KEY_PRESSED,
    KEY_RELEASED
} KeyEventType;

typedef struct sdl_text Sdl_text;

SDL_Rect *sdl_text_get_dstrect(Sdl_text *text);

SDL_Surface *sdl_text_get_message(Sdl_text *text);

SDL_Texture *sdl_text_get_texture(Sdl_text *text);
/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*KeyHandler)(char key, KeyEventType type, double held_time,
  Scene *scene, Body *body);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(Vector min, Vector max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.

 *
 * @return true if the window was closed, false otherwise
 */
// bool sdl_is_done(Scene *scene, Body *body);
Bool_Coords sdl_is_done(Scene *scene, Body *body);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(List *points, RGBColor color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param textures and dstrects to get the messages and their locations to draw
 */
void sdl_render_scene(Scene *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, KeyEventType type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 puts("A pressed");
 *                 break;
 *             case UP_ARROW:
 *                 puts("UP pressed");
 *                 break;
 *         }
 *     }
 * }
 * int main(int argc, char **argv) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(KeyHandler handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

void sdl_draw_text(char *txt, Vector centroid);

void sdl_draw_permanent_text(char *txt, int ptsize, SDL_Color color, Vector centroid);

void sdl_clean_up(Scene *scene);

void update_message_positions(List *textures, List *dstrects, Vector centroid);


#endif // #ifndef __SDL_WRAPPER_H__
