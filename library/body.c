#include "body.h"
#include "list.h"
#include "polygon.h"
#include <math.h>
#include <assert.h>
#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

struct body {
  List *shape;
  double mass;
  RGBColor color;
  Vector centroid;
  Vector velocity;
  double ang_vel;
  double inertia;
  Vector force;
  Vector impulse;
  double torque;
  void *info;
  FreeFunc info_freer;
  bool is_removed;
  bool just_collided;
  char *text;
  bool colliding;
  bool unmoved;
  bool stop;
};

void body_set_unmoved(Body *body, bool b) {
  body->unmoved = b;
}

void body_set_mass(Body *body, double mass) {
  body->mass = mass;
}

Body *body_init(List *shape, double mass, RGBColor color) {
  Body *body = (Body *) malloc(sizeof(Body));
  assert(body != NULL);
  assert(mass > 0);
  body->shape = shape;
  body->mass = mass;
  body->inertia = 0;
  body->color = color;
  body->centroid = polygon_centroid(body->shape);

  // Setting to VEC_ZERO is an arbitrary choice.
  body->velocity = VEC_ZERO;
  body->ang_vel = 0;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->is_removed = false;
  body->colliding = false;
  body->unmoved = false;
  body->info = NULL;
  body->just_collided = false;
  body->stop = false;
  body->text = NULL;
  return body;
}

bool body_get_stop(Body *body) {
  return body->stop;
}

void body_set_stop(Body *body, bool b) {
  body->stop = b;
}

Body *body_init_with_info_and_text(List *shape, double mass, RGBColor color,
  void *info, FreeFunc info_freer, char *text) {
  Body *body = body_init(shape, mass, color);

  body->info = info;
  body->info_freer = info_freer;
  body->text = text;
  return body;
}

void body_free(Body *body) {
  list_free(body->shape);
  if (body->info != NULL) {
      body->info_freer(body->info);
  }
  free(body);
}

void body_set_inertia(Body *body, double in) {
  body->inertia = in;
}

List *body_get_shape(Body *body) {
  List *new_shape = list_init(list_capacity(body->shape), list_freer(body->shape));

  for (size_t i = 0; i < list_size(body->shape); i++) {
    Vector old_vec = *(Vector *) list_get(body->shape, i);
    Vector vec_copy = (Vector) {old_vec.x, old_vec.y};
    list_add(new_shape, (void *) vec_init(vec_copy));
  }

  return new_shape;
}

Vector body_get_centroid(Body *body) {
  return body->centroid;
}

Vector body_get_velocity(Body *body) {
  return body->velocity;
}

double body_get_mass(Body *body) {
  return body->mass;
}

RGBColor body_get_color(Body *body) {
  return body->color;
}

void body_translate(Body *body, Vector v) {
  polygon_translate(body->shape, v);
}

void body_set_shape(Body *body, List *shape) {
  body->shape = shape;
}

void body_set_centroid(Body *body, Vector x) {
  Vector translation = vec_subtract(x, polygon_centroid(body_get_shape(body)));
  body_translate(body, translation);
  body->centroid = x;
}

void body_set_velocity(Body *body, Vector v) {
  body->velocity = v;
}

void body_set_rotation(Body *body, double angle) {
  Vector cur_vel = body->velocity;
  double rotation_angle = angle - vec_get_angle(cur_vel); //ensures absolute, not relative, angles
  polygon_rotate(body->shape, rotation_angle, body_get_centroid(body));
}

void body_add_force(Body *body, Vector force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(Body *body, Vector impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_add_torque(Body *body, double torque) {
  body->torque = torque;
}

Vector lowest_point(Body *body) {
  List *polygon = body_get_shape(body);
  Vector lowest = VEC_ZERO;

  size_t size = list_size(polygon);
  for(size_t i = 0; i < size; i++) {
    Vector cur = *(Vector *)(list_get(polygon, i));
    if (cur.y > lowest.y) lowest = cur;
  }

  return lowest;
}

bool horizontal(Body *body) {
  List *polygon = body_get_shape(body);
  size_t size = list_size(polygon);
  Vector centroid = body_get_centroid(body);

  Vector top_left = centroid;
  Vector top_right = centroid;
  Vector bot_left = centroid;
  Vector bot_right = centroid;

  for(size_t i = 0; i < size; i++) {
    Vector cur = *(Vector *)(list_get(polygon, i));

    if (cur.x < top_left.x && cur.y < top_left.y) top_left = cur;
    if (cur.x < bot_left.x && cur.y > bot_left.y) bot_left = cur;
    if (cur.x > top_right.x && cur.y < top_right.y) top_right = cur;
    if (cur.x > bot_right.x && cur.y < bot_right.y) bot_right = cur;
  }

  if (top_left.x - top_right.x < 10) {
    if (top_right.x - top_left.x > bot_left.y - top_left.y) return true;
  }

  return false;
}

void body_tick(Body *body, double dt) {

  if (horizontal(body) == true) body->ang_vel = 0;
  if (vec_magnitude(body->velocity) > 0 || vec_magnitude(body->impulse) > 0) body->unmoved = false;
  if (vec_magnitude(body->impulse) == 0) body->colliding = false;

  //if (body->unmoved == true) printf("%s in body tick\n", body_get_text(body));

  if (body->stop == false && body->unmoved == false) {
    // change in momentum and velocity
    if (vec_magnitude(body->impulse) > 0 && body->colliding == false) body->colliding = true;
    else body->impulse = VEC_ZERO;

    Vector dp = vec_add(body->impulse, vec_multiply(dt, body->force));
    Vector dv = vec_multiply(1.0 / body->mass, dp); // change in velocity
    Vector new_velocity = vec_add(body->velocity, dv);
    body_set_velocity(body, new_velocity);

    // angular momentum and velocity
    if (body->inertia != 0.0) {
      body->ang_vel = body->ang_vel + body->torque * dt;
      polygon_rotate(body->shape, 2 * body->ang_vel * dt, lowest_point(body));
    }

    Vector translation = vec_multiply(dt, body->velocity);
    Vector new_centroid = vec_add(body->centroid, translation);
    body_set_centroid(body, new_centroid);

    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    body->torque = 0;
  }

  body->unmoved = false;
}

void body_remove(Body *body) {
  body->is_removed = true;
}

bool body_is_removed(Body *body) {
  return body->is_removed;
}

void *body_get_info(Body *body) {
  return body->info;
}

void body_set_color(Body *body, RGBColor c) {
  body->color = c;
}

void body_set_text(Body *body, char *text) {
  body->text = text;
}
char *body_get_text(Body *body) {
  return body->text;
}
