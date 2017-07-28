#ifndef vec_token_H
#define vec_token_H

#include <stdio.h> //to remove
#include <stdlib.h> // malloc / free
#include <Definitions.h> 

typedef struct vec_token {
    token* array;
    int max_size;
    int counter;
} vec_token;

// initialisers
void init(vec_token* v, int max_size);
void destroy(vec_token* v);

// modifiers
void push_back(vec_token* v, token e);
void pop_back(vec_token* v);

// elements access
token peek_back(vec_token v);
unsigned size(vec_token v);

#endif
