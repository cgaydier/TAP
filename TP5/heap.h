#ifndef HEAP_H
#define HEAP_H 


// Structure de tas binaire:
//
//  nmax  = nombre maximum d'objets qui sont des void*
//  size  = nombre d'objets stockés dans le tas
//  array = tableau de stockage des objets à partir de l'indice 1 (au lieu de 0)
//  f     = fonction de comparaison de deux objets (min, max, ..., cf. man qsort)
//
// Attention ! "heap" est défini comme un pointeur pour optimiser les
// appels (empilement d'un mot (= 1 pointeur) au lieu de 4 sinon). 
typedef struct{
  int nmax;
  int size;
  void **array;
  int (*f)(const void *, const void *);
} *heap;


// Crée un tas d'au plus n>0 objets avec une fonction de comparaison
// f() prédéfinie. NB: La taille d'un objet pointé par un pointeur h
// est sizeof(*h).
extern heap heap_create(int n, int (*f)(const void *, const void *));


// Détruit le tas h. On supposera h!=NULL. Attention ! Il s'agit de
// libérer ce qui a été alloué par heap_create(). NB: Les objets
// proprement dit n'ont pas à être libérés.
extern void heap_destroy(heap h);


// Renvoie une valeur non-nulle si le tas h est vide, et 0 sinon. On
// supposera h!=NULL.
extern int heap_empty(heap h);


// Ajoute un objet au tas h. On supposera h!=NULL. Renvoie une valeur
// non-nulle s'il n'y a pas assez de place, et 0 sinon.
extern int heap_add(heap h, void *object);


// Renvoie l'objet en haut du tas h, c'est-à-dire l'élément minimal
// selon f(), sans le supprimer. On supposera h!=NULL. Renvoie NULL si
// le tas est vide.
extern void *heap_top(heap h);


// Comme heap_pop() sauf que l'objet est de plus supprimé. Renvoie
// NULL si le tas est vide.
extern void *heap_pop(heap h);


#endif
