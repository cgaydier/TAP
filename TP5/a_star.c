#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "variables.h"
#include "utils.h"
#include "heap.h" // il faut aussi votre code pour heap.c


// Une "heuristic" est une fonction h() qui renvoie une distance (type
// double) en fonction d'une position de départ et de fin,
// éventuellement en fonction de la grille (qu'on utilisera pas en
// fait).
typedef double (*heuristic)(position,position,grid);


// Heuristique "nulle" pour Dijkstra
double h0(position s, position t, grid G){
  return 0.0;
}


// Heuristique "vol d'oiseau"
double hvo(position s, position t, grid G){
  return fmax(abs(t.x-s.x),abs(t.y-s.y));
}


// Structure "noeud" pour le tas Q
typedef struct node {
  position pos; // position (.x,.y) d'un noeud u
  double cost;  // coût[u]
  struct node *parent; // parent[u] = pointeur vers le père, NULL pour start
  double f;     // f[u] = coût[u] + h(u,end)
} node;


// Votre fonction A_star(G,h) doit déterminer s'il existe une chemin
// dans G entre les positions G.start à G.end. S'il n'y a pas de
// chemin, affichez un simple message d'erreur. Sinon, vous devez
// remplir le champs .mark de la grille G pour que le chemin soit
// affiché par drawGrid(G). Mettez G.mark[i][j] = M_PATH si (i,j)
// appartient au chemin trouvé (cf. "variables.h").
//
// Le coût de l'arête u->v entre deux cases u et v voisines est
// déterminé par le tableau constant costs[Z] défini dans variables.h
// où Z est la valeur stockée dans la case v de la grille. Attention !
// costs[V_WALL] n'est pas définie car il n'y a pas de sommet en (i,j)
// si G.value[i][j] = V_WALL.
//
// Vous devez utiliser un tas de "node" pour gérer le tas min Q avec
// une fonction minimum qui dépend du champs .f des noeuds. Pour gérer
// l'ensemble P, servez-vous du champs G.mark[i][j] (= M_USED si (i,j)
// est dans P, et = 0 sinon).

int compareNode(const void *x, const void *y) {
    return ((node *)x)->f - ((node*)y)->f;

}


void A_star(grid G, heuristic h){
  heap Q = heap_create(G.X*G.Y,compareNode);
  node *start = malloc(sizeof(node));
  start->pos = G.start;
  start->cost = 0;
  start->parent = NULL;
  heap_add(Q,start);
  
  while( !heap_empty(Q) ) {
      node *current = heap_pop(Q);
      if(G.mark[current->pos.x][current->pos.y] == M_USED) {
          continue;
      }
      G.mark[current->pos.x][current->pos.y] = M_USED;
      
      if(current->pos.x == G.end.x && current->pos.y == G.end.y ) {
          //Construct path
          printf("Chemin\n");
          while(current != NULL) {
              G.mark[current->pos.x][current->pos.y] |= M_PATH;
              current = current->parent;
          }
          return;
      }
      for(int i = -1; i <= 1; i++) {
          for(int j = -1; j <= 1; j++) {
              int nbX = current->pos.x+i;
              int nbY = current->pos.y+j;
                  if(G.mark[nbX][nbY] == M_USED ) {} //Inside P
                  else if( G.value[nbX][nbY] == V_ROOM || G.value[nbX][nbY] == V_SAND) {
                      node *nb = malloc(sizeof(node));
                      nb->pos.x = nbX;
                      nb->pos.y = nbY;
                      nb->cost = current->cost + costs[G.value[nbX][nbY]];
                      nb->f = nb->cost + h(nb->pos,G.end,G);
                      nb->parent = current;
                      heap_add(Q,nb);
                      
                      //drawGrid(G);
                  }
          }
      }
      
  }
  printf("Erreur pas de chemin\n");
  
}


// Taille initiale de la fenêtre
int width = 640;
int height = 480;

bool running = true;
bool mouse_left_down = false;
bool mouse_right_down = false;
double scale = 1.0f;

// Délais d'affichage pour gridDraw()
int delay = 1; // unité = 10 ms

int main(int argc, char *argv[]){
  for(int i = 0; i < V_NUM ;i++) {
        costs[i]=i+1;
  }
  srandom(0xc0c0);
  initSDLOpenGL();
  int w = 401; // doit être impaire

  // labyrithne aléatoire w x w
  // le champs .mark est initialisé à 0
  grid G = initGridBlock(w,w);

  A_star(G,hvo);

  while (running) {
    drawGrid(G);
    handleEvent(true);
  }

  freeGrid(G);
  cleaning();

  return 0;
}
