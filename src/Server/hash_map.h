/*
 * Generic hashmap manipulation functions
 *
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 *
 * Modified by Guilherme Borges to allow void* keys; for now all keys have the same size
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	/* OK */

/*
 * map_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only map_t's.
 */
typedef void* map_t;

/*
 * Return an empty hashmap. Returns NULL if empty.
*/
map_t hashmap_new(size_t key_len);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
int hashmap_put(map_t in, void* key, void* value);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int hashmap_get(map_t in, void* key, void* *arg);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int hashmap_remove(map_t in, void* key);

/*
 * Free the hashmap
 */
void hashmap_free(map_t in);

/*
 * Get the current size of a hashmap
 */
int hashmap_length(map_t in);

#endif
