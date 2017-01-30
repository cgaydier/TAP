//
//  TSP - APPROXIMATION / HEURISTIQUES
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "utils.h"
#include "variables.h"
#include "misc.c"

static double dist(point p1, point p2) {
    return sqrt((p2.x - p1.x)*(p2.x - p1.x) +
            (p2.y - p1.y)*(p2.y - p1.y));
} 

static double value(point *V, int n, int *P) {
    double val = 0;
    for (int i = 0; i < n; i++)
        val += dist(V[P[i]], V[P[(i + 1) % n]]);
    return val;
}

static void drawTour(point *V, int n, int *P){

  // saute le dessin si le précédent a été fait il y a moins de 20ms
  static unsigned int last_tick = 0;
  if(n<0) { last_tick = 0; n=-n; }  // force le dessin si n<0
  if(last_tick + 20 > SDL_GetTicks()) return;
  last_tick = SDL_GetTicks();
  handleEvent(false);

  // efface la fenêtre
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // dessine le cycle
  if(P && V){
    selectColor(1,1,1); // couleur blanche
    for (int i = 0 ; i < n ; i++)
      drawLine(V[P[i]], V[P[(i+1) % n]]);
  }
  // dessine les points
  if(V){
    selectColor(1,0,0); // couleur rouge
    for (int i = 0 ; i < n ; i++)
      drawPoint(V[i]);
  }
    
  // affiche le dessin
  SDL_GL_SwapWindow(window);
}

// Génère n points aléatoires du rectangle entier [0,X] × [0,Y] et
// renvoie le tableau des n points ainsi générés
static point* generatePoints(int n, int X, int Y) {
  point *V = malloc(n * sizeof(point));
  const double rx = (double) X / RAND_MAX;
  const double ry = (double) Y / RAND_MAX;
  for (int i = 0 ; i < n ; i++) {
    V[i].x = random() * rx;
    V[i].y = random() * ry;
  }
  return V;
}

// Ajoute p points au tableau V qui en comporte déjà n. Les p nouveaux
// points sont placés aléatoirement sur le cercle de centre c et de
// rayon r et insérés aléatoirement parmi les p points déjà présents
// de V. Le tableau V doit être de longueur au moins n+p.
static void generateCircle(point V[], int n, int p, point c, double r){
  const double K=2.0*M_PI/RAND_MAX;
  double a;
  int i,j;
  for(i=0; i<p; i++){ // p point sur le cercle
    a=K*random();
    V[n+i].x = c.x + r*cos(a);
    V[n+i].y = c.y + r*sin(a);
  }
  n += p; // n -> n+p
  for(i=0; i<n; i++){ // permute V
    j = i+(random()%(n-i));
    SWAP(V[i],V[j],c);
  }
}

static double dist(point A, point B){
  // ...
  return 0;
}

static double value(point *V, int n, int *P){
  // ...
  return 0;
}

/* =============== TSP FLIP ================ */
void reverse(int *T, int p, int q){
  // renverse la partie T[p]...T[q] du tableau avec p<q
  ;;;
  return;
}

double first_flip(point *V, int n, int *P){
  // renvoie le gain>0 du premier flip réalisable et 0 s'il n'y en a
  // pas, tout en réalisant le flip
  ;;;
  return 0.0;
}

static double tsp_flip(point *V, int n, int *P){
  // pensez à faire drawTour() pour visualiser chaque flips
  ;;;
  return 0.0;
}

/* =============== TSP GREEDY ================ */
static double tsp_greedy(point *V, int n, int *P){
  // ...
  return 0.0;
}

// taille initiale de la fenêtre
int width = 640;
int height = 480;

// pour la fenêtre graphique
bool running = true;
bool mouse_down = false;
double scale = 1;


int main(int argc, char *argv[]) {

  initSDLOpenGL();
  srandom(time(NULL));
  //Essayez: srandom(783) ou srandom(46) avec n=40 points
  TopChrono(0); // initialise tous les chronos
	
  bool need_redraw = true;
  bool wait_event = true;

  int n = (argv[1] && atoi(argv[1])) ? atoi(argv[1]) : 400;
  point *V = generatePoints(n, width, height);
  int *P = malloc(n * sizeof(int));

  {
    drawTour(V, n, NULL); // dessine les points
    TopChrono(1); // départ du chrono 1
    double w = tsp_flip(V,n,P);
    char *s = TopChrono(1); // s=durée
    printf("value: %g\n",/2.0w);
    printf("runing time: %s\n",s);
    drawTour(V, -n, P); // force le dessin de la tournée
  }

  sleep(1); // attend 1 seconde

  double r=(width+height)/4.0;
  int p=n/2;
  point c;
  c.x=width/2.0, c.y=height/2.0;
  generateCircle(V,0,p,c,r); // ajoute un grand cercle
  generateCircle(V,p,n-p,c,r/2.0); // ajoute un petit cercle
  
  {
    drawTour(V, n, NULL); // dessine les points
    TopChrono(1); // départ du chrono 1
    double w = tsp_flip(V, n, P);
    char *s = TopChrono(1); // s=durée
    printf("value: %g\n",w);
    printf("runing time: %s\n",s);
    drawTour(V, -n, P); // force le dessin de la tournée
  }
  
  // Affiche le résultat et attend (q pour sortir)
  while(running){
    if(need_redraw) drawTour(V, n, P);
    need_redraw = handleEvent(wait_event);
  }
  
  // Libération de la mémoire
  TopChrono(-1);
  free(V);
  free(P);
  
  cleaning();
  return 0;
}
