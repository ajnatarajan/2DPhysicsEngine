// 0 is sky, 1 is ground, 2 is slingshot, 3 is sling

#include "vector.h"
#include "list.h"
#include "polygon.h"
#include "existentialist_birds.h"
#include "scene.h"
#include "collision.h"
#include "forces.h"
#include "color.h"
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

// different sdl window states based off user input
#define Y 1
#define N 2
#define WAITING 3

typedef enum {
    PIG,
    BIRD,
    NONINTERACTIVE,
    GROUND,
    BLOCK
} BodyType;


struct game_state {
  bool window_closed;
  bool accept_key_presses;
  int continue_playing;
  bool slingshot_movable;
  bool ready_to_fire;
  bool has_been_fired;
  Vector release_point;
  double bird_delete_timer;
  double level_over_timer;
  int num_birds_remaining;
  bool victory;
  int level_status;
};

const RGBColor RED = {.r = (double)255 / 255, .g = (double) 0/255, .b = (double) 0/255};
const RGBColor ORANGE = {.r = (double)254 / 255, .g = (double)112 / 255, .b = (double)12 / 255};
const RGBColor SKY_BLUE = {.r = (double)135 / 255, .g = (double)206 / 255, .b = (double)250 / 255};
const RGBColor DIRT_BROWN = {.r = (double)139 / 255, .g = (double)69 / 255, .b = (double)19 / 255};
const RGBColor LIGHT_BROWN = {.r = (double)222 / 255, .g = (double)184 / 255, .b = (double)135 / 255};
const RGBColor CHOCOLATE = {.r = (double)210 / 255, .g = (double)105 / 255, .b = (double)30 / 255};
const RGBColor DRK_GREEN = {.r = (double)0 / 255, .g = (double)100 / 255, .b = (double)0 / 255};
const RGBColor CLD_WHITE = {.r = (double)255 / 255, .g = (double)255 / 255, .b = (double)255 / 255};

List *list_of_works;
Game_State *game_state;





double rand_double(double min, double max) {
  return (double)(((double) rand() / RAND_MAX * (max - min)) + min);
}

const double GROUND_LEVEL_PROPORTION = .1;
const double GROUND_HEIGHT = WINDOW_HEIGHT * GROUND_LEVEL_PROPORTION;
const double BLOCK_MASS = 5.0; // formerly WOOD_MASS
const double SLINGSHOT_SPACE_PROPORTION = .25;
const double ARC_DENSITY = 1;



BodyType get_type(Body *body) {
    return *((BodyType *) body_get_info(body));
}

Game_State *game_state_init() {
  Game_State *game_state = malloc(sizeof(Game_State));
  game_state->window_closed = true;
  game_state->accept_key_presses = true;
  game_state->continue_playing = WAITING;
  game_state->slingshot_movable = false;
  game_state->ready_to_fire = false;
  game_state->has_been_fired = false;
  game_state->release_point = (Vector) {145, 140};
  game_state->bird_delete_timer = 0.0;
  game_state->level_over_timer = 0.0;
  game_state->num_birds_remaining = 5;
  game_state->victory = false;
  return game_state;
}

List *list_circle(double r, double angle1, double angle2) {
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

Body *make_circle(Vector center, double r, double angle1, double angle2,
  double mass, RGBColor color, int info, char *text) {
    List *circle_pts = list_circle(r, angle1, angle2);
    polygon_translate(circle_pts, center);
    BodyType *info_to_pass = malloc(sizeof(BodyType));
    *info_to_pass = info;
    Body *b = body_init_with_info_and_text(circle_pts, mass, color,
      info_to_pass, free, text);
    body_set_inertia(b, 2.0 / 5.0 * mass * r * r);
    return b;
}

Body *make_triangle(Vector pt1, Vector pt2, Vector pt3, double mass,
  RGBColor color, int info) {
  List *tri = list_init(1, NULL);
  Vector *tri1 = malloc(sizeof(Vector));
  Vector *tri2 = malloc(sizeof(Vector));
  Vector *tri3 = malloc(sizeof(Vector));

  *tri1 = pt1;
  *tri2 = pt2;
  *tri3 = pt3;

  list_add(tri, tri1);
  list_add(tri, tri2);
  list_add(tri, tri3);

  BodyType *info_to_pass = malloc(sizeof(BodyType));
  *info_to_pass = info;
  Body *b = body_init_with_info_and_text(tri, mass, color,
    info_to_pass, free, NULL);
  //body_set_inertia(b, 1.0 / 3.0 * mass * height * height);
  return b;
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




Body *make_rectangle(Vector center, double width, double height, double mass,
  RGBColor color, int info) {
    List *rectangle_points = list_rectangle(width, height);
    polygon_translate(rectangle_points, center);
    BodyType *info_to_pass = malloc(sizeof(BodyType));
    *info_to_pass = info;
    Body *b = body_init_with_info_and_text(rectangle_points, mass, color,
      info_to_pass, free, NULL);
    body_set_inertia(b, 1.0 / 3.0 * mass * height * height);
    return b;
}

bool no_bird(Scene *scene) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    if(get_type(scene_get_body(scene, i)) == BIRD) return false;
  }
  return true;
}

void make_bird(Scene *scene, Vector center, double radius, double mass, RGBColor color){
    if(no_bird(scene) && game_state->num_birds_remaining > 0) {
      game_state->ready_to_fire = false;
      game_state->has_been_fired = false;
      game_state->release_point = (Vector) {145, 140};
      Body *bird = make_circle(center, radius, 0, 2*M_PI, mass, color, BIRD, "bird");

      List *useless = list_init(1, NULL);
      list_add(useless, bird);

      scene_add_body(scene, bird);
      create_down(scene, GRAVITY, useless);
      stop_at_ground(scene, bird, scene_get_body(scene, 1));
      for(size_t i = 0; i < scene_bodies(scene); i++) {
        Body *b = scene_get_body(scene, i);
        if(get_type(b) != NONINTERACTIVE) {
          create_physics_collision(scene, 1, bird, b);
        }
      }
    }
}

void make_pig(Scene *scene, Vector center, double radius, double mass,
  RGBColor color){
    Body *pig = make_circle(center, radius, 0, 2*M_PI, mass, color, PIG, "pig");
    body_set_velocity(pig, (Vector) {0, 0});
    List *useless = list_init(1, NULL);
    list_add(useless, pig);

    create_down(scene, GRAVITY, useless);
    stop_at_ground(scene, pig, scene_get_body(scene, 1));

    scene_add_body(scene, pig);
    for(size_t i = 0; i < scene_bodies(scene); i++) {
      Body *curr_body = scene_get_body(scene, i);
      if(get_type(curr_body) != NONINTERACTIVE) {
        create_inelastic_collision(scene, pig, curr_body);
        create_normal_force(scene, pig, curr_body, GRAVITY);
      }
    }
}

void make_cloud(Scene *scene, Vector center, double width, double height) {
  Body *cloud = make_rectangle(center, width, height, CLD_MASS, CLD_WHITE,
    NONINTERACTIVE);
  body_set_text(cloud, "cloud");
  scene_add_body(scene, cloud);
  body_set_velocity(cloud, (Vector) {.x = 150, .y = 0});
}

void cloud_wrap_around(Scene *scene, Vector min_window, Vector max_window) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr_body = scene_get_body(scene, i);
    if(get_type(curr_body) == NONINTERACTIVE) {
      Vector centroid = body_get_centroid(curr_body);
      Vector first_pt = *((Vector *)list_get(body_get_shape(curr_body), 0));
      double half_rect_width = fabs(centroid.x - first_pt.x);
      // off the left side of the screen, respawn on right
      if(centroid.x + half_rect_width <= min_window.x) {
        body_set_centroid(curr_body, (Vector) {max_window.x + half_rect_width,
          centroid.y});
      }
      // off the right side of the screen, respawn on left
      else if(centroid.x - half_rect_width >= max_window.x) {
        body_set_centroid(curr_body, (Vector) {min_window.x - half_rect_width,
          centroid.y});
      }
    }
  }
}

void make_clouds(Scene *scene, Vector min_window, Vector max_window) {
  make_cloud(scene, (Vector) {200, 425}, 150, 25);
  make_cloud(scene, (Vector) {350, 325}, 80, 25);
  make_cloud(scene, (Vector) {425, 450}, 120, 30);


  make_cloud(scene, (Vector) {550, 350}, 75, 35);
  make_cloud(scene, (Vector) {675, 200}, 100, 25);
  make_cloud(scene, (Vector) {900, 400}, 60, 40);
}

void make_background(Scene *scene, Vector min_window, Vector max_window) {
    double window_width = max_window.x - min_window.x;
    double window_height = max_window.y - min_window.y;

    Vector sky_center = (Vector){window_width / 2, window_height / 2};
    Body *sky = make_rectangle(sky_center, window_width, window_height,
      INFINITY, SKY_BLUE, NONINTERACTIVE);
    scene_add_body(scene, sky); //index 0

    Vector ground_center = (Vector){window_width / 2, GROUND_HEIGHT / 2};
    Body *ground = make_rectangle(ground_center, window_width, GROUND_HEIGHT,
      INFINITY, DIRT_BROWN, GROUND);
    scene_add_body(scene, ground); //index 1
}

void nothing_below_ground(Scene *scene, Vector min_window, Vector max_window) {
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr_body = scene_get_body(scene, i);
    int type = get_type(curr_body);
    if(type == PIG) {
      if(body_get_centroid(curr_body).y - PIG_RADIUS <= (min_window.y + GROUND_HEIGHT)) {
        body_set_centroid(curr_body, (Vector) {body_get_centroid(curr_body).x, GROUND_HEIGHT + PIG_RADIUS});
      }
    } else if(type == BIRD) {
      if(body_get_centroid(curr_body).y - BIRD_RADIUS <= (min_window.y + GROUND_HEIGHT)) {
        body_set_centroid(curr_body, (Vector) {body_get_centroid(curr_body).x, GROUND_HEIGHT + BIRD_RADIUS});
      }
    }
  }
}


// height is longer than width
// center is defined as the center of the bottom-most block (block1)
void make_tower(Scene *scene, double width, double height, Vector center){
  Vector center2 = vec_add((Vector){height/2 - width/2, height/2 + width/2},
    center);
  Vector center3 = vec_add((Vector){-height/2 + width/2, height/2 + width/2},
    center);

  Vector center4 = vec_add((Vector) {0, height}, center);
  double new_height = SCALE_UPPER_BLOCK * height;
  Vector center5 = vec_add((Vector){new_height/2 - width/2, new_height/2 - width / 2},
    center4);
  Vector center6 = vec_add((Vector){-new_height/2 + width/2, new_height/2 - width/2},
    center4);

  Body *block1 = make_rectangle(center, height, width, BLOCK_MASS, LIGHT_BROWN, BLOCK);
  Body *block2 = make_rectangle(center2, width, height, BLOCK_MASS, LIGHT_BROWN, BLOCK);
  Body *block3 = make_rectangle(center3, width, height, BLOCK_MASS, LIGHT_BROWN, BLOCK);
  Body *block4 = make_rectangle(center4, height*SCALE_UPPER_BLOCK, width,
    BLOCK_MASS, LIGHT_BROWN, BLOCK);
  Body *block5 = make_rectangle(center5, width, height*SCALE_UPPER_BLOCK,
    BLOCK_MASS, LIGHT_BROWN, BLOCK);
  Body *block6 = make_rectangle(center6, width, height*SCALE_UPPER_BLOCK,
    BLOCK_MASS, LIGHT_BROWN, BLOCK);

  List *list_of_bodies = list_init(5, NULL);
  list_add(list_of_bodies, block1);
  list_add(list_of_bodies, block2);
  list_add(list_of_bodies, block3);
  list_add(list_of_bodies, block4);
  list_add(list_of_bodies, block5);
  list_add(list_of_bodies, block6);

  create_down(scene, GRAVITY, list_of_bodies);
  for(size_t i = 0; i < 6; i++) {
    Body *block = list_get(list_of_bodies, i);
    scene_add_body(scene, block);
    stop_at_ground(scene, block, scene_get_body(scene, 1));

    for(size_t j = 0; j < scene_bodies(scene); j++) {
      Body *curr = scene_get_body(scene, j);
      if(get_type(curr) != NONINTERACTIVE) {
        create_inelastic_collision(scene, block, curr);
        create_normal_force(scene, block, curr, GRAVITY);
      }
    }
  }
}

void make_towers_and_pigs(Scene *scene, int num_towers, double width,
  double height, double pig_radius, Vector min_window, Vector max_window) {

    double center_y = GROUND_HEIGHT + width/2;
    List *centers = list_init(num_towers, NULL);

    for(size_t i = 0; i < num_towers; i++) {
      double prop = 0.2 + ((i+1) * 0.8) / (num_towers + 1);
      double *to_add = malloc(sizeof(double));
      *to_add = prop;
      list_add(centers, to_add);
    }

    for (size_t j = 0; j < num_towers; j++) {
      double prop = *(double *)list_get(centers, j);
      Vector center = (Vector){max_window.x * prop, center_y};
      make_tower(scene, width, height, center);

      //2 * pig_radius represents mass of pig since mass scales with size
      Vector pig_center = (Vector) {.x = center.x, .y = center.y + width/2 + pig_radius};
      Vector upper_pig_center = (Vector) {.x = center.x, .y = center.y +
        3.0/2.0 * width + height + pig_radius - 15}; //merge note-- .y can also be lowered by 10
      make_pig(scene, pig_center, pig_radius, PIG_MASS, DRK_GREEN);
      make_pig(scene, upper_pig_center, pig_radius, PIG_MASS, DRK_GREEN);
    }
    list_free(centers);
}

void check_slingshot_movable(Scene *scene, int mouse_x, int mouse_y) {
  size_t left_x = SLING_CENTER.x - SLING_DIMENSIONS.x / 2;
  size_t right_x = SLING_CENTER.x + SLING_DIMENSIONS.x / 2;
  size_t bot_y = SLING_CENTER.y - SLING_DIMENSIONS.y / 2;
  size_t top_y = SLING_CENTER.y + SLING_DIMENSIONS.y / 2;

  // specifically when the mouse is up
  if (mouse_x == -1 && mouse_y == WINDOW_HEIGHT + 1) {
    game_state->slingshot_movable = false;
  }

  // specifically when the mouse is down/being dragged
  if(left_x <= mouse_x && mouse_x <= right_x && bot_y <= mouse_y &&
    mouse_y <= top_y) {
    game_state->slingshot_movable = true;
  }

}

void redraw_slingshot(Scene *scene, int mouse_x, int mouse_y) {
  List *curr_tri = body_get_shape(scene_get_body(scene, 3));
  Vector *pt1 = list_get(curr_tri, 0);
  Vector *pt2 = list_get(curr_tri, 1);
  Vector *pt3 = malloc(sizeof(Vector));

  if(game_state->slingshot_movable) *pt3 = (Vector) {mouse_x, mouse_y};
  else *pt3 = (Vector) {145, 140};

  Body *new_sling = make_triangle(*pt1, *pt2, *pt3, INFINITY, RED, NONINTERACTIVE);

  scene_set_body(scene, 3, new_sling);

}

void make_slingshot(Scene *scene) {
    Vector slingshot_center = (Vector){150, 100};
    Body *slingshot = make_rectangle(slingshot_center, 25, 100, INFINITY, CHOCOLATE, NONINTERACTIVE);

    Vector sling_pt1 = (Vector) {SLING_CENTER.x + SLING_DIMENSIONS.x / 2,
      SLING_CENTER.y - SLING_DIMENSIONS.y / 2};
    Vector sling_pt2 = (Vector) {SLING_CENTER.x + SLING_DIMENSIONS.x / 2,
      SLING_CENTER.y + SLING_DIMENSIONS.y / 2};

    Vector sling_pt3 = (Vector) {SLING_CENTER.x - SLING_DIMENSIONS.x / 2,
        SLING_CENTER.y};

    Body *sling = make_triangle(sling_pt1, sling_pt2, sling_pt3, INFINITY,
      RED, NONINTERACTIVE);
    scene_add_body(scene, slingshot); // slingshot is at index 2
    scene_add_body(scene, sling); // sling is at index 3
}

Vector calculate_bird_leaving_velocity(Vector release_point) {
  /* We want this to be calculated: magnitude scales w distance, angle is based
   * off angle between release point and initial slingshot */
   Vector sling_base_pt = (Vector) {SLING_CENTER.x + SLING_DIMENSIONS.x / 2,
     SLING_CENTER.y};
   Vector difference = vec_subtract(sling_base_pt, release_point);
   double angle = vec_get_angle(difference);
   double mag = vec_magnitude(difference);
   if(difference.x < 0) {
     mag *= -1;
   }
   double x_vel = mag * SLINGSHOT_VELOCITY_FACTOR;
   double y_vel = x_vel * tan(angle);
   return (Vector) {x_vel, y_vel};
}

bool point_offscreen(Vector v, Vector min_window, Vector max_window) {
   return v.x < min_window.x || v.x > max_window.x || v.y < min_window.y || v.y > max_window.y;
}


void move_bird_on_slingshot(Scene *scene, Vector release_point) {
  Body *bird;
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *curr_body = scene_get_body(scene, i);
    // we want the latest bird on the slingshot
    if(get_type(curr_body) == BIRD) bird = curr_body;
  }
  /*if(bird == NULL) {
    make_bird(scene, (Vector) {.x = 50, .y = GROUND_HEIGHT}, 10, 20, DIRT_BROWN);
    for(size_t i = 0; i < scene_bodies(scene); i++) {
      Body *curr_body = scene_get_body(scene, i);
      // we want the latest bird on the slingshot
      if(get_type(curr_body) == BIRD) bird = curr_body;
    }
  }*/

  if(!game_state->ready_to_fire) {
    if(point_offscreen(game_state->release_point, (Vector) {0,0},
      (Vector) {WINDOW_WIDTH, WINDOW_HEIGHT})) {
      game_state->release_point = (Vector) {145, 140};
    }
    body_set_centroid(bird, game_state->release_point);
  } else {
    if(point_offscreen(game_state->release_point, (Vector) {0,0},
      (Vector) {WINDOW_WIDTH, WINDOW_HEIGHT})) {
      game_state->release_point = (Vector) {145, 140};
      game_state->ready_to_fire = false;
    } else {
      Vector leaving_velocity = calculate_bird_leaving_velocity(game_state->release_point);
      body_set_velocity(bird, leaving_velocity);

      List *useless = list_init(1, NULL);
      list_add(useless, bird);

      create_down(scene, GRAVITY, useless);

      for(size_t i = 0; i < scene_bodies(scene); i++) {
        Body *curr_body = scene_get_body(scene, i);
        if(get_type(curr_body) != NONINTERACTIVE) {
          create_physics_collision(scene, 1, bird, curr_body);
        }
      }
      game_state->has_been_fired = true;
    }

  }
}


void remove_pigs_and_birds(Scene *scene, Vector min_window, Vector max_window, double dt) {
  // remove pigs that are offscreen or touching the ground
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    Vector center;
    double radius;
    if(get_type(b) == PIG || get_type(b) == BIRD) {
      List *pig_pts = body_get_shape(b);
      Vector *pt = list_get(pig_pts, 0);
      center = body_get_centroid(b);
      radius = vec_magnitude((Vector) {.x = center.x - (*pt).x,
        .y = center.y - (*pt).y});
    }
    if(get_type(b) == PIG) {
      if(center.y - radius <= (min_window.x + GROUND_HEIGHT)) body_remove(b);
      else if(center.x + radius >= max_window.x) body_remove(b);
      else if(center.x - radius <= min_window.x) body_remove(b);
    }


    double vel = vec_magnitude(body_get_velocity(b));


    if(get_type(b) == BIRD) {
      if(center.x + radius >= max_window.x) {
        body_remove(b);
        game_state->num_birds_remaining--;
      }

      else if(center.x - radius <= min_window.x) {
        if(game_state->has_been_fired) {
          body_remove(b);
          game_state->num_birds_remaining--;
        }
      }
      else if(center.y - radius <= min_window.y) {
        if(game_state->has_been_fired) {
          body_remove(b);
          game_state->num_birds_remaining--;
        }
      }

      // remove bird if bird's stop flag is true
      if(body_get_stop(b)) game_state->bird_delete_timer += dt;
      if(game_state->bird_delete_timer > 1) {
        if(game_state->has_been_fired) {
          body_remove(b);
          game_state->bird_delete_timer = 0;
          game_state->num_birds_remaining--;
        }
      }
      else if(vel < MIN_VELOCITY_THRESHOLD && game_state->has_been_fired) {
        body_remove(b);
        game_state->num_birds_remaining--;
      }

    }
  }
}


void check_if_level_over(Scene *scene) {
  int num_pigs_left = 0;
  for(size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    if(get_type(b) == PIG) {
      num_pigs_left++;
      // if some pig is moving, the level is not done
      if(!(vec_magnitude(body_get_velocity(b)) < MIN_VELOCITY_THRESHOLD)) {
        game_state->level_status = STILL_GOING;
        return;
      }
    }
  }
  // if we've gotten to this point, no pigs are moving

  // if no pigs are left, we have won
  if(num_pigs_left == 0) {
    game_state->level_status = WON;
    return;
  } else if(game_state->num_birds_remaining == 0) {
    game_state->level_status = LOST;
    return;
  }
  // in any other situation (i.e. still have birds left)
  game_state->level_status = STILL_GOING;
}


void display_winning_interim_screen(Scene *scene) {
  sdl_draw_text("I suppose you won, but is there really any winning in life anyway", (Vector) {500, 150});
  sdl_draw_text("Continue to next level?", (Vector) {500, 250});
  sdl_draw_text("Press [y]/[n]", (Vector) {500, 350});
}

void display_losing_interim_screen(Scene *scene) {
  sdl_draw_text("You lost, keep waiting for Godot bud", (Vector) {500, 150});
  sdl_draw_text("Continue to next level?", (Vector) {500, 250});
  sdl_draw_text("Press [y]/[n]", (Vector) {500, 350});
}

void update_remaining_birds(Scene *scene) {
  Vector first_center = (Vector){150, 350};
  Vector second_center = (Vector) {235, 350};
  Body *first = make_circle(first_center, 5, 0, 2*M_PI, INFINITY, SKY_BLUE,
    NONINTERACTIVE, "Birds Remaining: ");
  char *string_num_birds = malloc(sizeof(char));
  switch(game_state->num_birds_remaining) {
    case 0:
      string_num_birds = "0";
      break;
    case 1:
      string_num_birds = "1";
      break;
    case 2:
      string_num_birds = "2";
      break;
    case 3:
      string_num_birds = "3";
      break;
    case 4:
      string_num_birds = "4";
      break;
    case 5:
      string_num_birds = "5";
      break;
    case 6:
      string_num_birds = "6";
      break;
  }
  Body *second = make_circle(second_center, 5, 0, 2*M_PI, INFINITY, SKY_BLUE,
    NONINTERACTIVE, string_num_birds);

  if(scene_bodies(scene) <= 10) {
    scene_add_body(scene, first);
    scene_add_body(scene, second);
  } else {
    scene_set_body(scene, 10, first);
    scene_set_body(scene, 11, second);
  }
}

void populate_list_of_works() {
  list_of_works = list_init(1, NULL);
  list_add(list_of_works, "The Stranger by Camus");
  list_add(list_of_works, "The Myth of Sisyphus by Camus");
  list_add(list_of_works, "Being and Nothingness by Sartre");
  list_add(list_of_works, "Existentialism is a Humanism by Sartre");
  list_add(list_of_works, "Nausea by Sartre");
  list_add(list_of_works, "Thus Spoke Zarathustra by Nietzsche");
  list_add(list_of_works, "Waiting for Godot by Beckett");
  list_add(list_of_works, "The Trial by Kafka");
  list_add(list_of_works, "The Metamorphosis by Kafka");
  list_add(list_of_works, "Fear and Trembling by Kierkegaard");
  list_add(list_of_works, "The Ethics of Ambiguity by de Beauvoir");
  list_add(list_of_works, "The Rebel by Camus");
  list_add(list_of_works, "The Fall by Camus");
  list_add(list_of_works, "On the Genealogy of Morality by Nietzsche");
  list_add(list_of_works, "Understanding Existentialism by Reynolds");
  list_add(list_of_works, "The Phenomenology of Spirit by Hegel");
  list_add(list_of_works, "Works of Love by Kierkegaard");
  list_add(list_of_works, "Being in the World by Binswanger");
  list_add(list_of_works, "Being and Time by Heidegger");
  list_add(list_of_works, "Man's Search for Meaning by Frankl");
}

void display_title_screen(Scene *scene, List *random_nums) {
  sdl_draw_permanent_text("Welcome to Existential Birds", 40, BLK,
    (Vector) {500, 50});
  sdl_draw_text("Press [y] to continue or [n] to quit", (Vector) {500, 150});
  sdl_draw_text("Recommended works to read before playing: ", (Vector) {500, 200});

  Vector starting_centroid = (Vector) {500, 250};
  for(size_t i = 0; i < list_size(random_nums); i++) {
    int index = *((int *)list_get(random_nums, i));
    sdl_draw_text(list_get(list_of_works, index), starting_centroid);
    starting_centroid = (Vector) {starting_centroid.x, starting_centroid.y + 50};
  }

}

void handler(char key, KeyEventType type, double held_time, Scene *scene,
  Body *body) {
    if(game_state->accept_key_presses) {
      if(type == KEY_PRESSED) {
        switch(key) {
          case YES:
            game_state->continue_playing = Y;
            break;
          case NO:
            game_state->continue_playing = N;
            break;
        }
      }
    }
}

bool num_in_lst(List *lst, int num) {
  for(size_t i = 0; i < list_size(lst); i++) {
    int curr_val = *((int *)(list_get(lst, i)));
    if(num == curr_val) return true;
  }
  return false;
}

void title_screen_wait(Scene *scene) {
  populate_list_of_works();

  // get three random integers to pick works to display on the title screen
  List *three_rand_nums = list_init(1, NULL);
  for(int i = 0; i < 3; i++) {
    int random = rand() % list_size(list_of_works);
    if(num_in_lst(three_rand_nums, random)) i--; //try again
    else {
      int *to_add = malloc(sizeof(int));
      *to_add = random;
      list_add(three_rand_nums, to_add);
    }

  }
  game_state->accept_key_presses = true;
  while(!(sdl_is_done(scene, scene_get_body(scene, 0)).b) &&
    game_state->continue_playing == WAITING) {
    sdl_clear();
    display_title_screen(scene, three_rand_nums);
    sdl_show();
  }
  game_state->accept_key_presses = false;
}



bool respond_to_title_input(Scene *scene) {
  if(game_state->continue_playing == WAITING || game_state->continue_playing == N) {
    sdl_clean_up(scene);
    exit(1);
    return false;
  } else game_state->continue_playing = WAITING;
  return true;
}

void make_level(Scene *scene, Vector min_window, Vector max_window) {
  double num_towers = (int) rand_double(MIN_NUM_TOWERS, MAX_NUM_TOWERS);

  // you get one bird per tower
  game_state->num_birds_remaining = num_towers;

  make_background(scene, min_window, max_window);
  make_slingshot(scene);
  make_clouds(scene, min_window, max_window);
  update_remaining_birds(scene);

  make_towers_and_pigs(scene, num_towers, BLOCK_WIDTH, BLOCK_HEIGHT, PIG_RADIUS,
    min_window, max_window);
  make_bird(scene, (Vector) {.x = 50, .y = 100}, BIRD_RADIUS, BIRD_MASS, DIRT_BROWN);
}



int main(int argc, char* argv[]) {
  game_state = game_state_init();

  Vector min_window = {.x = 0, .y = 0};
  Vector max_window = {.x = WINDOW_WIDTH, WINDOW_HEIGHT};
  sdl_init(min_window, max_window);

  Scene *title_scene = scene_init();
  sdl_on_key(handler);
  title_screen_wait(title_scene);
  if(!respond_to_title_input(title_scene)) return 0;
  scene_free(title_scene);
  Scene *scene;

  // main loop: goes through iterations of level/interim screen
  while(1) {
    scene = scene_init();
    if(sdl_is_done(scene, scene_get_body(scene, 0)).b) break;
    bool want_out = false;
    make_level(scene, min_window, max_window);


    time_since_last_tick();


    Bool_Coords done = sdl_is_done(scene, scene_get_body(scene, 3));
    int mouse_x = SLING_CENTER.x - SLING_DIMENSIONS.x / 2;
    int mouse_y = SLING_CENTER.y;
    game_state->release_point = (Vector) {mouse_x, mouse_y};

    // for one level
    while(!done.b) {
      mouse_x = done.x;
      mouse_y = WINDOW_HEIGHT - done.y;

      if(!(mouse_x == (SLING_CENTER.x - SLING_DIMENSIONS.x / 2) &&
        mouse_y == SLING_CENTER.y)) {
        // this is a flag that user has released mouse
        if(!(mouse_x == -1 && mouse_y == WINDOW_HEIGHT + 1)) {
          game_state->release_point.x = mouse_x;
          game_state->release_point.y = mouse_y;
        } else game_state->ready_to_fire = true;
      }

      double dt = time_since_last_tick();

      /* called every iteration since this function implicitly only carries out
       * stuff if another bird cannot be found on the map */
      make_bird(scene, (Vector) {.x = 50, .y = GROUND_HEIGHT}, BIRD_RADIUS,
        BIRD_MASS, DIRT_BROWN);

      check_slingshot_movable(scene, mouse_x, mouse_y);

      // only redraw if its not a null_event
      if(!(mouse_x == (SLING_CENTER.x - SLING_DIMENSIONS.x / 2) && SLING_CENTER.y)) {
        redraw_slingshot(scene, mouse_x, mouse_y);
      }

      // redraw back to usual form if user released mouse
      if(mouse_x == -1 && mouse_y == WINDOW_HEIGHT + 1) redraw_slingshot(scene,
        (SLING_CENTER.x - SLING_DIMENSIONS.x / 2), SLING_CENTER.y);

      if(!game_state->has_been_fired) move_bird_on_slingshot(scene, game_state->release_point);
      cloud_wrap_around(scene, min_window, max_window);
      remove_pigs_and_birds(scene, min_window, max_window, dt);
      update_remaining_birds(scene);
      check_if_level_over(scene);

      //display_num_remaining_birds(scene, min_window, max_window);

      scene_tick(scene, dt);
      sdl_render_scene(scene);

      if(game_state->level_status == WON || game_state->level_status == LOST) {
        game_state->level_over_timer += dt;
        if(game_state->level_over_timer > 1) {
          game_state->window_closed = false;
          game_state->level_over_timer = 0.0;
          break;
        }
      }
      done = sdl_is_done(scene, scene_get_body(scene, 3));
    }

    if(game_state->window_closed) break;




    // interim screen
    if(!(game_state->window_closed)) {
      game_state->window_closed = true;
      game_state->accept_key_presses = true;
      while(!(sdl_is_done(scene, scene_get_body(scene, 0)).b) &&
        game_state->continue_playing == WAITING) {
        if(game_state->level_status == WON) {
          sdl_clear();
          display_winning_interim_screen(scene);
          sdl_show();
        } else if(game_state->level_status == LOST) {
          sdl_clear();
          display_losing_interim_screen(scene);
          sdl_show();
        }

      }
      /* if continue_playing == WAITING after being kicked out of the while loop
       * then this means the window was closed. */

      if(game_state->continue_playing == WAITING || game_state->continue_playing == N) {
        want_out = true;
        break;
      }

      else {
        game_state->accept_key_presses = false;
        game_state->continue_playing = WAITING;
      }
    }

    if(want_out) break;

    // remove all bodies
    for(size_t i = 0; i < scene_bodies(scene); i++) {
      scene_free_body(scene, i);
      i--;
    }
    scene_free(scene);
    game_state->level_status = STILL_GOING;
  }
  scene_free(scene);
  return 0;
}
