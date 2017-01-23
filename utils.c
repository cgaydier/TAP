#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "variables.h"
#include "utils.h"

void cleaning() {
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void drawLine(point p1, point p2) {
	glBegin(GL_LINES);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();
}

void drawPoint(point p) {
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex2f(p.x, p.y);
	glEnd();
}

void getCenterCoord(double *x, double *y) {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	pixelToCoord((viewport[0] + viewport[2]) / 2, (viewport[1] + viewport[3]) / 2, x, y);
}

bool handleEvent(bool wait_event) {
	SDL_Event e;
	bool need_redraw = false;

	if (wait_event)
		SDL_WaitEvent(&e);
	else if (!SDL_PollEvent(&e))
		return need_redraw;
	do {
		switch (e.type) {
		case SDL_KEYDOWN:
			if (e.key.keysym.sym != SDLK_q)
				break;
		case SDL_QUIT:
			running = false;
			break;
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				double x,y;
				getCenterCoord(&x, &y);
				glViewport(0, 0, e.window.data1, e.window.data2);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(0.0, e.window.data1, e.window.data2, 0.0f, 0.0f, 1.0f);
				glTranslatef(e.window.data1/2-x, e.window.data2/2-y, 0.0f);
				zoomAt(scale, x, y);
				need_redraw = true;
			}
			break;
		case SDL_MOUSEWHEEL:
			if (e.wheel.y > 0) {
				int x,y;
				SDL_GetMouseState(&x, &y);
				zoomPixelIn(x, y);
				scale *= 2;
				need_redraw = true;
			} else if (e.wheel.y < 0) {
				int x,y;
				SDL_GetMouseState(&x, &y);
				zoomPixelOut(x, y);
				scale *= 0.5;
				need_redraw = true;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button == SDL_BUTTON_LEFT)
				mouse_down = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button == SDL_BUTTON_LEFT)
				mouse_down = false;
			break;
		case SDL_MOUSEMOTION:
			if (mouse_down) {
				glTranslatef(e.motion.xrel / scale, e.motion.yrel / scale, 0);
				need_redraw = true;
			}
		}
	} while(SDL_PollEvent(&e));

	return need_redraw;
}


void initSDLOpenGL() {
	// Initialisation de SDL
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"TP Techniques algorithmiques",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN
	);

	if (window == NULL) {
		// Echec lors de la création de la fenêtre
		printf("Could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	SDL_GetWindowSize(window, &width, &height);
	// Contexte OpenGL
	glcontext = SDL_GL_CreateContext(window);

	// Projection de base, un point OpenGL == un pixel
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, height, 0.0, 0.0f, 1.0f);
}

void pixelToCoord(int pixel_x, int pixel_y, double *x, double *y) {
	GLdouble ray_z;
	GLint viewport[4];
	GLdouble proj[16];
	GLdouble modelview[16];

	// we query OpenGL for the necessary matrices etc.
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	GLdouble _X = pixel_x;
	GLdouble _Y = viewport[3] - pixel_y;

	// using 1.0 for winZ gives u a ray
	gluUnProject(_X, _Y, 1.0f, modelview, proj, viewport, x, y, &ray_z);
}

void selectColor(double red, double green, double blue) {
	glColor3f(red, green, blue);
}

void zoomAt(double scale, double x, double y) {
	glTranslatef(x,y,0);
	glScalef(scale, scale, 1.0);
	glTranslatef(-x,-y,0);
}

void zoomPixel(double scale, int mouse_x, int mouse_y ) {
	double x,y;
	pixelToCoord(mouse_x, mouse_y, &x, &y);
	zoomAt(scale,x,y);
}

void zoomPixelIn(int x, int y) {
	zoomPixel(2.0, x, y);
}

void zoomPixelOut(int x, int y) {
	zoomPixel(0.5, x, y);
}