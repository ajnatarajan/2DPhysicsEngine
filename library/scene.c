/*
 * Recent changes
 *
 * Added associated_bodies to instance_force
 * Wrote instance_force_free
 * Deprecated scene_add_force_creator
 * Wrote scene_add_bodies_force_creator
 * Wrote body/force creator removal in scene_tick
 */

 #include <assert.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include "scene.h"
 #include "list.h"
 #include "vector.h"
 #include "polygon.h"

struct scene {
  List *bodies;
  size_t num_bodies;
  List *instance_forces;
  size_t num_instance_forces;
};

struct instance_force {
  ForceCreator force_creator;
  void *aux; // one_body, two_bodies, n_bodies
  List *bodies; // slightly redundant in terms of storing bodies but exists
                // so scene.c can access individual bodies without accessing the
                // structs in forces.c
  FreeFunc aux_freer;
};

Instance_Force *instance_force_init(ForceCreator force_creator, void *aux, List *bodies, FreeFunc freer) {
  Instance_Force *instance_force = (Instance_Force *) malloc(sizeof(Instance_Force));
  instance_force->force_creator = force_creator;
  instance_force->aux = aux;
  instance_force->bodies = bodies;
  instance_force->aux_freer = freer;
  return instance_force;
}

void instance_force_free_limited(Instance_Force *i) {
  if(i->aux_freer != NULL) {
    i->aux_freer(i->aux);
  }
  free(i->bodies);
  free(i);
}

Scene *scene_init(void) {
  Scene *scene = (Scene *) malloc(sizeof(Scene));
  assert(scene != NULL);
  scene->bodies = list_init(INITIAL_CAPACITY, (FreeFunc) body_free);
  scene->num_bodies = 0;
  scene->instance_forces = list_init(INITIAL_CAPACITY, (FreeFunc) instance_force_free_limited);
  scene->num_instance_forces = 0;
  return scene;
}

/*
void scene_free(Scene *scene) {
  list_free(scene->bodies);
  list_free(scene->instance_forces);
  free(scene);
}*/
void scene_free(Scene *scene) {
    /*
  for(size_t i = 0; i < scene->num_instance_forces; i++) {
    Instance_Force *instance_force = list_get(scene->instance_forces, i);
    instance_force->aux_freer(instance_force->aux);
} */
    list_free(scene->bodies);
    list_free(scene->instance_forces);
    free(scene);
}

size_t scene_bodies(Scene *scene) {
  return scene->num_bodies;
}

Body *scene_get_body(Scene *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(Scene *scene, Body *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

// mark the body for removal
void scene_remove_body(Scene *scene, size_t index) {
    Body *b = (Body *)scene_get_body(scene, index);
    body_remove(b);
}

// this actually frees it
void scene_free_body(Scene *scene, size_t index) {
    Body *b = (Body *)scene_get_body(scene, index);
    list_remove(scene->bodies, index);
    body_free(b);
    scene->num_bodies--;
}

void scene_set_body(Scene *scene, size_t index, Body *b) {
    list_set(scene->bodies, index, b);
    //printf("New body inserted at %zu\n", index);
    //body_free(c);
}

/*
 * This method has been deprecated as of project 5 but remains for compatibility
 * purposes.
 */
void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux,
  FreeFunc freer) {
    List *l = list_init(0, NULL);
    scene_add_bodies_force_creator(scene, forcer, aux, l, freer);
}

void scene_add_bodies_force_creator(
  Scene *scene, ForceCreator forcer, void *aux, List *bodies_list, FreeFunc freer) {
      Instance_Force *to_add = instance_force_init(forcer, aux, bodies_list, freer);
      list_add(scene->instance_forces, to_add);
      scene->num_instance_forces++;
      /*
      for (int i = 0; i < list_size(bodies_list); i++) {
          scene_add_body(scene, list_get(bodies_list, i));
      }
      */
}


void scene_tick(Scene *scene, double dt) {
    // printf("scene_tick is running!\n");
    // Debugging purposes: print the pointers of each body in scene->bodies.
    for (size_t i = 0; i < scene->num_bodies; i++) {
        // printf("The pointer of the body at index %zu is %p\n")
        for (size_t j = 0; j < scene->num_bodies; j++) {
            if (scene_get_body(scene, i) == scene_get_body(scene, j) && i != j) {

                printf("Yikes at indices %zu and %zu in a list with the last index as %zu\n", i, j, scene->num_bodies - 1);
            }
        }
    }

    // Apply forces wherever necessary.
    for (size_t j = 0; j < scene->num_instance_forces; j++) {
        //printf("iterating at index %zu\n", j);
        Instance_Force *i = list_get(scene->instance_forces, j);
        ForceCreator curr_creator = i->force_creator;
        curr_creator(i->aux);
    }

    // Remove force creators associated with flagged bodies.
    for (size_t i = 0; i < scene->num_instance_forces; i++) {
        bool free_instance_force = false;
        Instance_Force *k = list_get(scene->instance_forces, i);
        List *l = k->bodies;
        for (size_t j = 0; j < list_size(l); j++) {
            Body *b = list_get(k->bodies, j);
            if (body_is_removed(b)) {
                //printf("body is removed");
                free_instance_force = true;
                break;
            }
        }
        if (free_instance_force) {
            //printf("freeing a force");
            list_remove(scene->instance_forces, i);
            instance_force_free_limited(k);
            scene->num_instance_forces--;
            i--;
        }
    }

    // Remove bodies flagged for removal.
    for (size_t i = 0; i < scene->num_bodies; i++) {
        // printf("We're looking at the body in index %zu of %zu\n", i, scene->num_bodies);
        Body *b = scene_get_body(scene, i);
        // printf("%p\n", b);
        if (body_is_removed(b)) {
            //printf("We're about to free and remove the body at index %zu\n", i);
            scene_free_body(scene, i);
            i--;
        }
    }

    // Tick bodies that still exist.
    for (size_t i = 0; i < scene->num_bodies; i++) {
        Body *b = scene_get_body(scene, i);
        body_tick(b, dt);
    }

}
