#include "vec_token.h"

void init(vec_token* v, int max_size) {
    v->counter = 0;
    v->max_size = max_size;

    // allocation
    v->array = (token*) malloc(sizeof(token) * v->max_size);
}

void destroy(vec_token* v) {
    free(v->array);
}

void push_back(vec_token* v, token e) {
    if(v->counter < v->max_size)
        v->array[v->counter++] = e;
}

void pop_back(vec_token* v) {
    if(v->counter > 0)
        v->counter--;
}

token peek_back(vec_token v) {
    return v.array[v.counter - 1];
}

unsigned size(vec_token v) {
    return v.counter;
}

/*
int main (char* argv, int argc) {
    vec_token va;
    init(&va, 4);

    push_back(&va, 8);
    push_back(&va, 19);
    push_back(&va, 3);
    push_back(&va, 4);

    vec_token vb;
    init(&vb, 4);
    
    push_back(&vb, 19);
    push_back(&vb, 1);
    push_back(&vb, 4);
    push_back(&vb, 3);
    
    for(int i = 0; i < r.counter; i++)
        printf("%d ", r.array[i]);
    printf("\n");

    return 0;
}*/
