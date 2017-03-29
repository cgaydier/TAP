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

// taille initiale de la fenêtre
int width = 640;
int height = 480;

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


static double dist(point p1, point p2) {
    return sqrt((p2.x - p1.x)*(p2.x - p1.x) +
            (p2.y - p1.y)*(p2.y - p1.y));
}

int compareX( const void* a, const void* b)
{
     point point_1 = * ( (point*) a );
     point point_2 = * ( (point*) b );

     if ( point_1.x == point_2.x ) return 0;
     else if ( point_1.x < point_2.x ) return -1;
     else return 1;
}


int compareY( const void* a, const void* b)
{
     point point_1 = * ( (point*) a );
     point point_2 = * ( (point*) b );

     if ( point_1.y == point_2.y ) return 0;
     else if ( point_1.y < point_2.y ) return -1;
     else return 1;
}

point* pppp_rec(point* Px,int nX, point* Py, int nY) {
	if(nX == 1) {
		point *ret = malloc(sizeof(point)*2);
		ret[0].x = 0;
		ret[0].y = 0;
		ret[1].x = width;
		ret[1].y = height;
		return ret;
	}
	if(nX == 2) {
		point *ret = malloc(sizeof(point)*2);
		ret[0] = Px[0];
		ret[1] = Px[1];
		return ret;
	}
	if(nX == 3) {
		double val;
		point *ret = malloc(sizeof(point)*2);
		double min = dist(Px[0],Px[1]);
		ret[0] = Px[0];
		ret[1] = Px[1];
		if((val = dist(Px[0],Px[2])) < min) {
			ret[1] = Px[2];
			min = val;
		}
		if((val = dist(Px[1],Px[2])) < min) {
			ret[0] = Px[1];
			ret[1] = Px[2];
		}
		return ret;
	}
	point x_med = Px[nX/2];
	point Ax[nX],Bx[nX];
	point Ay[nY],By[nY];
	int size_A = 0;
	int size_B = 0;
	for(int i=0; i < nX; i++) {
		if(Px[i].x <= x_med.x) {
			Ax[size_A] = Px[i];
			Ay[size_A] = Px[i];
			size_A++;
		}
	}
	for(int i=0; i < nX; i++) {
		if(Px[i].x > x_med.x) {
			Bx[size_B] = Px[i];
			By[size_B] = Px[i];
			size_B++;
		}
	}
	qsort(Ax,size_A,sizeof(point),compareX);
	qsort(Ay,size_A,sizeof(point),compareY);
	qsort(Bx,size_B,sizeof(point),compareX);
	qsort(By,size_B,sizeof(point),compareY);
	
	
	point *pppp_A = pppp_rec(Ax,size_A,Ay,size_A);
	point *pppp_B = pppp_rec(Bx,size_B,By,size_B);
	
	double delta = dist(pppp_A[0],pppp_A[1]);
	if(dist(pppp_B[0],pppp_B[1]) < delta) {
		delta = dist(pppp_B[0],pppp_B[1]);
	}
	
	
	point S[nX];
	int size_S = 0;

	for(int i = 0; i < nY;i++) {
		if(fabs(Py[i].x - x_med.x) < delta) {
			S[size_S++] = Py[i];
		}
	}
	
	point pppp_S[2];
	pppp_S[0] = S[0];
	pppp_S[1] = S[1];
	double min_S = dist(S[0],S[1]);
	for(int i = 0; i < size_S; i++) {
		for(int j = i+1; j < i+8 && j < size_S; j++) {
			if(dist(S[i],S[j]) < min_S) {
				min_S = dist(S[i],S[j]);
				pppp_S[0] = S[i];
				pppp_S[1] = S[j];
			}
		}
	}
	
	double min_A = dist(pppp_A[0],pppp_A[1]);
	double min_B = dist(pppp_B[0],pppp_B[1]);
	
	point *result = malloc(sizeof(point)*2);
	if(min_S < min_A) {
		if(min_S < min_B) {
			result[0] = pppp_S[0];
			result[1] = pppp_S[1];
		} else {
			result[0] = pppp_B[0];
			result[1] = pppp_B[1];
		}
	} else {
		if(min_A < min_B) {
			result[0] = pppp_A[0];
			result[1] = pppp_A[1];
		} else {
			result[0] = pppp_B[0];
			result[1] = pppp_B[1];
		}
	}
	
	return result;
}
point* pppp(point* P,int n) {
	point *Px = malloc(sizeof(point)*n);
	point *Py = malloc(sizeof(point)*n);
	for(int i = 0; i < n; i++) {
		Px[i] = P[i];
		Py[i] = P[i];
	}
	qsort(Px,n,sizeof(point),compareX);
	qsort(Py,n,sizeof(point),compareY);
	return pppp_rec(Px,n,Py,n);
}



void drawPPPP(point *P,int n,point *PP) {
 	// Saute le dessin si le précédent a été fait il y a moins de 20ms
    static unsigned int last_tick = 0;
    if (last_tick + 20 > SDL_GetTicks()) return;
    last_tick = SDL_GetTicks();

    // Gestion de la file d'event
    handleEvent(false);

    // Efface la fenêtre
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    selectColor(1, 0, 0);
    for (int i = 0; i < n; i++) drawPoint(P[i]);
    
    selectColor(0, 1, 0);
   	drawLine(PP[0],PP[1]);
   	drawPoint(PP[0]);
   	drawPoint(PP[1]);
    SDL_GL_SwapWindow(window);

}


// pour la fenêtre graphique
bool running = true;
bool mouse_down = false;
double scale = 1;


int main(int argc,char **argv) {
	initSDLOpenGL();
    srandom(46);
    //Essayez: srandom(783) ou srandom(46) avec n=40 points
    bool need_redraw = true;
    bool wait_event = true;

    int n = (argv[1] && atoi(argv[1])) ? atoi(argv[1]) : 400;
    point *V = generatePoints(n, width, height);
    point *PP = NULL;
    while (running) {
        wait_event = true;
        if (need_redraw) {
		    PP = pppp(V, n);
        }
        drawPPPP(V,n,PP);
        need_redraw = handleEvent(wait_event);
    }
	return EXIT_SUCCESS;
}
