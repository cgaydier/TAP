#include <stdbool.h>

// Initialisation de SDL et OpenGL
void initSDLOpenGL();
// Libération de mémoire
void cleaning();

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
void zoomPixelIn(int x, int y);
void zoomPixelOut(int x, int y);

// Primitives de dessin
void drawLine(point p1, point p2);
void drawPoint(point p);
void selectColor(double red, double green, double blue);