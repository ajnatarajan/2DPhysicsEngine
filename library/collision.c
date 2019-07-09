#include "collision.h"
#include "vector.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

Vector project_min_max(List *shape, Vector line) {
  double min = INT_MAX;
  double max = INT_MIN;

  for (size_t i = 0; i < list_size(shape); i++) {
    Vector cur = *(Vector *) list_get(shape, i);
    double projection = vec_dot(cur, line);

    if (projection < min) min = projection;
    if (projection > max) max = projection;
  }

  return (Vector) {min, max};
}

double get_overlap(Vector v1, Vector v2) {
  if (v1.y > v2.y) {
    Vector temp = v1;
    v1 = v2;
    v2 = temp;
  }
  return v1.y - fmax(v2.x, v1.x);
}

CollisionInfo find_collision(List *shape1, List *shape2) {
  double min_overlap = INT_MAX;
  Vector min_overlap_axis;

  for (size_t i = 0; i < list_size(shape1) + list_size(shape2); i++) {
    Vector cur;
    Vector next;

    if (i < list_size(shape1)) {
      cur = *(Vector *) list_get(shape1, i);
      next = *(Vector *) list_get(shape1, (i + 1) % list_size(shape1));
    }
    else {
      cur = *(Vector *) list_get(shape2, i-list_size(shape1));
      next = *(Vector *) list_get(shape2, (i-list_size(shape1) + 1) % list_size(shape2));
    }

    Vector edge = vec_subtract(next, cur);
    Vector perpendicular = vec_unit((Vector) {edge.y, -edge.x});
    Vector proj1 = project_min_max(shape1, perpendicular);
    Vector proj2 = project_min_max(shape2, perpendicular);
    double overlap = get_overlap(proj1, proj2);

    // no overlap
    if (overlap < 0) return (CollisionInfo) {false, VEC_ZERO};
    else if (overlap < min_overlap) {
      min_overlap = overlap;
      min_overlap_axis = perpendicular;
    }
  }
  //printf("ooo a collision\n");
  //printf("%f, %f\n", vec_unit(min_overlap_axis).x, vec_unit(min_overlap_axis).y);
  return (CollisionInfo) {true, min_overlap_axis};

}
