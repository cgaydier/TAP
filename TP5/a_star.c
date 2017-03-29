#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
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
  int source;
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
#define CHRONOMAX 10
extern char *TopChrono(const int i){
/*
  Met à jour le chronomètre interne numéro i (i=0..CHRNONMAX-1) et
  renvoie sous forme de char* le temps écoulé depuis le dernier appel
  à la fonction pour le même chronomètre. La précision dépend du temps
  mesuré. Il varie entre la seconde et le 1/1000 de seconde. Plus
  précisément le format est le suivant:

  1d00h00'  si le temps est > 24h (précision: 1')
  1h00'00"  si le temps est > 60' (précision: 1s)
  1'00"0    si le temps est > 1'  (précision: 1/10s)
  1"00      si le temps est > 1"  (précision: 1/100s)
  0"000     si le temps est < 1"  (précision: 1/1000s)

  Pour initialiser et mettre à jour tous les chronomètres (dont le
  nombre vaut CHRONOMAX), il suffit d'appeler une fois la fonction,
  par exemple avec TopChrono(0). Si i<0, alors les pointeurs alloués
  par l'initialisation sont désalloués. La durée maximale est limitée
  à 100 jours. Si une erreur se produit (durée supérieure ou erreur
  avec gettimeofday()), alors on renvoie la chaîne "--error--".
*/
  if(i>=CHRONOMAX) return "--error--";

  /* variables globales, locale à la fonction */
  static int first=1; /* =1 ssi c'est la 1ère fois qu'on exécute la fonction */
  static char *str[CHRONOMAX];
  static struct timeval last[CHRONOMAX],tv;
  int j;

  if(i<0){ /* libère les pointeurs */
    if(!first) /* on a déjà alloué les chronomètres */
      for(j=0;j<CHRONOMAX;j++)
	free(str[j]);
    first=1;
    return NULL;
  }

  /* tv=temps courant */
  int err=gettimeofday(&tv,NULL);

  if(first){ /* première fois, on alloue puis on renvoie TopChrono(i) */
    first=0;
    for(j=0;j<CHRONOMAX;j++){
      str[j]=malloc(10); // assez grand pour "--error--", "99d99h99'" ou "23h59'59""
      last[j]=tv;
    }
  }

  /* t=temps en 1/1000" écoulé depuis le dernier appel à TopChrono(i) */
  long t=(tv.tv_sec-last[i].tv_sec)*1000L + (tv.tv_usec-last[i].tv_usec)/1000L;
  last[i]=tv; /* met à jour le chrono interne i */
  if((t<0L)||(err)) t=LONG_MAX; /* temps erroné */

  /* écrit le résultat dans str[i] */
  for(;;){ /* pour faire un break */
    /* ici t est en millième de seconde */
    if(t<1000L){ /* t<1" */
      sprintf(str[i],"0\"%03li",t);
      break;
    }
    t /= 10L; /* t en centième de seconde */
    if(t<6000L){ /* t<60" */
      sprintf(str[i],"%li\"%02li",t/100L,t%100L);
      break;
    }
    t /= 10L; /* t en dixième de seconde */
    if(t<36000L){ /* t<1h */
      sprintf(str[i],"%li'%02li\"%li",t/360L,(t/10L)%60L,t%10L);
      break;
    }
    t /= 10L; /* t en seconde */
    if(t<86400L){ /* t<24h */
      sprintf(str[i],"%lih%02li'%02li\"",t/3600L,(t/60L)%60L,t%60L);
      break;
    }
    t /= 60L; /* t en minute */
    if(t<144000){ /* t<100 jours */
      sprintf(str[i],"%lid%02lih%02li'",t/1440L,(t/60L)%24L,t%60L);
      break;
    }
    /* error ... */
    sprintf(str[i],"--error--");
  }

  return str[i];
}

void A_star2(grid G, heuristic h){
  heap Q = heap_create(G.X*G.Y,compareNode);
    for(int i = 0; i < G.X; i++)
        for(int j = 0; j < G.Y; j++)
            G.mark[i][j]=M_NULL;
  node *start = malloc(sizeof(node));
  start->pos = G.start;
  start->cost = 0;
  start->parent = NULL;
  start->f=0;
  start->source = 1;
  
  node *end = malloc(sizeof(node));
  end->pos = G.end;
  end->cost = 0;
  end->parent = NULL;
  end->f = 0;
  end->source = 0;
  
  heap_add(Q,start);
  heap_add(Q,end);
  int totalNodes = 0;
  while( !heap_empty(Q) ) {
      node *current = heap_pop(Q);
      if(G.mark[current->pos.x][current->pos.y] == M_USED && current->source == 0) {
          printf("Fini\n");
          node *parent = current;
          //Construct path
          printf("Chemin\n");
          int totalCosts = 0;
          while(current != NULL) {
              totalCosts += current->cost;
              G.mark[current->pos.x][current->pos.y] = M_PATH;
              current = current->parent;
              totalNodes += 1;
          }
          current = heap_pop(Q);
          while(current != NULL) {
              if(((node *)current->parent)->pos.x == parent->pos.x && 
                 ((node *)current->parent)->pos.y == parent->pos.y && current->source == 1) {
                  break;
              }
              current = heap_pop(Q);
          }
          
          while(current != NULL) {
              totalCosts += current->cost;
              G.mark[current->pos.x][current->pos.y] = M_PATH;
              current = current->parent;
              totalNodes += 1;
          }
          printf("Longeur du chemin : %d\n",totalNodes);
          printf("Cout du chemin : %d\n",totalCosts);
          return;
      } else if (G.mark[current->pos.x][current->pos.y] == M_USED2 && current->source == 1) {
          printf("Fini2\n");
          node *parent = current;
          //Construct path
          printf("Chemin\n");
          int totalCosts = 0;
          while(current != NULL) {
              totalCosts += current->cost;
              G.mark[current->pos.x][current->pos.y] = M_PATH;
              current = current->parent;
              totalNodes += 1;
          }
          current = heap_pop(Q);
          while(current != NULL) {
              if(((node *)current->parent)->pos.x == parent->pos.x && 
                 ((node *)current->parent)->pos.y == parent->pos.y && current->source == 0) {
                  break;
              }
              current = heap_pop(Q);
          }
          
          while(current != NULL) {
              totalCosts += current->cost;
              G.mark[current->pos.x][current->pos.y] = M_PATH;
              current = current->parent;
              totalNodes += 1;
          }
          printf("Longeur du chemin : %d\n",totalNodes);
          printf("Cout du chemin : %d\n",totalCosts);
          return;
      }
      if(G.mark[current->pos.x][current->pos.y] == M_USED || G.mark[current->pos.x][current->pos.y] == M_USED2) {
          continue;
      }
      if(current->source == 1)
        G.mark[current->pos.x][current->pos.y] = M_USED;
      else
        G.mark[current->pos.x][current->pos.y] = M_USED2;
      
      
      for(int i = -1; i <= 1; i++) {
          for(int j = -1; j <= 1; j++) {
              int nbX = current->pos.x+i;
              int nbY = current->pos.y+j;
                  if(G.mark[nbX][nbY] == M_USED && current->source == 1  ) {
                      continue;
                  } else if (G.mark[nbX][nbY] == M_USED2 && current->source == 0 ) {
                      continue;
                  } else if( G.value[nbX][nbY] != V_WALL ) {
                      node *nb = malloc(sizeof(node));
                      nb->pos.x = nbX;
                      nb->pos.y = nbY;
                      if(abs(i) == abs(j) && (abs(i) == 1 || abs(i) == 0)) {
                        nb->cost = current->cost + weight[G.value[nbX][nbY]];
                      } else {
                        nb->cost = current->cost + weight[G.value[nbX][nbY]];
                      }
                      if(current->source == 1) {
                        nb->f = nb->cost + h(nb->pos,G.end,&G);
                        nb->source = 1;
                      }
                      else {
                        nb->f = nb->cost + h(nb->pos,G.start,&G);
                        nb->source = 0;
                      }
                      
                      nb->parent = current;
                      //G.mark[nbX][nbY] = M_FRONT;
                      heap_add(Q,nb);
                      
                      drawGrid(G);
                  }
          }
      }
      
  }
  printf("Erreur pas de chemin\n");
  
}

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
  TopChrono(1); // départ du chrono 1
  A_star2(G,hvo); // h0() ou hvo()
  char *s = TopChrono(1); // s=durée
  
  G = initGridFile("mygrid.txt"); // grille à partir d'un fichier
  TopChrono(2); // départ du chrono 2
  A_star(G,hvo); // h0() ou hvo()
  char *s2 = TopChrono(2); // s=durée
  
  printf("A_star2 = %s\nA_star = %s\n",s,s2);
  while(running){
    drawGrid(G);
    handleEvent(true); // presser 'q' pour quiter
  }

  freeGrid(G);
  cleaning_SDL_OpenGL();

  return 0;
}
