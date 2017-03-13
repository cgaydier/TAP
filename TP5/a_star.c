#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "variables.h"
#include "utils.h"
#include "heap.h" // il faut aussi votre code pour heap.c


// Une fonction de type "heuristic" est une fonction h() qui renvoie
// une distance (type double) entre une position de départ et de fin
// de la grille. La fonction pourrait aussi dépendre de la grille,
// mais on ne l'utilisera pas forcément.
typedef double (*heuristic)(position,position,grid*);


// Heuristique "nulle" pour Dijkstra
double h0(position s, position t, grid *G){
  return 0.0;
}


// Heuristique "vol d'oiseau" pour A*
double hvo(position s, position t, grid *G){
  return fmax(abs(t.x-s.x),abs(t.y-s.y));
}


// Structure "noeud" pour le tas min Q
typedef struct{
  position pos;        // position (.x,.y) d'un noeud u
  double cost;         // coût[u]
  double f;            // f[u] = coût[u] + h(u,end)
  struct node *parent; // parent[u] = pointeur vers le père, NULL pour start
} node;


int compareNode(const void *x, const void *y) {
    return ((node *)x)->f - ((node*)y)->f;
}
// Les arêtes, connectant les cases de la grille, sont valués
// seulement certaines valeurs possibles. Le poids de l'arête u->v,
// noté w(u,v) dans le cours, entre deux cases u et v voisines est
// déterminé par la valeur de la case finale v. Plus précisément, si
// la case v de la grille contient la valeur C, le poids de u->v
// vaudra weight[C] dont les valeurs exactes sont définies ci-après.
// La liste des valeurs possibles d'une case est donné dans
// variables.h: V_FREE, V_WALL, V_WATER, ... Attention !
// weight[V_WALL]<0. Ce n'est pas une valuation correcte car il n'y a
// pas de sommet en position (i,j) si G.value[i][j] = V_WALL.

double weight[]={
    1.0,  // V_FREE
  -99.0,  // V_WALL
    3.0,  // V_SAND
    9.0,  // V_WATER
    2.3,  // V_MUD
    1.5,  // V_GRASS
    0.1,  // V_TUNNEL
};


// Votre fonction A_star(G,h) doit construire un chemin dans la grille
// G entre la position G.start et G.end selon l'heuristique h(). S'il
// n'y en a de chemin, affichez un simple message d'erreur. Sinon,
// vous devez remplir le champs .mark de la grille pour que le chemin
// trouvé soit affiché plus tard par drawGrid(G). La convention est
// qu'en retour G.mark[i][j] = M_PATH ssi (i,j) appartient au chemin
// trouvé (cf. "variables.h").
//
// Pour gérer l'ensemble P, servez-vous du champs G.mark[i][j] (=
// M_USED ssi (i,j) est dans P) qui est initialisé à une valeur
// différente de M_USED par initGrid().
//
// Pour gérer l'ensemble Q, vous devez utiliser un tas min de noeuds
// (type node) avec une fonction de comparaison qui dépend du champs
// .f des noeuds. Pensez que la taille du tas Q est au plus la somme
// des degrés des sommets dans la grille.

void A_star(grid G, heuristic h){
  heap Q = heap_create(G.X*G.Y,compareNode);
  node *start = malloc(sizeof(node));
  start->pos = G.start;
  start->cost = 0;
  start->parent = NULL;
  heap_add(Q,start);
  int totalNodes = 0;
  while( !heap_empty(Q) ) {
      node *current = heap_pop(Q);
      if(G.mark[current->pos.x][current->pos.y] == M_USED) {
          continue;
      }
      G.mark[current->pos.x][current->pos.y] = M_USED;
      if(current->pos.x == G.end.x && current->pos.y == G.end.y ) {
          //Construct path
          printf("Chemin\n");
          int totalCosts = 0;
          while(current != NULL) {
              totalCosts += current->cost;
              G.mark[current->pos.x][current->pos.y] |= M_PATH;
              current = current->parent;
              totalNodes += 1;
          }
          printf("Longeur du chemin : %d\n",totalNodes);
          printf("Cout du chemin : %d\n",totalCosts);
          return;
      }
      for(int i = -1; i <= 1; i++) {
          for(int j = -1; j <= 1; j++) {
              int nbX = current->pos.x+i;
              int nbY = current->pos.y+j;
                  if(G.mark[nbX][nbY] == M_USED ) {} //Inside P
                  else if( G.value[nbX][nbY] != V_WALL ) {
                      node *nb = malloc(sizeof(node));
                      nb->pos.x = nbX;
                      nb->pos.y = nbY;
                      if(abs(i) == abs(j) && (abs(i) == 1 || abs(i) == 0)) {
                        nb->cost = current->cost + weight[G.value[nbX][nbY]];
                      } else {
                        nb->cost = current->cost + weight[G.value[nbX][nbY]];
                      }
                      nb->f = nb->cost + h(nb->pos,G.end,&G);
                      nb->parent = current;
                      G.mark[nbX][nbY] = M_FRONT;
                      heap_add(Q,nb);
                      
                      
                      drawGrid(G);
                  }
          }
      }
      
  }
  printf("Erreur pas de chemin\n");
  
}


// constantes à initialiser avant init_SDL_OpenGL()
int width=640, height=480; // dimensions initiale de la fenêtre
int delay=100; // presser 'a' ou 'z' pour accélèrer ou ralentir l'affichage, unité = 1/100 s

int main(int argc, char *argv[]){

  unsigned t=time(NULL)%1000;
  srandom(t);
  printf("seed: %u\n",t); // pour rejouer la même grille au cas où

  // tester les différentes grilles ...
  
  grid G = initGridFile("mygrid.txt"); // grille à partir d'un fichier
  //grid G = initGridPoints(320,240,V_FREE,1); // grille uniforme
  //grid G = initGridPoints(32,24,V_WALL,0.2); // grille de points aléatoires de type donné
  //grid G = initGridLaby(221,221); // labyrinthe aléatoire

  // Pour ajouter à G des "régions" de différent types:

  //addRandomBlob(G, V_WALL,   (G.X+G.Y)/20);
  //addRandomBlob(G, V_SAND,   (G.X+G.Y)/15);
  //addRandomBlob(G, V_WATER,  (G.X+G.Y)/15);
  //addRandomBlob(G, V_MUD,    (G.X+G.Y)/15);
  //addRandomBlob(G, V_GRASS,  (G.X+G.Y)/15);
  //addRandomBlob(G, V_TUNNEL, (G.X+G.Y)/15);
  
  scale=round(fmin(width/G.X,height/G.Y)); // zoom courrant = nombre de pixels par unité
  init_SDL_OpenGL(); // à mettre avant le 1er "draw"
  
  drawGrid(G); // dessin de la grille avant l'algo

  A_star(G,h0); // h0() ou hvo()

  while(running){
    drawGrid(G);
    handleEvent(true); // presser 'q' pour quiter
  }

  freeGrid(G);
  cleaning_SDL_OpenGL();

  return 0;
}
