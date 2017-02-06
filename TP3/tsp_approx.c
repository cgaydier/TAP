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

#include "variables.h"
#include "utils.h"
#include "misc.c" 

static void drawTour(point *V, int n, int *P){

  // saute le dessin si le précédent a été fait il y a moins de 20ms
  static unsigned int last_tick = 0;
  if(n<0) { last_tick = 0; n=-n; }  // force le dessin si n<0
  if(last_tick + 20 > SDL_GetTicks()) return;
  last_tick = SDL_GetTicks();

  // gestion de la file d'event
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

static void drawPath(point *V, int n, int *path, int len){

  // saute le dessin si le précédent a été fait il y a moins de 20ms
  static unsigned int last_tick = 0;
  if(n<0) { last_tick = 0; n=-n; }  // force le dessin si n<0
  if(last_tick + 20 > SDL_GetTicks()) return;
  last_tick = SDL_GetTicks();

  // gestion de la file d'event
  handleEvent(false);

  // efface la fenêtre
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // dessine le chemin
  selectColor(0,1,0); // vert
  for (int i = 0 ; i < len-1 ; i++)
    drawLine(V[path[i]], V[path[i+1]]);

  // dessine les points
  selectColor(1,0,0); // rouge
  for (int i = 0 ; i < n ; i++)
    drawPoint(V[i]);

  // affiche le dessin
  SDL_GL_SwapWindow(window);
}


// Génère n points aléatoires du rectangle entier [0,X] × [0,Y] et
// renvoie le tableau des n points ainsi générés et met à jour la
// variable globale vertices[].
static point* generatePoints(int n, int X, int Y) {
  vertices = malloc(n * sizeof(point));
  const double rx = (double) X / RAND_MAX;
  const double ry = (double) Y / RAND_MAX;
  for (int i = 0 ; i < n ; i++){
    vertices[i].x = random() * rx;
    vertices[i].y = random() * ry;
  }
  return vertices;
}

// Ajoute p points au tableau V qui en comporte déjà n. Les p nouveaux
// points sont placés aléatoirement sur le cercle de centre c et de
// rayon r et insérés aléatoirement parmi les p points déjà présents
// de V. Le tableau V doit être de longueur au moins n+p.
static void generateCircle(point V[], int n, int p, point c, double r){
  const double K=2.0*M_PI/RAND_MAX;
  double a;
  int i;
  for(i=0; i<p; i++){ // p point sur le cercle
    a=K*random();
    V[n+i].x = c.x + r*cos(a);
    V[n+i].y = c.y + r*sin(a);
  }
  Permute(V,n+p); // permute aléatoirement tous les points de V
}
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

/* =============== TSP FLIP ================ */
void reverse(int *T, int p, int q) {
    int tmp;
    for (int i = p, j = q; i < j; i++, j--) {
        tmp = T[i];
        T[i] = T[j];
        T[j] = tmp;
    }
    return;
}

double first_flip(point *V, int n, int *P) {
    for (int i = 0; i < n - 2; i++) {
        for (int j = i + 2; j < n; j++) {
            double diff = (dist(V[P[i]], V[P[(i + 1) % n]]) + dist(V[P[j]], V[P[(j + 1) % n]])) - (dist(V[P[i]], V[P[j]]) + dist(V[P[(i + 1) % n]], V[P[(j + 1) % n]]));
            if (diff > 0) {
                reverse(P, (i + 1) % n, j);
                return diff;
            }
        }
    }
    return 0.0;
}

static double tsp_flip(point *V, int n, int *P) {
    while (first_flip(V, n, P)) {
        drawTour(V, n, P);
    }
    return value(V, n, P);
}

static int nearest(point *V, int n, int current, char *available) {
    int nearest = -1;
    double min = 999999999999;
    for (int i = 0; i < n; i++) {
        if (!available[i] || i == current)
            continue;
        int _dist = dist(V[current], V[i]);
        if (_dist < min) {
            nearest = i;
            min = _dist;
        }
    }
    return nearest;
}

/* =============== TSP GREEDY ================ */
static double tsp_greedy(point *V, int n, int *P) {
    char tmp[n];
    P[0] = 0;
    for (int i = 0; i < n; i++)
        tmp[i] = 1;
    tmp[0] = 0;
    for (int i = 1; i < n; i++) {
        P[i] = nearest(V, n, P[i - 1], tmp);
        tmp[P[i]] = 0;
    }
    return value(V, n, P);
    return 0.0;
}

/* ==== Approximation MST ====*/

// à insérer avant main() dans tsp_approx.c
// compléter tsp_mst() en fin de source

// vous pourrez changer la boucle principale à la fin du main() par
// quelque chose comme ceci:
//
// graph G = createGraph(n);
// ;;;
// while(running){
//  wait_event = true;
//  if (has_changed) tsp_mst(V,n,P,G);
//  if (flipFirst(V, n, P)) wait_event = false;
//  drawGraph(V,n,P,G);
//  has_changed = handleEvent(wait_event);
// }
// freeGraph(G);

typedef struct { // un graphe
    int n; // n=nombre de sommets
    int *deg; // deg[u]=nombre de voisins du sommets u
    int **list; // list[u][i]=i-ème voisin de u
} graph;

typedef struct { // une arête u-v avec un poids
    int u, v; // extrémités de l'arête u-v
    double weight; // poids de l'arête u-v
} edge;

// dessine le graphe G, les points V et la trounée définie par Q

static void drawGraph(point *V, int n, int *Q, graph G) {

    // Saute le dessin si le précédent a été fait il y a moins de 20ms
    static unsigned int last_tick = 0;
    if (last_tick + 20 > SDL_GetTicks()) return;
    last_tick = SDL_GetTicks();

    // Gestion de la file d'event
    handleEvent(false);

    // Efface la fenêtre
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dessin
    glLineWidth(3.0);
    selectColor(0, 0.4, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < G.deg[i]; j++)
            if (i < G.list[i][j])
                drawLine(V[i], V[G.list[i][j]]);
    glLineWidth(1.0);
    selectColor(1, 1, 1);
    for (int i = 0; i < n; i++)
        drawLine(V[Q[i]], V[Q[(i + 1) % n]]);

    // Rouge
    selectColor(1, 0, 0);
    for (int i = 0; i < n; i++) drawPoint(V[i]);

    // Affiche le dessin
    SDL_GL_SwapWindow(window);
}




// Crée un graphe à n sommets avec des listes de tailles n.
// Attention !  G.deg[u] n'est pas initialisé

graph createGraph(int n) {
    graph G;
    G.n = n;
    G.deg = malloc(n * sizeof (int));
    G.list = malloc(n * sizeof (int*));
    for (int u = 0; u < n; u++)
        G.list[u] = malloc(n * sizeof (int)); // taille n par défaut
    return G;
}

// Libère les listes de G

void freeGraph(graph G) {
    for (int u = 0; u < G.n; u++) free(G.list[u]);
    free(G.list);
    free(G.deg);
}

// Ajoute l'arête u-v au graphe G de manière symétrique

void addEdge(graph G, int u, int v) {
    G.list[u][G.deg[u]++] = v;
    G.list[v][G.deg[v]++] = u;
}

// comparaison du poids de deux arêtes pour qsort()

int compEdge(const void *p1, const void *p2) {
    const edge *e1 = p1;
    const edge *e2 = p2;
    return (int) (e1->weight - e2->weight);
}


// Fusionne deux racines x et y du moins haut vers le plus haut

void Union(int x, int y, int *parent, int *height) {
    if (height[x] > height[y]) parent[y] = x;
    else {
        parent[x] = y;
        if (height[x] == height[y]) height[y]++;
    }
}

void UnionSlow(int x, int y, int *parent){
    parent[x] = y;
}

// Renvoie la racine de l'arbre de x avec compression de chemin

int Find(int x, int *parent) {
    if (x != parent[x]) parent[x] = Find(parent[x], parent);
    return parent[x];
}

int FindSlow(int x, int *parent){
    while( x != parent[x])
        x = parent[x];
    return x;
}
// Calcule dans le tableau Q ordre de première visite des sommets de G
// depuis u selon un parcours de profondeur d'abord. Le paramètre s
// est utilisé pour la récursivité: p est le sommet parent de u.

void dfs(graph G, int u, int *Q, int p) {
    static int t; // t=temps=variable globale
    if (p < 0) t = 0;
    Q[t++] = u;
    for (int i = 0; i < G.deg[u]; i++)
        if (G.list[u][i] != p) dfs(G, G.list[u][i], Q, u);
}


// Fonction à compléter
// G doit être déclaré et libéré par l'appelant

double tsp_mst(point *V, int n, int *Q, graph G) {
    // Algo. Kruskal:
    // 0. vider le graphe G
    // 1. remplir et trier le tableau d'arêtes
    // 2. répéter pour chaque arête u-v:
    //    si u-v ne forme pas un cycle dans G (<=> u,v dans des composantes différentes)
    //    alors ajouter u-v à G
    // 3. calculer dans Q le DFS de G

    for (int i = 0; i < n; i++) {
        G.deg[i] = 0;
        for (int j = 0; j < n; j++)
            G.list[i][j] = 0;
    }
    edge *edges = malloc(n * (n - 1) / 2 * sizeof (edge)); // tableau d'arêtes
    int k = 0;
    for (int i = 0; i < n; i++)
        for (int j = i+1; j < n; j++) {
            edges[k].u = i;
            edges[k].v = j;
            edges[k].weight = dist(V[i], V[j]);
            k++;
        }
    qsort(edges, n * (n - 1) / 2, sizeof (edge), compEdge);
    // pour Union-and-Find
    int *parent = malloc(n * sizeof (int)); // parent[x]=parent de x (=x si racine)
    int *height = malloc(n * sizeof (int)); // height[x]=hauteur de l'arbre de racine x
    for (int x = 0; x < n; x++) {
        parent[x] = x; // chacun est racine de son arbre
        height[x] = 0; // hauteur nulle
    }
    for (int i = 0; i < n * (n - 1) / 2; i++) {
        int x = Find(edges[i].u, parent);
        int y = Find(edges[i].v, parent);
        if (x != y) {
            Union(x, y, parent, height);
            addEdge(G, edges[i].u, edges[i].v);
        }
    }

    free(parent);
    free(height);
    free(edges);

    // calcule le dfs dans Q et renvoie la valeur de la tournée
    dfs(G, 0, Q, -1);
    return value(V, n, Q);
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
    int *P = malloc(n * sizeof (int));

    for (int i = 0; i < n; i++) {
        P[i] = i;
    }
    graph G = createGraph(n);
    while (running) {
        wait_event = true;
        if (need_redraw) tsp_mst(V, n, P, G);
        tsp_flip(V,n,P);
        drawGraph(V, n, P, G);
        need_redraw = handleEvent(wait_event);
    }
    freeGraph(G);

    // Libération de la mémoire
    TopChrono(-1);
    free(V);
    free(P);

    cleaning();
    return 0;
}
