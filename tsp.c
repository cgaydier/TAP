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
    for (int i = k; i < n;i++) {
        int j=n-1;
        while (Tab[j].use == 1 && j > 0) {
            j--;
        }
        if(Tab[j].use == 0) {
            P[i] = Tab[j].val;
            Tab[j].use = 1;
        }
    }

}


static double tsp_brute_force(point *V, int n, int *P) {
    int tmp[n];
    P[0]=0;
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

// Taille initiale de la fenêtre
int width = 640;
int height = 480;

bool running = true;
bool mouse_down = false;
double scale = 1.0f;

static void draw(point *V, int n, int *P) {
    // Efface la fenêtre
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dessin…
    // Choisir la couleur blanche
    selectColor(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < n; i++)
        drawLine(V[P[i]], V[P[(i + 1) % n]]);
    // Rouge
    selectColor(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < n; i++)
        drawPoint(V[i]);

    handleEvent(false);

    // Affiche le dessin
    SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[]) {

    initSDLOpenGL();
    srand(0xc0ca);
    bool need_redraw = true;
    bool wait_event = true;

    int n = 10;
    int X = 300, Y = 200;
    point *V = generatePoints(n, X, Y);
    int *P = malloc(n * sizeof (int));
    for (int i = 0; i < n; i++) P[i] = i;

    TopChrono(0); // initialise tous les chronos
    TopChrono(1); // départ du chrono 1

    double w = tsp_brute_force(V, n, P);

    char *s = TopChrono(1); // s=durée

    printf("value: %g\n", w);
    printf("runing time: %s\n", s);

    // Affiche le résultat (q pour sortir)
    while (running) {

        if (need_redraw)
            draw(V, n, P);

        need_redraw = handleEvent(wait_event);
    }

    // Libération de la mémoire
    TopChrono(-1);
    free(V);
    free(P);

    cleaning();
    return 0;
}
