#ifndef VARIABLES_H
#define VARIABLES_H

// Variables globales pour ne pas s'embêter

#include "OpenGL_SDL.h"
#include <stdbool.h>

typedef struct{
  double x, y;
} point;

enum{
  V_ROOM = 0, // Case vide
  V_WALL,     // Mur
  V_SAND,     // Sable
  V_WATE,     // Eau
  V_BURN,     // Feu
  V_WIND,     // Vent ?

  V_NUM, // Nombre de types d'obstacle

  M_USED = 0x01,
  M_PATH = 0x02,
  M_SEEN = 0x04,
  M_NEXT = 0x08
};

typedef struct{
  int x,y;
} position;

typedef struct{
  int X,Y;     // dimensions: X et Y
  int **value; // valuation: value[i][j], i=0..X-1, j=0..Y-1
  int **mark;  // marquage: mark[i][j], i=0..X-1, j=0..Y-1
  position start, end;
} grid;

double costs[V_NUM];

// Taille de la fenêtre mise à jour celle-ci est redimensionnée
int width, height;

double scale;                          // Zoom courrant
bool mouse_left_down, mouse_right_down;// Boutons de la souris enfoncé
bool running;                          // Dans la boucle principale

// Délais d'affichage pour gridDraw()
int delay; // unité = 10 ms

// Objets SDL
SDL_Window *window;
SDL_GLContext glcontext;

#endif