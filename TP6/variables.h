#include <SDL2/SDL_opengl.h>
#include <stdbool.h>
// Variables globales pour ne pas s'embêter

#define NUM_VERTICES 1000

typedef struct {
	double x, y;
} point;

point *vertices;

// Taille de la fenêtre mise à jour celle-ci est redimensionnée
int width, height;

double scale;                          // Zoom courrant
bool mouse_left_down, mouse_right_down;// Boutons de la souris enfoncé
bool running;                          // Dans la boucle principale

// Objets SDL
SDL_Window *window;
SDL_GLContext glcontext;