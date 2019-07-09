#include "vector.h"
#include "list.h"
#include "polygon.h"
#include "spaceinvaders.h"
#include "scene.h"
#include "collision.h"
#include "forces.h"
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

const int ARC_DENSITY = 1;
const double SQUASH_ME_FACTOR = 0.2;
const double SQUASH_BULLET_FACTOR = 0.2;
const double SQUASH_ENEMY_FACTOR = 1.0;
const double EPSILON = .1;
const int ENEMY = 0;
const int FRIEND = 1;
const int NOT_BULLET = 0;
const int BULLET = 1;
const double ENEMY_SHIP_MASS = 20.0;
const double ME_MASS = 20.0;
const double BULLET_MASS = 5.0;
const double BULLET_RADIUS = 10.0;
const double BULLET_TIMER = 1.0;
const double GRAV = 20;

const Vector BULLET_VELOCITY = {.x = 0, .y = 200};
const Vector ENEMY_SHIP_VELOCITY = {.x = 150, .y = 0};
const Vector ME_VELOCITY = {.x = 400, .y = 0};

const double MAX_ENEMIES = 12.0;
const double ACTUAL_ENEMIES = 11.0;

const double SEPARATOR_FACTOR = MAX_ENEMIES * 2.0;
const double WHITE_SPACE_FACTOR = 2*(MAX_ENEMIES - ACTUAL_ENEMIES) / (ACTUAL_ENEMIES + 1.0);
const size_t NUM_ROWS = 3;
const double ENEMY_ANGLE_ONE = M_PI / 8;
const double ENEMY_ANGLE_TWO = 7 * M_PI / 8;

const RGBColor ENEMY_COLOR = {
  .r = 169.0/256,
  .g = 169.0/256,
  .b = 169.0/256
};
const RGBColor FRIEND_COLOR = {
  .r = 57.0/256,
  .g = 255.0/256,
  .b = 20.0/256
};

Body *me;
Scene *scene;

struct alliance {
  int ally;
  int bullet;
};

Alliance *ally_init(int ally, int bullet) {
  Alliance *new_alliance = malloc(sizeof(Alliance));
  new_alliance->ally = ally;
  new_alliance->bullet = bullet;
  return new_alliance;
}

List *list_space_body(double r, double angle1, double angle2, double squash_factor,
  char direction) {
    double angle = angle1;
    size_t num_points = (size_t) ARC_DENSITY * r * (angle2- angle1) / (2 * M_PI);
    List *points = list_init(num_points, free);

    for (size_t i = 0; i < num_points; i++) {
      Vector pt = VEC_ZERO;
      if(direction == 'x') {
        pt = (Vector) {squash_factor * r * cos(angle), r * sin(angle)};
      } else if(direction == 'y') {
        pt = (Vector) {r * cos(angle), squash_factor * r * sin(angle)};
      } else {
        pt = (Vector) {r * cos(angle), r * sin(angle)};
      }
      list_add(points, (void *)vec_init(pt));
      angle += (angle2 - angle1) / num_points;
    }
    list_add(points, (void *)vec_init(VEC_ZERO));
    return points;
}

Body *make_space_body(Vector center, double r, double angle1, double angle2,
  double squash_factor, double mass, RGBColor color, Alliance *alliance,
  char direction) {
    List *space_body_pts = list_space_body(r, angle1, angle2, squash_factor,
      direction);
    polygon_translate(space_body_pts, center);
    Alliance *ally = ally_init(alliance->ally, alliance->bullet);
    return body_init_with_info(space_body_pts, mass, color, ally, free);
}

void make_all_enemies(Scene *scene, Vector min_window, Vector max_window) {
    double window_width = max_window.x - min_window.x;
    double window_height = max_window.y - min_window.y;
    double radius = window_width / SEPARATOR_FACTOR;
    double white_space = radius * WHITE_SPACE_FACTOR;
    Vector center = {.x = radius + white_space, .y = window_height - radius};
    Alliance *enemy = ally_init(ENEMY, NOT_BULLET);
    for(size_t i = 0; i < ACTUAL_ENEMIES; i++) {
      for(size_t j = 0; j < NUM_ROWS; j++) {
        // 'a' is chosen as an arbitrary value such that nothing is squashed.
        Body *new_enemy = make_space_body(center, radius, ENEMY_ANGLE_ONE,
          ENEMY_ANGLE_TWO, SQUASH_ENEMY_FACTOR, ENEMY_SHIP_MASS, ENEMY_COLOR,
          enemy, 'a');
        body_set_velocity(new_enemy, ENEMY_SHIP_VELOCITY);
        scene_add_body(scene, new_enemy);
        create_destructive_collision(scene, new_enemy, me);


        Vector vert_decrement = {.x = 0, .y = -radius - white_space};
        center = vec_add(center, vert_decrement);
      }
      center.x += (2 * radius + white_space);
      center.y = window_height - radius;
    }
}

void make_me(Scene *scene, Vector min_window, Vector max_window){
  double window_width = max_window.x - min_window.x;
  Alliance *ally = ally_init(FRIEND, NOT_BULLET);
  double radius = window_width / SEPARATOR_FACTOR;
  Vector center = {.x = window_width / 2.0, .y = radius};

  me = make_space_body(center, radius, ENEMY_ANGLE_ONE, ENEMY_ANGLE_TWO,
    SQUASH_ENEMY_FACTOR, ME_MASS, FRIEND_COLOR, ally, 'a');
  scene_add_body(scene, me);
}

void wrap_around(Scene *scene, Vector min_window, Vector max_window) {
  double window_width = max_window.x - min_window.x;
  double radius = window_width / SEPARATOR_FACTOR;
  double white_space = radius * WHITE_SPACE_FACTOR;
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr = scene_get_body(scene, i);
    if(((Alliance *)body_get_info(curr))->ally == ENEMY &&
      ((Alliance *)body_get_info(curr))->bullet == NOT_BULLET) {
        if(enemy_touching_edge(curr, radius, min_window, max_window)) {
          Vector new_centroid = {
            .x = (body_get_centroid(curr)).x,
            .y = (body_get_centroid(curr)).y - (NUM_ROWS*(radius + white_space))
          };
          body_set_centroid(curr, new_centroid);
          body_set_velocity(curr, vec_negate(body_get_velocity(curr)));
        }
    }
  }

}

bool enemy_touching_edge(Body *body, double radius, Vector min_window, Vector max_window) {
  if(body_get_centroid(body).x + radius*cos(ENEMY_ANGLE_ONE) >= max_window.x) {
    return true;
  } else if(body_get_centroid(body).x + radius*cos(ENEMY_ANGLE_TWO) <= min_window.x) {
    return true;
  }
  return false;
}

void make_friend_bullet(Scene *scene) {
  double radius = BULLET_RADIUS;
  Alliance *ally = ally_init(FRIEND, BULLET);
  Vector center = body_get_centroid(me);

  Body *new_bullet = make_space_body(center, radius, 0, 2*M_PI+EPSILON, SQUASH_BULLET_FACTOR,
    BULLET_MASS, FRIEND_COLOR, ally, 'x');

  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr = scene_get_body(scene, i);
    if(((Alliance *)body_get_info(curr))->ally == ENEMY &&
      ((Alliance *)body_get_info(curr))->bullet == NOT_BULLET) {
          create_destructive_collision(scene, new_bullet, curr);
    }

  }
  body_set_velocity(new_bullet, BULLET_VELOCITY);

  scene_add_body(scene, new_bullet);
}

void make_enemy_bullet(Scene *scene) {
  double radius = BULLET_RADIUS;
  Alliance *enemy = ally_init(ENEMY, BULLET);

  List *enemy_ships = list_init(scene_bodies(scene), (FreeFunc)body_free);

  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr = scene_get_body(scene, i);
    if(((Alliance *)body_get_info(curr))->ally == ENEMY &&
      ((Alliance *)body_get_info(curr))->bullet == NOT_BULLET) {
      list_add(enemy_ships, curr);
    }
  }

  // Without the following code block, if you try to make bullets when there
  // exist no more ships, the program's literally going to die (heap buffer overflow).
  if (list_size(enemy_ships) == 0) {
      return;
  }

  int index = (((double) rand() / RAND_MAX * (list_size(enemy_ships))));

  // index of 0 refers to the only friendly body, so literally any other index is probably an enemy
  Vector center = body_get_centroid((Body *)list_get(enemy_ships, index));
  Body *new_bullet = make_space_body(center, radius, 0, 2*M_PI+EPSILON, SQUASH_BULLET_FACTOR,
    BULLET_MASS, ENEMY_COLOR, enemy, 'x');
  create_destructive_collision(scene, new_bullet, me);

  body_set_velocity(new_bullet, vec_negate(BULLET_VELOCITY));

  List *l = list_init(1, NULL);
  list_add(l, new_bullet);
  create_down(scene, GRAV, l);

  scene_add_body(scene, new_bullet);
}

bool find_me(Scene *scene) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *body = scene_get_body(scene, i);
    if (((Alliance *)body_get_info(body))->ally == FRIEND &&
      ((Alliance *)body_get_info(body))->bullet == NOT_BULLET) {
      return true;
    }
  }
  return false;
}

bool is_off_screen(Body *body, Vector min_window, Vector max_window) {
  if(body_get_centroid(body).y <= min_window.y) return true;
  if(body_get_centroid(body).y >= max_window.y) return true;
  return false;
}

void remove_off_screens(Vector min_window, Vector max_window) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    if(is_off_screen(scene_get_body(scene, i), min_window, max_window)) {
      body_remove(scene_get_body(scene, i));
    }
  }
}

void prevent_me_off_screen(Vector min_window, Vector max_window) {
  double window_width = max_window.x - min_window.x;
  double radius = window_width / SEPARATOR_FACTOR;
  if(body_get_centroid(me).x + radius >= max_window.x) {
    body_set_centroid(me, (Vector) {max_window.x - radius,
      body_get_centroid(me).y});
  }
  if(body_get_centroid(me).x - radius <= min_window.x) {
    body_set_centroid(me, (Vector) {min_window.x + radius,
      body_get_centroid(me).y});
  }
}

void handler(char key, KeyEventType type, double held_time) {
  if(find_me(scene)) {
    if(type == KEY_PRESSED) {
      switch(key) {
        case LEFT_ARROW:
          body_set_velocity(me, vec_negate(ME_VELOCITY));
          break;
        case RIGHT_ARROW:
          body_set_velocity(me, ME_VELOCITY);
          break;
        case SPACE_BAR:
          make_friend_bullet(scene);
          break;
      }
    } else {
      if(key == LEFT_ARROW || key == RIGHT_ARROW) {
        body_set_velocity(me, VEC_ZERO);
      }
    }
  }
}


int main(int argc, char* argv[]) {
  Vector min_window = {.x = 0, .y = 0};
  Vector max_window = {.x = WINDOW_WIDTH, WINDOW_HEIGHT};
  sdl_init(min_window, max_window);
  scene = scene_init();
  double timer = 0.0;

  make_me(scene, min_window, max_window);
  make_all_enemies(scene, min_window, max_window);

  sdl_on_key(handler);
  while (!sdl_is_done()) {
    double dt = time_since_last_tick();
    timer += dt;

    if(find_me(scene)) {
      prevent_me_off_screen(min_window, max_window);
      if (timer >= BULLET_TIMER) {
        make_enemy_bullet(scene);
        timer = 0;
      }
    }


    wrap_around(scene, min_window, max_window);
    remove_off_screens(min_window, max_window);
    scene_tick(scene, dt);

    sdl_render_scene(scene);
  }

  scene_free(scene);
  return 0;
}
