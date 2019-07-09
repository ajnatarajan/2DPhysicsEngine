#include "forces.h"
#include "collision.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

const double ERROR = 10;

struct collision_type {
  Two_Bodies *two_body;
  void *aux;
  CollisionHandler handler;
};

struct two_bodies {
  Body *body1;
  Body *body2;
  double constant;
};

struct one_body {
  Body *body;
  double constant;
};

// A generalization of one_body and two_bodies.
struct n_bodies {
    List *bodies;
    double constant;
};

// Used in force creation methods below to avoid code duplication.
List *two_bodies_list_init(Body *b1, Body *b2) {
    List *l = list_init(2, NULL);
    list_add(l, b1);
    list_add(l, b2);
    return l;
}

CollisionType *collision_type_init(Two_Bodies *two_body, CollisionHandler h, void *aux) {
  CollisionType *c_type = (CollisionType *) malloc(sizeof(CollisionType));
  assert(c_type != NULL);
  c_type->two_body = two_body;
  c_type->handler = h;
  c_type->aux = aux;
  return c_type;
}

N_Bodies *n_bodies_init(List *l, double constant) {
    N_Bodies *n_bodies = (N_Bodies *)malloc(sizeof(N_Bodies));
    assert(n_bodies != NULL);
    n_bodies->bodies = l;
    n_bodies->constant = constant;
    return n_bodies;
}

Two_Bodies *two_bodies_init(Body *body1, Body *body2, double constant) {
  Two_Bodies *two_bodies = (Two_Bodies *)malloc(sizeof(Two_Bodies));
  assert(two_bodies != NULL);
  two_bodies->body1 = body1;
  two_bodies->body2 = body2;
  two_bodies->constant = constant;
  return two_bodies;
}

One_Body *one_body_init(Body *body, double constant) {
  One_Body *one_body = (One_Body *)malloc(sizeof(One_Body));
  assert(one_body != NULL);
  one_body->body = body;
  one_body->constant = constant;
  return one_body;
}

void n_bodies_free(N_Bodies *n_bodies) {
    list_free(n_bodies->bodies);
}

void two_bodies_free(Two_Bodies *two_bodies) {
  body_free(two_bodies->body1);
  body_free(two_bodies->body2);
}

void one_body_free(One_Body *one_body) {
  body_free(one_body->body);
}

size_t n_bodies_size(N_Bodies *n) {
    return list_size(n->bodies);
}

Body *n_bodies_get(N_Bodies *n, size_t i) {
    return (Body *)list_get(n->bodies, i);
}

Vector get_difference(Two_Bodies *two_body) {
  Body *body1 = two_body->body1;
  Body *body2 = two_body->body2;

  Vector centroid1 = body_get_centroid(body1);
  Vector centroid2 = body_get_centroid(body2);

  return vec_subtract(centroid2, centroid1);
}

double dist_squared(Two_Bodies *two_body) {
  return pow(vec_magnitude(get_difference(two_body)), 2);
}

void apply_force(Two_Bodies *two_body, double force_magnitude) {
  Body *body1 = two_body->body1;
  Body *body2 = two_body->body2;

  Vector diff = get_difference(two_body);
  double dist = vec_magnitude(diff);
  Vector dir = vec_multiply(1.0 / dist, diff);

  if(dist > DIST_TOO_SMALL) {
    body_add_force(body1, vec_multiply(force_magnitude, dir));
    body_add_force(body2, vec_multiply(-force_magnitude, dir));
  }
}

void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2) {
  Two_Bodies *aux = two_bodies_init(body1, body2, G);
  List *l = two_bodies_list_init(body1, body2);
  scene_add_bodies_force_creator(scene, (ForceCreator) create_gravity_force,
  (void *) aux, l, (FreeFunc) free);
}

void create_gravity_force(Two_Bodies *two_body) {
  Body *b1 = two_body->body1;
  Body *b2 = two_body->body2;

  double force = two_body->constant * body_get_mass(b1) * body_get_mass(b2)
  / dist_squared(two_body);

  apply_force(two_body, force);
}

void create_spring(Scene *scene, double k, Body *body1, Body *body2) {
  Two_Bodies *aux = two_bodies_init(body1, body2, k);
  List *l = two_bodies_list_init(body1, body2);
  scene_add_bodies_force_creator(scene, (ForceCreator) create_spring_force,
  (void *) aux, l, (FreeFunc) free);
}


void create_spring_force(Two_Bodies *two_body) {
  double force =  two_body->constant * vec_magnitude(get_difference(two_body));
  apply_force(two_body, force);
}

void create_drag(Scene *scene, double gamma, Body *body) {
  One_Body *aux = one_body_init(body,gamma);
  List *l = list_init(1, NULL);
  list_add(l, body);
  scene_add_bodies_force_creator(scene, (ForceCreator) create_drag_force,
    (void *) aux, l, (FreeFunc) free);
}

void create_drag_force(One_Body *one_body) {
  Body *body = one_body->body;
  body_add_force(body, vec_multiply(-one_body->constant, body_get_velocity(body)));
}

void create_down(Scene *scene, double g, List *l) {
    N_Bodies *aux = n_bodies_init(l, g);
    scene_add_bodies_force_creator(scene, (ForceCreator) create_down_force,
        (void *) aux, l, (FreeFunc) free);
}

void create_down_force(N_Bodies *nb) {
  Vector dir = {.x = 0, .y = -1};
  for (int i = 0; i < list_size(nb->bodies); i++) {
      Body *body = n_bodies_get(nb, i);
      body_add_force(body, vec_multiply(body_get_mass(body) * nb->constant, dir));
  }
}

void destructive_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void semidestructive_handler(Body *body1, Body *body2, Vector axis, void *aux) {
    body_remove(body2);
}

void physics_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  double u1 = vec_dot(body_get_velocity(body1), axis);
  double u2 = vec_dot(body_get_velocity(body2), axis);
  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);
  double reduced_mass;
  if (m1 == INFINITY) reduced_mass = m2;
  else if (m2 == INFINITY) reduced_mass = m1;
  else reduced_mass = m1 * m2 / (m1 + m2);

  Two_Bodies *two_body = (Two_Bodies *) aux;
  double cr = two_body->constant; // coefficient of restitution

  double impulse = reduced_mass * (1 + cr) * (u2 - u1);
  body_add_impulse(body1, vec_multiply(impulse, axis));
  body_add_impulse(body2, vec_multiply(-impulse, axis));


  if (impulse > ERROR) {
    // printf("%f \n", impulse);
    if (m1 != INFINITY) body_add_torque(body1, impulse);
    if (m2 != INFINITY) body_add_torque(body2, -impulse);
  }
}

void inelastic_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);

  if (m1 != INFINITY && m2 != INFINITY) {
    Vector v1 = body_get_velocity(body1);
    Vector v2 = body_get_velocity(body2);

    Vector tot_mom = vec_add(vec_multiply(m1, v1), vec_multiply(m2, v2));
    Vector tot_vel = vec_multiply(1 / (m1 + m2), tot_mom);

    if (vec_magnitude(v1) > ERROR || vec_magnitude(v2) > ERROR) {
      body_set_velocity(body1, tot_vel);
      body_set_velocity(body2, tot_vel);
    }
  }
  else {
    body_set_velocity(body1, VEC_ZERO);
    body_set_velocity(body2, VEC_ZERO);
  }
}

void normal_force_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  // Switches body1 and body2 to ensure body1 is on top
  if (body_get_centroid(body1).y > body_get_centroid(body2).y) {
    Body *temp = body1;
    body1 = body2;
    body2 = temp;
  }

  if (body_get_mass(body1) == INFINITY) {
    Body *temp = body1;
    body1 = body2;
    body2 = temp;
  }

  body_set_unmoved(body1, true);
}

void stop_ground_handler(Body *body1, Body *body2, Vector axis, void *aux) {
  body_set_stop(body1, true);
}

void force_creator(void *aux) {
  CollisionType *c = (CollisionType *) aux;
  Two_Bodies *two_body = c->two_body;
  Body *body1 = two_body->body1;
  Body *body2 = two_body->body2;
  void *aux_pass = c->aux;
  CollisionHandler handler = c->handler;
  CollisionInfo info = find_collision(body_get_shape(body1), body_get_shape(body2));

  if (info.collided == true) {
    handler(body1, body2, info.axis, aux_pass);
  }
}

void create_collision(Scene *scene, Body *body1, Body *body2,
    CollisionHandler handler, void *aux, FreeFunc freer) {
      Two_Bodies *two_body = two_bodies_init(body1, body2, 0);
      CollisionType *c = collision_type_init(two_body, handler, aux);
      List *bodies = two_bodies_list_init(body1, body2);
      scene_add_bodies_force_creator(scene, (ForceCreator) force_creator, c,
      bodies, freer);
}

void create_destructive_collision(Scene *scene, Body *body1, Body *body2){
  Two_Bodies *two_body = two_bodies_init(body1, body2, 0);
  create_collision(scene, body1, body2, (CollisionHandler) destructive_handler,
                  two_body, (FreeFunc) free);
}

void create_semidestructive_collision(Scene *scene, Body *body1, Body *body2){
    Two_Bodies *two_body = two_bodies_init(body1, body2, 0);
    create_collision(scene, body1, body2, (CollisionHandler) semidestructive_handler,
                    two_body, (FreeFunc) free);
}

void create_physics_collision(Scene *scene, double elasticity, Body *body1,
  Body *body2) {
    Two_Bodies *two_body = two_bodies_init(body1, body2, elasticity);
    create_collision(scene, body1, body2, (CollisionHandler) physics_handler,
                    two_body, (FreeFunc) free);
 }

 void create_inelastic_collision(Scene *scene, Body *body1, Body *body2) {
     create_collision(scene, body1, body2, (CollisionHandler) inelastic_handler,
                     NULL, (FreeFunc) free);
}

void create_normal_force(Scene *scene, Body *body1, Body *body2, double g) {
  Two_Bodies *two_body = two_bodies_init(body1, body2, g);
  create_collision(scene, body1, body2, (CollisionHandler) normal_force_handler,
                  two_body, (FreeFunc) free);
}

void stop_at_ground(Scene *scene, Body *body, Body *ground) {
  Two_Bodies *two_body = two_bodies_init(body, ground, 0);
  create_collision(scene, body, ground, (CollisionHandler) stop_ground_handler,
                  two_body, (FreeFunc) free);
}
