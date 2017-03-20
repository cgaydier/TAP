#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "variables.h"

#include "misc.c"

#define IN_SET(S,i)  ((S) & (1 << (i))) // est-ce que i est dans S ?
#define ADD_SET(S,i) (S | (1 << (i)))   // ajoute i à S
#define DEL_SET(S,i) (S & ~(1 << (i)))  // supprimer i de S

const float WHITE[] = {1.0f, 1.0f, 1.0f};
const float RED[] = {1.0f, 0.0f, 0.0f};
const float GREEN[] = {0.0f, 1.0f, 0.0f};
const float BLUE[] = {0.0f, 0.0f, 1.0f};
int lenPath;
// Génère des points aléatoires dans le rectangle [0,max_X] × [0,max_Y]

static point* generatePoints(int n, int max_X, int max_Y) {
    point *V = malloc(n * sizeof (point));
    double ratio_x = (double) max_X / RAND_MAX;
    double ratio_y = (double) max_Y / RAND_MAX;
    for (int i = 0; i < n; i++) {
        V[i].x = rand() * ratio_x;
        V[i].y = rand() * ratio_y;
    }
    return V;
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

static double value_opt(point *V, int n, int *P, int max) {
    double val = 0;
    for (int i = 0; i < n; i++) {
        val += dist(V[P[i]], V[P[(i + 1) % n]]);
        if (val > max) {
            return -(i + 1);
        }
    }
    return val;
}

static void print(int *P, int n) {
    for (int i = 0; i < n; i++) {
        printf("P[%d] = %d ", i, P[i]);
    }
    printf("\n");
}

struct paire {
    int val;
    int use;
};

static void MaxPermutation(int *P, int n, int k) {
    struct paire Tab[n];
    for (int i = 0; i < n; i++) {
        Tab[i].val = i;
        Tab[i].use = 0;
    }
    for (int i = 0; i < k; i++)
        Tab[P[i]].use = 1;
    for (int i = k; i < n; i++) {
        int j = n - 1;
        while (Tab[j].use == 1 && j > 0) {
            j--;
        }
        if (Tab[j].use == 0) {
            P[i] = Tab[j].val;
            Tab[j].use = 1;
        }
    }

}

static double tsp_brute_force(point *V, int n, int *P) {
    int tmp[n];
    P[0] = 0;
    memcpy(tmp, P, sizeof (int)*n);
    double max = value(V, n, P);

    while (NextPermutation(tmp, n) && tmp[0] == 0) {
        double res = value_opt(V, n, tmp, max);
        if (res > 0) {
            if (res < max) {
                memcpy(P, tmp, sizeof (int)*n);
                max = res;
            }
        } else {
            int k = -res;
            MaxPermutation(tmp, n, k);
        }
    }
    return max;
}

static int nearest(point *V, int n, int current, char *available) {
        int nearest = -1;
        double min = 999999999999;
        for(int i = 0; i < n; i++) {
                if(!available[i] || i==current)
                        continue;
                int _dist = dist(V[current],V[i]);
                if( _dist < min ) {
                        nearest = i;
                        min = _dist;
                }
        }
        return nearest;
}
static double tsp_plus_proche(point *V, int n, int *P) {
        char tmp[n];
        P[0] = 0;
        for(int i = 0; i < n;i++)
                tmp[i] = 1;
        tmp[0] = 0;
        for(int i = 1; i < n;i++) {
                P[i] = nearest(V,n,P[i-1],tmp);
                tmp[P[i]] = 0;
        }
        return value(V, n, P);		
}
 

// Taille initiale de la fenêtre
int width = 640;
int height = 480;

bool running = true;
bool mouse_down = false;
double scale = 1.0f;

static void draw(point *V, int n, int *P, int *P2, int *P3) {
    // Efface la fenêtre
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    point offset;
    point offset2;
    offset.x = 640;
    offset.y = 0;
    offset2.x = 0;
    offset2.y = 480;
    // Dessin…
    // Choisir la couleur blanche
    selectColor(WHITE[0], WHITE[1], WHITE[2]);
    for (int i = 0; i < n && i < lenPath; i++) {
        drawLine(V[P[i]], V[P[(i + 1) % n]]);
    }
    //glTranslatef(640.0f,0.0f,0.0f);
    selectColor(BLUE[0], BLUE[1], BLUE[2]);
    for (int i = 0; i < n && i < lenPath; i++) {
        drawLineOff(V[P2[i]], V[P2[(i + 1) % n]],offset);
    }
    //glTranslatef(0.0f,480.0f,0.0f);
    selectColor(RED[0], RED[1], RED[2]);
    for (int i = 0; i < n && i < lenPath; i++) {
        drawLineOff(V[P3[i]], V[P3[(i + 1) % n]],offset2);
    }
    // Rouge
    selectColor(1.0f, 0.0, 0.0f);
    for (int i = 0; i < n ; i++) {
        drawPoint(V[i]);
        drawPointOff(V[i],offset);
        drawPointOff(V[i],offset2);
    }

    handleEvent(false);

    // Affiche le dessin
    SDL_GL_SwapWindow(window);
}
static void drawPath(point *V, int n, int *path, int len);

/* ==== Programmation dynamique ====*/

static int NextSet(unsigned S, int n) {
    /*
      Renvoie le plus petit entier immédiatement supérieure à S>0 et qui a
      soit le même poids que S (c'est-à-dire le même nombre de 1 dans son
      écriture binaire que S), soit le poids de S plus 1 s'il n'y en a pas
      de même poids. La fonction renvoie 0 si S est le dernier entier sur
      n bits, soit S=2^n-1. Pensez à inclure <string.h>.
     */
    int p1 = ffs(S);
    int p2 = ffs(~(S >> p1)) + p1 - 1;
    if (p2 - p1 + 1 == n) return 0; // cas S=2^n
    if (p2 == n) return (1 << (p2 - p1 + 2)) - 1; // cas: poids(S)+1
    int mask = (-1) << p2;
    return (S & mask) | 1 << p2 | ((1 << (p2 - p1)) - 1); // cas: poids(S)
}

/* une cellule de la table */
typedef struct {
    int v;
    double d;
} cell;

static double tsp_prog_dyn(point *V, int n, int *Q) {
    /*
      Version programmation dynamique du TSP. Le résultat doit être écrit
      dans la permutation Q. On renvoie la valeur de la tournée minimum ou
      -1 s'il y a eut une erreur.

      Attention ! Par rapport au cours, il est plus malin de commencer la
      tournée à partir du sommet d'indice n-1 (au lieu de 0). Pourquoi ?

      Donc D[t][S] = coût minimum d'un chemin allant de V[n-1] à V[t] qui
      visite tous les sommets d'indice dans S.

      Il sera intéressant de dessiner à chaque fois que possible le chemin
      courant avec drawPath()
     */


    cell **D;
    int S = 0;
    D = malloc(sizeof (cell*) * n);
    for (int j = 0; j < n; j++) {
        D[j] = malloc(sizeof (cell) * (1 << n));
    }
    for (int i = n - 1; i >= 0; i--) {
        D[i][ADD_SET(0, i)].d = dist(V[n - 1], V[i]);
        D[i][ADD_SET(0, i)].v = n - 1;        
    }

    do {
        for (int C = 0; C < n - 1; C++) {
            if (IN_SET(S, C)) {
                int SwithoutC = DEL_SET(S, C);
                if(S != ADD_SET(0, C)) {
                    D[C][S].d = -1.0f;
                }
                for (int j = 0; j < n - 1; j++) {
                    if (IN_SET(SwithoutC, j)) {
                        double lengthWithC = D[j][SwithoutC].d + dist(V[C], V[j]);
                        if (D[C][S].d < 0 || lengthWithC < D[C][S].d) {
                            D[C][S].d = lengthWithC;
                            D[C][S].v = j;
                        }
                    }
                }
            }
        }

    } while ((S = NextSet(S, n - 1)) && running);
    // ne pas reconstruire la permutation si le calcul a été interrompu
    if (!running) {
        //free ...
        return -1;
    }

    // tournée_min
    int Sfull = 0;
    for (int i = 0; i < n - 1; i++) {
        Sfull = ADD_SET(Sfull, i);
    }
    double min = D[0][Sfull].d + dist(V[0], V[n - 1]);
    int clast = 0;
    for (int i = 1; i < n - 1; i++) {
        double minCalc = D[i][Sfull].d + dist(V[i], V[n - 1]);
        if (minCalc < min) {
            clast = i;
            min = minCalc;
        }
    }
    S = Sfull;
    Q[0] = n - 1;
    for (int i = 1; i < n; i++) {
        int tmp = clast;
        clast = D[clast][Sfull].v;
        Sfull = DEL_SET(Sfull, tmp);
        Q[i] = tmp;
    }
    for (int i = 0; i < n; i++)
        free(D[i]);
    free(D);

    return value(V, n, Q);
}

static double tsp_twist(point *V,int n, int *P) {
    int min = value(V,n,P);
    int minP[n];
    for(int it = 0; it < n;it++)
        minP[it] = P[it];
    for(int i = 0; i < n; i++)
        for(int j = n-1; j > i;j--) {
            int tmp = P[i];
            P[i] = P[j];
            P[j] = tmp;
            if(value(V,n,P) < min){
                min = value(V,n,P);
                for(int it = 0; it < n;it++)
                    minP[it] = P[it];
            }
            tmp = P[i];
            P[i] = P[j];
            P[j] = tmp;
        }
    for(int it = 0; it < n;it++)
        P[it] = minP[it];
    return value(V,n,minP);
}

static void drawPath(point *V, int n, int *path, int len) {
    // Saute le dessin si le précédent a été fait il y a moins de 20ms
    static unsigned int last_tick = 0;
    //if (last_tick + 20 > SDL_GetTicks())
    //    return;
    last_tick = SDL_GetTicks();

    // Gestion de la file d'event
    handleEvent(false);

    // Efface la fenêtre
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dessin …
    // Choisir la couleur blanche
    selectColor(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < len - 1; i++)
        drawLine(V[path[i]], V[path[i + 1]]);
    // Rouge
    selectColor(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < n; i++)
        drawPoint(V[i]);

    // Affiche le dessin
    SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[]) {

    initSDLOpenGL();
    srand(0xc0ca);
    bool need_redraw = true;
    bool wait_event = true;
    
    int n = 12;
    lenPath = n;
    int X = 300, Y = 200;
    point *V = generatePoints(n, X, Y);
    int *P = malloc(n * sizeof (int));
    double w,w2,w3;
    char *s,*s2,*s3;
    int *P2 = malloc(n * sizeof (int));
    int *P3 = malloc(n * sizeof (int));
    for (int i = 0; i < n; i++) P[i] = i;
    
    printf("Bruteforce : White path\n");
    TopChrono(0); // initialise tous les chronos
    TopChrono(1); // départ du chrono 1

    w = tsp_brute_force(V, n, P);
    s = TopChrono(1); // s=durée

    printf("value: %g\n", w);
    printf("runing time: %s\n", s);



    printf("Dynamic : Blue path\n");
    for (int i = 0; i < n; i++) P2[i] = i;
    TopChrono(0); // initialise tous les chronos
    TopChrono(1); // départ du chrono 1


    w2 = tsp_prog_dyn(V, n, P2);
    s2 = TopChrono(1); // s=durée

    printf("value: %g\n", w2);
    printf("runing time: %s\n", s2);
    printf("\n");
    
    printf("Nearest : Red path\n");
    for (int i = 0; i < n; i++) P3[i] = i;
    TopChrono(0); // initialise tous les chronos
    TopChrono(1); // départ du chrono 1


    w3 = tsp_plus_proche(V, n, P3);
    w3 = tsp_twist(V,n,P3);
    s3 = TopChrono(1); // s=durée

    printf("value: %g\n", w3);
    printf("runing time: %s\n", s3);
    printf("\n");
    
    // Affiche le résultat (q pour sortir)
    while (running) {

        if (need_redraw) {
            draw(V, n, P, P2, P3);
            //drawPath(V,n,P3,lenPath%n);
        }

        need_redraw = handleEvent(wait_event);
    }

    // Libération de la mémoire
    TopChrono(-1);
    free(V);
    free(P);

    cleaning();
    return 0;
}
