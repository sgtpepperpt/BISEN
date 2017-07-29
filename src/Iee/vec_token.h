#ifndef vec_token_H
#define vec_token_H

#include <stdio.h> //to remove
#include <stdlib.h> // malloc / free
#include <IeeUtils.h>

typedef struct vec_token {
    iee_token* array;
    int max_size;
    int counter;
} vec_token;

// initialisers
void init(vec_token* v, int max_size);
void grow(vec_token* v);
void destroy(vec_token* v);

// modifiers
void push_back(vec_token* v, iee_token e);
void pop_back(vec_token* v);

// elements access
iee_token peek_back(vec_token v);
unsigned size(vec_token v);

#endif
