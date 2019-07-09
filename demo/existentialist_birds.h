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

// for level_status
#define WON 1
#define LOST 2
#define STILL_GOING 3

const Vector SLINGSHOT_CENTER = {.x = 100, .y = 250};
const Vector SLING_CENTER = (Vector){150, 140};
const double SLINGSHOT_VELOCITY_FACTOR = 3;
const double MIN_VELOCITY_THRESHOLD = 5;


// width, height
const Vector SLING_DIMENSIONS = {.x = 10, .y = 10};



const double MIN_NUM_TOWERS = 2.0;
const double MAX_NUM_TOWERS = 5.99;

const double BLOCK_WIDTH = 10.0;
const double BLOCK_HEIGHT = 100.0;
const double PIG_RADIUS = 10.0;
const double PIG_MASS = 20;
const double BIRD_RADIUS = 10.0;
const double BIRD_MASS = 20.0;
const double SCALE_UPPER_BLOCK = 1.2;
const double CLD_MASS = 30;


const double GRAVITY = 100;

const SDL_Color BLK = {0, 0, 0};

typedef struct game_state Game_State;


Game_State *game_state_init();

void make_bird(Scene *scene, Vector center, double radius, double mass,
  RGBColor color);

void make_tower(Scene *scene, double length, double width, Vector center);

void make_background(Scene *scene, Vector min_window, Vector max_window);

void make_slingshot(Scene *scene);

void make_pig(Scene *scene, Vector center, double radius, double mass, RGBColor color);

void make_ground(Scene *scene);

void make_towers_and_pigs(Scene *scene, int num_towers, double width,
  double height, double pig_radius, Vector min_window, Vector max_window);
