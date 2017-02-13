#include <stdlib.h>
#include <stdio.h>
#include "heap.h"

heap heap_create(int n, int (*f)(const void *, const void *)){
    heap h = malloc(sizeof(heap));
    h->array = malloc(sizeof(void *) * n);
    h->f = f;
    h->nmax = n;
    h->size = 0;
    return h;
}

void heap_destroy(heap h){
    free(h->array);
}


int heap_empty(heap h){
  return h->size == 0;
}

int heap_add(heap h, void *object){
    if(h->size+1 > h->nmax)
        return 1;
    h->array[h->size+1] = object;
    int current = h->size + 1;
    while (current != 1 && h->f(h->array[current/2],h->array[current]) > 0) {
        void *tmp = h->array[current/2] ;
        h->array[current/2] = h->array[current];
        h->array[current] = tmp;
        current = current/2;
    }
    h->size++;
    return 0;
}


void *heap_top(heap h){
    if(h->size == 0) {
        return NULL;
    }
  return h->array[1];
}


void *heap_pop(heap h){
    if(heap_empty(h)) {
        return NULL;
    }
    void *root = h->array[1];
    h->array[1] = h->array[h->size];
    int i = 1;
    int min = 1;
    do {
        if(2*i <= h->size && h->f( h->array[2*i], h->array[min]) < 0 ) {
            min = 2*i;
        }
        if(2*i+1 <= h->size && h->f( h->array[2*i+1], h->array[min]) < 0 ){
            min = 2*i+1;
        }
        if(i != min) {
            void *tmp = h->array[min] ;
            h->array[min] = h->array[i];
            h->array[i] = tmp;
            i = min;      
        } else {
            break;
        }
    } while(1);

    h->size--;
    
    return root;
}
