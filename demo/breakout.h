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

Body *make_rectangle(Vector center, double width, double height, double mass, RGBColor color);

List *list_rectangle(double width, double height);
