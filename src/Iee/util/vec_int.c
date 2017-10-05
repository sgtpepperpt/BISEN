#include "vec_int.h"

void vi_init(vec_int* v, int max_size) {
    v->counter = 0;
    v->max_size = max_size;

    // allocation
    v->array = (int*) malloc(sizeof(int) * v->max_size);
    //for(int i = 0; i < v->max_size; i++)
    //    v->array[i] = 0;
}

void vi_destroy(vec_int* v) {
    free(v->array);
}

void vi_push_back(vec_int* v, int e) {
    if(v->counter < v->max_size)
        v->array[v->counter++] = e;
}

void vi_pop_back(vec_int* v) {
    if(v->counter > 0)
        v->counter--;
}

int vi_peek_back(vec_int v) {
    return v.array[v.counter - 1];
}

unsigned vi_size(vec_int v) {
    return v.counter;
}

int vi_contains(vec_int v, int e) {
    for(int i = 0; i < v.counter; i++) {
        if(v.array[i] == e)
            return 1;
    }

    return 0;
}

vec_int vi_vec_union(vec_int a, vec_int b) {
    vec_int v;
    vi_init(&v, a.counter + b.counter);
        
    // add all elements from a
    for(int i = 0; i < a.counter; i++) {
        vi_push_back(&v, a.array[i]);
    }

    // add all elements from b, if they're not in the union yet
    for(int i = 0; i < b.counter; i++) {
        if(!vi_contains(v, b.array[i]))
            vi_push_back(&v, b.array[i]);
    }

    return v;
}

vec_int vi_vec_intersection(vec_int a, vec_int b) {
    vec_int v;
    vi_init(&v, max(a.counter, b.counter));

    if(a.counter > b.counter) {
        for(int i = 0; i < a.counter; i++) {
            if(vi_contains(b, a.array[i])) {
                vi_push_back(&v, a.array[i]);
            }
        }
    } else {
        for(int i = 0; i < b.counter; i++) {
            if(vi_contains(a, b.array[i])) {
                vi_push_back(&v, b.array[i]);
            }
        }
    }

    return v;
}

int max(int a, int b)
{
    return a < b? b : a;
}

/*
int main (char* argv, int argc) {
    vec_int va;
    init(&va, 4);

    push_back(&va, 8);
    push_back(&va, 19);
    push_back(&va, 3);
    push_back(&va, 4);

    vec_int vb;
    init(&vb, 4);
    
    push_back(&vb, 19);
    push_back(&vb, 1);
    push_back(&vb, 4);
    push_back(&vb, 3);
    
    vec_int r = array_union(va, vb);
    
    for(int i = 0; i < r.counter; i++)
        printf("%d ", r.array[i]);
    printf("\n");

    return 0;
}*/
