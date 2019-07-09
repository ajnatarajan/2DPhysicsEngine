#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "vector.h"
#include "polygon.h"

double polygon_area(List *polygon) {
  double area = 0;
  size_t size = list_size(polygon);
  /* Shoelace formula */
  for(size_t i = 0; i < size; i++) {
    double nextY = (*(Vector *)(list_get(polygon, (size + i + 1) % size))).y;
    double prevY = (*(Vector *)(list_get(polygon, (size + i - 1) % size))).y;
    double currX = (*(Vector *)(list_get(polygon, i))).x;
    area += (currX * (nextY - prevY));
  }

  area *= 0.5;
  return area;
}

Vector polygon_centroid(List *polygon) {
  /* Computes the center of mass of the polygon, as per the formula from the
     Wikipedia link given in the corresponding header file. */
  double area = polygon_area(polygon);
  size_t size = list_size(polygon);
  Vector toReturn = {
    .x = 0,
    .y = 0
  };
  for(size_t i = 0; i < size; i++) {
    Vector currPoint = *(Vector *)(list_get(polygon, i));
    Vector nextPoint = *(Vector *) (list_get(polygon, (size + i + 1) % size));
    double currX = currPoint.x;
    double currY = currPoint.y;
    double nextX = nextPoint.x;
    double nextY = nextPoint.y;

    toReturn.x += ((currX + nextX) * (currX * nextY - nextX * currY));
    toReturn.y += ((currY + nextY) * (currX * nextY - nextX * currY));
  }
  toReturn.x /= (6 * area);
  toReturn.y /= (6 * area);

  return toReturn;
}

void polygon_translate(List *polygon, Vector translation) {
  /* For each vector in the given vector list, this applies the given
     transformation. */
  size_t size = list_size(polygon);
  for(size_t i = 0; i < size; i++) {
    Vector currVertex = *(Vector *)(list_get(polygon, i));
    currVertex.x += translation.x;
    currVertex.y += translation.y;
    Vector *toSet = (Vector *) malloc(sizeof(Vector));
    *toSet = currVertex;
    list_set(polygon, i, toSet);
  }
}

void polygon_rotate(List *polygon, double angle, Vector point) {

  /* Translates the polygon to the (0, 0) position. */
  Vector translateToZero = vec_multiply(-1.0, point);
  polygon_translate(polygon, translateToZero);
  size_t size = list_size(polygon);
  /* Rotates each vector in a given vector list by the given angle about the
     given point. */
  for(size_t i = 0; i < size; i++) {
    Vector rotatedVec = vec_rotate(*(Vector *)(list_get(polygon, i)), angle);
    Vector *toSet = (Vector *) malloc(sizeof(Vector));
    *toSet = rotatedVec;
    list_set(polygon, i, toSet);
  }
  /* Translate polygon back to its original centroid. */
  polygon_translate(polygon, point);
}
