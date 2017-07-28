#ifndef vec_int_H
#define vec_int_H

#include <stdio.h> //to remove
#include <stdlib.h> // malloc / free

typedef struct vec_int {
    int* array;
    int max_size;
    int counter;
} vec_int;

// initialisers
void init(vec_int* v, int max_size);
void destroy(vec_int* v);

// modifiers
void push_back(vec_int* v, int e);
void pop_back(vec_int* v);

// elements access
int peek_back(vec_int v);
unsigned size(vec_int v);
int contains(vec_int v, int e);

// set operations
vec_int vec_intersection(vec_int a, vec_int b);
vec_int vec_union(vec_int a, vec_int b);

// util
int max(int a, int b);

#endif
