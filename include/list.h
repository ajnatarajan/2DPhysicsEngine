#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#define INITIAL_CAPACITY 10
#define GROW_FACTOR 2.0


/**
 * A function that can be called on list elements to release their resources.
 * Examples: free, body_free
 */
typedef void (*FreeFunc)(void *data);

/**
 * A growable array of pointers.
 * Can store values of any pointer type (e.g. Vector*, Body*).
 * The list automatically grows its internal array when more capacity is needed.
 */
 typedef struct list List;

/**
 * Allocates memory for a new list with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param initial_size the number of elements to allocate space for
 * @param freer if non-NULL, a function to call on elements in the list
 *   in list_free() and list_set() when they are no longer in use
 * @return a pointer to the newly allocated list
 */
List *list_init(size_t initial_size, FreeFunc freer);

/**
 * Releases the memory allocated for a list.
 *
 * @param list a pointer to a list returned from list_init()
 */
void list_free(List *list);

/**
 * Gets the size of a list (the number of occupied elements).
 * Note that this is NOT the list's capacity.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the number of elements in the list
 */
size_t list_size(List *list);

/**
 * Gets the capacity of the list.
 *
 * @param list a pointer to a list returned from list_init()
 * @return capacity of the list
 */
size_t list_capacity(List *list);

/**
 * Gets the freer of the list.
 *
 * @param list a pointer to a list returned from list_init()
 * @return freer of the list
 */
FreeFunc list_freer(List *list);
/**
 * Gets the element at a given index in a list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from list_init()
 * @param index an index in the list (the first element is at 0)
 * @return the element at the given index, as a void*
 */
void *list_get(List *list, size_t index);

/**
 * Removes the element at a given index in a list and returns it,
 * moving all subsequent elements towards the start of the list.
 * Asserts that the index is valid, given the list's current size.
 *
 * @param list a pointer to a list returned from list_init()
 * @return the element at the given index in the list
 */
void *list_remove(List *list, size_t index);

/**
 * Appends an element to the end of a list.
 * If the list is filled to capacity, resizes the list to fit more elements
 * and asserts that the resize succeeded.
 * Also asserts that the value being added is non-NULL.
 *
 * @param list a pointer to a list returned from list_init()
 * @param value the element to add to the end of the list
 */
void list_add(List *list, void *value);

/**
 * Doubles the size of the list
 * @param list a pointer to a list returned from list_init()
*/

void list_grow(List *list);

/**
 * Sets value at index of list to value
 * @param list a pointer to a list returned from list_init()
 * @param value the element to add at index
*/
void list_set(List *list, size_t index, void *value);


#endif // #ifndef __LIST_H__
