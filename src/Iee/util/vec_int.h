#ifndef vec_int_H
#define vec_int_H

#include <stdlib.h> // malloc / free
#include <string.h> // memset

typedef struct vec_int {
    int* array;
    int max_size;
    int counter;
} vec_int;

// initialisers
void vi_init(vec_int* v, int max_size);
void vi_destroy(vec_int* v);

// modifiers
void vi_push_back(vec_int* v, int e);
void vi_pop_back(vec_int* v);

// elements access
int vi_peek_back(vec_int v);
unsigned vi_size(vec_int v);
int vi_contains(vec_int v, int e);

// set operations
vec_int vi_vec_intersection(vec_int a, vec_int b, unsigned char* count, unsigned ndocs);
vec_int vi_vec_union(vec_int a, vec_int b, unsigned char* count, unsigned ndocs);

// util
int max(int a, int b);

#endif
