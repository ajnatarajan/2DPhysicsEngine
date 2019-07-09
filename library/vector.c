#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/vector.h"

const Vector VEC_ZERO = {
  .x = 0.0,
  .y = 0.0
};

Vector *vec_init(Vector v) {
  Vector *new_v = malloc(sizeof(Vector));
  *new_v = v;
  return new_v;
}

double vec_get_angle(Vector v) {
  if(v.x == 0 && v.y >= 0) {
    return M_PI / 2;
  } else if (v.x == 0 && v.y <= 0) {
    return 3*M_PI/2;
  }

  if(v.x >= 0 && v.y >= 0) {
    return atan((double) v.y / v.x);
  } else if (v.x <= 0 && v.y >= 0) {
    return atan((double) v.y / v.x) + (M_PI);
  } else if (v.x <= 0 && v.y <= 0) {
    return atan((double) v.y / v.x) + (M_PI);
  } else {
    return atan((double) v.y / v.x);
  }

}

Vector vec_add(Vector v1, Vector v2) {
  Vector v3;
  v3.x = v1.x + v2.x;
  v3.y = v1.y + v2.y;
  return v3;
}

Vector vec_subtract(Vector v1, Vector v2) {
  Vector v3;
  v3.x = v1.x - v2.x;
  v3.y = v1.y - v2.y;
  return v3;
}

Vector vec_negate(Vector v) {
  Vector v1;
  v1.x = -1 * v.x;
  v1.y = -1 * v.y;
  return v1;
}

Vector vec_multiply(double scalar, Vector v) {
  Vector v1;
  v1.x = scalar * v.x;
  v1.y = scalar * v.y;
  return v1;
}

double vec_dot(Vector v1, Vector v2) {
  return (v1.x * v2.x + v1.y * v2.y);
}

double vec_cross(Vector v1, Vector v2) {
  return (v1.x * v2.y - v1.y * v2.x);
}

Vector vec_rotate(Vector v, double angle) {
  Vector v1;
  v1.x = v.x * cos(angle) - v.y * sin(angle);
  v1.y = v.x * sin(angle) + v.y * cos(angle);
  return v1;
}

double vec_magnitude(Vector v) {
    return sqrt(pow(v.x, 2) + pow(v.y, 2));
}

bool vec_equal(Vector v1, Vector v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

Vector vec_unit(Vector v) {
  return vec_multiply(1.0 / vec_magnitude(v), v);
}
