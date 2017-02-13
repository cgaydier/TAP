/* test_heap.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heap.h"

static inline int min(int x,int y){ return (x<y)? x:y; }

int fcmp_min(const void *x, const void *y){
  return *(int*)x - *(int*)y;
}

void print_heap(heap h,char format[]){

  printf("--\n");
  if(h==NULL){
    printf("heap = NULL\n\n");
    return;
  }

  int i,k=1;
  int n=0; // n=nombre d'éléments affichés
  int p=0; // p=hauteur

  while(n<h->size){
    k=(1<<p);
    for(i=0;(i<k)&&(n<h->size);i++,n++)
      printf(format,*(int*)h->array[n+1]);
    printf("\n");
    p++;
  }

  for(i=0;i<k;i++) printf("---");
  printf("\n\n");
}

int main (int argc, char* argv[]){
  srandom(time(NULL));
  int n = (argv[1] && atoi(argv[1])) ? atoi(argv[1]) : 15;
  int T[n];
  heap h;
  int i;
  char fmt[]="%02i ";

  printf("\narray unsorted: ");
  for(i=0;i<n;i++){
    T[i]=random()%100;
    printf(fmt,T[i]);
  }
  printf("\n\n");

  h=heap_create(n,fcmp_min);
  for(i=0;i<n;i++){
    printf("insert ");
    printf(fmt,T[i]);
    printf("\n");    
    if(heap_add(h,&(T[i]))) break;
    print_heap(h,fmt);
  }

  printf("array sorted: ");
  for(i=0;i<n;i++){
    printf(fmt,*(int*)heap_pop(h));
  }
  printf("\n\n");

  //heap_destroy(h);
  return 0;  
}
