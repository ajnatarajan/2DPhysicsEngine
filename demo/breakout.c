#include "vector.h"
#include "list.h"
#include "polygon.h"
#include "breakout.h"
#include "scene.h"
#include "collision.h"
#include "forces.h"
#include "color.h"
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

const double SQRT2O2 = 0.70710678118;
const double ARC_DENSITY = 5;

const double MAX_RECTANGLES = 11.0;
const double ACTUAL_RECTANGLES = 10.0;
const double RECTANGLE_MASS = INFINITY;
const double COLOR_CONSTANT = 0;

const RGBColor COLORS[(int)ACTUAL_RECTANGLES] = {
    {.r = (double)255 / 255, .g = (double) 85/255, .b = (double) 6/255}, // red
    {.r = (double)254 / 255, .g = (double)112 / 255, .b = (double)12 / 255}, // orange
    {.r = (double)252 / 255, .g = (double)214 / 255, .b = (double)23 / 255}, // yellow
    {.r = (double)197 / 255, .g = (double)251 / 255, .b = (double)34 / 255}, // light green
    {.r = (double)56 / 255, .g = (double)248 / 255, .b = (double)72 / 255}, // green
    {.r = (double)78 / 255, .g = (double)245 / 255, .b = (double)231 / 255}, // sky blue
    {.r = (double)89 / 255, .g = (double)192 / 255, .b = (double)244 / 255}, // baby blue
    {.r = (double)131 / 255, .g = (double)110 / 255, .b = (double)241 / 255}, // indigo
    {.r = (double)190 / 255, .g = (double)120 / 255, .b = (double)239 / 255}, // lavender
    {.r = (double)238 / 255, .g = (double)130 / 255, .b = (double)238 / 255}, // violet
};

const double BALL_RADIUS = 10.0;
const double BALL_MASS = 2.0;
const Vector BALL_INIT_VELOCITY = {.x = -400 * SQRT2O2, .y = 400 * SQRT2O2};

const double SEPARATOR_FACTOR = MAX_RECTANGLES;
const double WHITE_SPACE_FACTOR = (MAX_RECTANGLES - ACTUAL_RECTANGLES) / (ACTUAL_RECTANGLES + 1.0);
const size_t NUM_ROWS = 3;

const Vector VEC_CENTER = {.x = 400, .y = 400};

const Vector PLAYER_VELOCITY = {.x = 600, .y = 0};
const double PLAYER_MASS = INFINITY;

const double COLOR_FACTOR = 1.6;
const double COLOR_THRESHOLD = 0.7 * 3;

double rand_double(double min, double max) {
  return (double)(((double) rand() / RAND_MAX * (max - min)) + min);
}

RGBColor rand_color(){
  return (RGBColor){rand_double(0, 1), rand_double(0, 1), rand_double(0, 1)};
}

double new_color_capped(double c) {
    if (COLOR_FACTOR * c > 1) {
        return 1;
    }
    else {
        return COLOR_FACTOR * c + COLOR_CONSTANT;
    }
}

// body1 should be the ball; body2 should be a brick
void brick_health_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  RGBColor c = body_get_color(body2);
  RGBColor new_c = (RGBColor) {new_color_capped(c.r), new_color_capped(c.g), new_color_capped(c.b)};
  printf("\nOld: %f %f %f\n", c.r, c.g, c.b);
  body_set_color(body2, new_c);
  printf("New: %f %f %f\n", new_c.r, new_c.g, new_c.b);
  if (new_c.r + new_c.g + new_c.b > COLOR_THRESHOLD) {

    body_remove(body2);
  }
}

Body *make_rectangle(Vector center, double width, double height, double mass, RGBColor color) {
    List *rectangle_points = list_rectangle(width, height);
    polygon_translate(rectangle_points, center);
    //Alliance *ally = ally_init(alliance->ally, alliance->bullet);
    return body_init(rectangle_points, mass, color);
}

void make_all_rectangles(Scene *scene, Vector min_window, Vector max_window) {
    double window_width = max_window.x - min_window.x;
    double window_height = max_window.y - min_window.y;

    double width = window_width / SEPARATOR_FACTOR;
    double height = window_height/ 20.0;

    double white_space = width * WHITE_SPACE_FACTOR;
    //Vector center = {.x = white_space, .y = window_height - height};
    Vector center = {.x = width/2.0 + white_space, .y = window_height - height/2.0};
    Body *ball = scene_get_body(scene, 1);

    for(size_t i = 0; i < ACTUAL_RECTANGLES; i++) {
      for(size_t j = 0; j < NUM_ROWS; j++) {
        Body *new_rectangle = make_rectangle(center, width, height, RECTANGLE_MASS, COLORS[i]);

        scene_add_body(scene, new_rectangle);
        create_physics_collision(scene, 1.0, new_rectangle, ball);
        create_collision(scene, ball, new_rectangle, brick_health_handler,
          NULL, free);
        // (scene, ball, new_rectangle);
        //create_destructive_collision(scene, new_rectangle, ball);

        Vector vert_decrement = {.x = 0, .y = -(height + white_space)};
        center = vec_add(center, vert_decrement);
      }
      center.x += (width + white_space);
      center.y = window_height - height/2.0;
    }
}

List *list_rectangle(double width, double height) {
    size_t num_points = 4;
    List *points = list_init(num_points, free);


    Vector pt1 = (Vector) {width/2.0, height/2.0};
    list_add(points, (void *)vec_init(pt1));

    Vector pt2 = (Vector) {-width/2.0, height/2.0};
    list_add(points, (void *)vec_init(pt2));

    Vector pt3 = (Vector) {-width/2.0, -height/2.0};
    list_add(points, (void *)vec_init(pt3));

    Vector pt4 = (Vector) {width/2.0, -height/2.0};
    list_add(points, (void *)vec_init(pt4));

    return points;
}

List *get_circle(double r, double angle1, double angle2) {
  double angle = angle1;
  size_t num_points = (size_t) ARC_DENSITY * r * (angle2 - angle1) / (2 * M_PI);
  List *points = list_init(num_points, free);

  for (size_t i = 0; i < num_points; i++) {
    Vector pt = (Vector) {r * cos(angle), r * sin(angle)};
    list_add(points, (void *)vec_init(pt));
    angle += (angle2 - angle1) / num_points;
  }

  return points;
}

void make_ball(Scene *scene, Vector min_window, Vector max_window) {
  double window_width = max_window.x - min_window.x;
  double window_height = max_window.y - min_window.y;

  double height = window_height / 20;

  List *ball_pts = get_circle(BALL_RADIUS, 0, 2 * M_PI);

  Vector center = (Vector) {window_width / 2, height * 2};
  polygon_translate(ball_pts, center);

  Body *ball = body_init(ball_pts, BALL_MASS, rand_color());
  body_set_velocity(ball, BALL_INIT_VELOCITY);
  Body *player = scene_get_body(scene, 0);


  scene_add_body(scene, ball);
  create_physics_collision(scene, 1.0, player, ball);

}

void make_player(Scene *scene, Vector min_window, Vector max_window, double mass) {
  Vector center = {.x = max_window.x/2.0, .y = max_window.y/20.0};
  double window_width = max_window.x - min_window.x;
  double window_height = max_window.y - min_window.y;
  double width = window_width / (SEPARATOR_FACTOR);
  double height = window_height / 20.0;
  Body *player = make_rectangle(center, width, height, mass, rand_color());
  scene_add_body(scene, player);
}

void make_all_barriers(Scene *scene, Vector min_window, Vector max_window, double mass) {
  // center, width, height, mass
  double window_width = max_window.x - min_window.x;
  double window_height = max_window.y - min_window.y;
  Vector left_center = {.x = -5, .y = window_height / 2.0};
  Vector right_center = {.x = window_width + 5, .y = window_height / 2.0};
  Vector top_center = {.x = window_width / 2.0, .y = window_height + 5};
  Body *left = make_rectangle(left_center, 10, window_height, mass, rand_color());
  Body *right = make_rectangle(right_center, 10, window_height, mass, rand_color());
  Body *top = make_rectangle(top_center, window_width, 10, mass, rand_color());
  scene_add_body(scene, left);
  scene_add_body(scene, right);
  scene_add_body(scene, top);

  Body *ball = scene_get_body(scene, 1);
  create_physics_collision(scene, 1.0, left, ball);
  create_physics_collision(scene, 1.0, right, ball);
  create_physics_collision(scene, 1.0, top, ball);
}

void prevent_player_off_screen(Scene *scene, Vector min_window, Vector max_window) {
  Body *player = scene_get_body(scene, 0);
  double window_width = max_window.x - min_window.x;
  double width = window_width / (SEPARATOR_FACTOR);
  if(body_get_centroid(player).x + width/2.0 >= max_window.x) {
    body_set_centroid(player, (Vector) {max_window.x - width/2.0,
      body_get_centroid(player).y});
  }
  if(body_get_centroid(player).x - width/2.0 <= min_window.x) {
    body_set_centroid(player, (Vector) {min_window.x + width/2.0,
      body_get_centroid(player).y});
  }
}

void restart_window(Scene *scene, Vector min_window, Vector max_window) {
  for(size_t i = 2; i < scene_bodies(scene); i++) {
    body_remove(scene_get_body(scene, i));
  }
  make_all_rectangles(scene, min_window, max_window);
  make_all_barriers(scene, min_window, max_window, INFINITY);
}

void respawn_ball(Scene *scene, Vector min_window, Vector max_window) {
  Body *ball = scene_get_body(scene, 1);
  double window_width = max_window.x - min_window.x;
  double window_height = max_window.y - min_window.y;

  if(body_get_centroid(ball).y < min_window.y) {
    double height = window_height / 20;
    Vector center = (Vector) {window_width / 2, height * 2};
    body_set_centroid(ball, center);
    body_set_velocity(ball, BALL_INIT_VELOCITY);
    restart_window(scene, min_window, max_window);
  }
}



void handler(char key, KeyEventType type, double held_time,
  Scene *scene, Body *body) {
    if(type == KEY_PRESSED) {
      switch(key) {
        case LEFT_ARROW:
          body_set_velocity(body, vec_negate(PLAYER_VELOCITY));
          break;
        case RIGHT_ARROW:
          body_set_velocity(body, PLAYER_VELOCITY);
          break;
      }
    } else {
      if(key == LEFT_ARROW || key == RIGHT_ARROW) {
        body_set_velocity(body, VEC_ZERO);
      }
    }
  }


int main(int argc, char* argv[]) {
  Vector min_window = {.x = 0, .y = 0};
  Vector max_window = {.x = WINDOW_WIDTH, WINDOW_HEIGHT};
  sdl_init(min_window, max_window);

  Scene *scene = scene_init();
  double timer = 0.0;

  make_player(scene, min_window, max_window, PLAYER_MASS);
  make_ball(scene, min_window, max_window);
  make_all_rectangles(scene, min_window, max_window);
  make_all_barriers(scene, min_window, max_window, INFINITY);

  Body *player = scene_get_body(scene, 0);
  sdl_on_key(handler);
  while (!sdl_is_done(scene, player).b) {
    double dt = time_since_last_tick();
    timer += dt;
    respawn_ball(scene, min_window, max_window);
    prevent_player_off_screen(scene, min_window, max_window);
    scene_tick(scene, dt);
    sdl_render_scene(scene);
  }

  scene_free(scene);
  return 0;
}
