#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "variables.h"

// Initialisation de SDL et OpenGL
void init_SDL_OpenGL(void);

// Libération de mémoire
void cleaning_SDL_OpenGL(void);

// Gestion des évènements
bool handleEvent(bool wait_event);

// Convertit les coordonnées pixels en coordonnée dans le dessin
void pixelToCoord(int pixel_x, int pixel_y, double *x, double *y);

// Récupère les coordonnées du centre de la fenêtre
void getCenterCoord(double *x, double *y);

// Zoom centré en (x,y) en mutlipliant par scale
void zoomAt(double scale, double x, double y);

// Zooms centré au pixel (x,y)
void zoomPixel(double scale, int mouse_x, int mouse_y ) ;
void zoomPixelIn(int,int);
void zoomPixelOut(int,int);

// Primitives de dessin
void drawLine(point,point);
void drawPoint(point);
void selectColor(double,double,double);

// Dessin & construction de grille, le point (0,0) de la grille étant
// le coin en haut à gauche
void makeImage(grid*);
void drawGrid(grid);
grid initGridLaby(int,int);
grid initGridPoints(int,int,int,double);
grid initGridFile(char*);
void addRandomBlob(grid,int,int);
void freeGrid(grid);
position randomPosition(grid,int);

#endif
