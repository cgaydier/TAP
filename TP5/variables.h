#ifndef VARIABLES_H
#define VARIABLES_H

// Variables globales pour ne pas s'embêter

#include "OpenGL_SDL.h"
#include <stdbool.h>

typedef struct{
  double x,y;
} point;

typedef struct{
  int x,y;
} position;

typedef struct{
  int X,Y;        // dimensions: X et Y
  int **value;    // valuation des cases: value[i][j], 0<=i<X, 0<=j<Y
  int **mark;     // marquage des cases: mark[i][j], 0<=i<X, 0<=j<Y
  position start; // position de la source
  position end;   // position de la destination
} grid;

// valeurs possibles des cases d'une grille pour .value et .mark
// l'ordre doit être cohérent avec color[] et weight[]
enum{

  // pour .value
  V_FREE=0, // case vide
  V_WALL,   // Mur
  V_SAND,   // Sable
  V_WATER,  // Eau
  V_MUD,    // Boue
  V_GRASS,  // Herbe
  V_TUNNEL, // Tunnel

  // pour .mark
  M_NULL, // sommet non marqué
  M_USED, // sommet marqué dans P
  M_FRONT,// sommet marqué dans Q
  M_PATH, // sommet dans le chemin

  // divers
  C_START,    // couleur de la position de départ
  C_END,      // idem pour la destination
  C_FINAL,    // couleur de fin du dégradé pour M_USED
  C_END_WALL, // couleur de destination si sur V_WALL

};

typedef struct{
  // l'ordre de la déclaration est important
  GLubyte R;
  GLubyte G;
  GLubyte B;
} RGB;

extern RGB color[]; // couleurs définies dans utils.c

// faux ssi drawXXX() peut être sauté si l'affichage a eut lieu il y a
// moins de 1/50s, ce qui accélère l'affichage. Mettre = true pour
// forcer l'affichage
bool update;

int width, height;     // dimensions de la fenêtre graphique courante 
bool mouse_left_down;  // bouton gauche de souris enfoncé ?
bool mouse_right_down; // bouton droit de souris enfoncé ?
double scale;          // zoom courrant = nombre de pixels par unité
bool running;          // dans la boucle principale, presser 'q' pour false
int delay;             // délais d'affichage pour drawGrid(), unité = 0"01

// Objets SDL/GL
SDL_Window *window;
SDL_GLContext glcontext;
GLvoid *gridImage;
GLuint texName;

#endif
