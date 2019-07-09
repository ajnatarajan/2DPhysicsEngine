#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../include/sdl_wrapper.h"
#include "../include/scene.h"

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500



typedef struct alliance Alliance;

Alliance *ally_init(int ally, int bullet);

/* General shape-creating function which me, enemy ships, and all bullets are
 * made from.
 */
List *list_space_body(double r, double angle1, double angle2,
  double squash_factor, char direction);

/* Function to be called when making any object in this scene. Initializes the
 * body.
 */
Body *make_space_body(Vector center, double r, double angle1, double angle2,
  double squash_factor, double mass, RGBColor color, Alliance *alliance,
  char direction);

/* Create bodies for all enemy ships and add them to the scene. */
void make_all_enemies(Scene *scene, Vector min_window, Vector max_window);

/* Function to be called in main to check for any collisions and mark proper
 * bodies for removal based off whether they collided with anything.
 */
void check_any_collisions(Scene *scene);

/* Function to be called in check_any_collisions. Marks bodies for removal
 * based off whether "me" and an enemy bullet collide or "me" and enemy ship
 * collide.
 */
void check_collision_me(Scene *scene, List *enemy_bullets,
  List *enemy_ships);

/* Function to be called in check_any_collisions. Marks bodies for removal
 * based off whether an enemy ship and one of my bullets collide.
 */
void check_collision_enemies(Scene *scene, List *enemy_ships, List *my_bullets);


void make_me(Scene *scene, Vector min_window, Vector max_window);

/* Makes sure enemy ships are shifted down and move with opposite velocity uppon
 * hitting either side of the window.
 */
void wrap_around(Scene *scene, Vector min_window, Vector max_window);

/* Function to be called in wrap_around. */
bool enemy_touching_edge(Body *body, double radius, Vector min_window, Vector max_window);

void make_friend_bullet(Scene *scene);

void make_enemy_bullet(Scene *scene);

/* Return if "me" has been freed yet or not. */
bool find_me(Scene *scene);

/* Function to be called in prevent_me_off_screen. */
bool is_off_screen(Body *body, Vector min_window, Vector max_window);

/* Ensure player cannot move "me" beyond x bounds of screen. */
void prevent_me_off_screen(Vector min_window, Vector max_window);

void handler(char key, KeyEventType type, double held_time);
