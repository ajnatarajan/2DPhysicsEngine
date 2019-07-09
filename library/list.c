#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/list.h"
#include <stddef.h>

struct list {
  size_t size;
  size_t capacity;
  void **data;
  FreeFunc freer;
};

List *list_init(size_t initial_size, FreeFunc freer) {
  List *list = (List *) malloc(sizeof(List));
  assert(list != NULL);
  list->size = 0;
  list->capacity = initial_size;
  list->data = malloc(sizeof(void *) * list->capacity);
  list->freer = freer;
  return list;
}

void list_free(List *list) {
  if(list->freer != NULL) {
    for (int i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
      // Alternatively: arr_list_get(arr_list, i);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(List *list) {
  return list->size;
}

size_t list_capacity(List *list) {
  return list->capacity;
}

FreeFunc list_freer(List *list) {
  return list->freer;
}

void *list_get(List *list, size_t index) {
  if (index > list->size) {
    printf("Index %zu out of get bounds for array list of size %zu\n", index, list->size);
    exit(0);
  }
  return list->data[index];
}

void *list_remove(List *list, size_t index) {
  if (index >= list->size) {
    printf("Index %zu out of remove bounds for array list of size %zu\n", index, list->size);
    exit(1);
  }
  void *removedElement = list->data[index];
  for (int i = index; i < list->size - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->size--;
  return removedElement;
}

void list_add(List *list, void *value) {
  list_grow(list);
  list->data[list->size++] = value;
}

void list_grow(List *list) {
  if (list->size >= list->capacity) {
    list->capacity *= GROW_FACTOR;

    /* https://www.tutorialspoint.com/c_standard_library/c_function_realloc.htm */
    list->data = realloc(list->data, sizeof(void *) * list->capacity);
  }
}

void list_set(List *list, size_t index, void *value) {
  assert(index < list->size);
  list->data[index] = value;
}
