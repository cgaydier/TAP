#include <SDL2/SDL_opengl.h>
#include <stdbool.h>
// Variables globales pour ne pas s'embêter

// Taille de la fenêtre mise à jour celle-ci est redimensionnée
int width, height;

double scale;        // Zoom courrant
bool mouse_down;     // Bouton de la souris enfoncé
bool running;        // Dans la boucle principale

// Objets SDL
SDL_Window *window;
SDL_GLContext glcontext;