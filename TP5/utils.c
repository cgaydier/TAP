#include <stdio.h>
#include <stdlib.h>

#include "OpenGL_SDL.h"
#include "variables.h"
#include "utils.h"

static GLvoid *gridImage;
static GLuint texName;

// Construit l'image de la grille à partir de la grille G
void makeImage(grid G){
	GLubyte *I = gridImage;
	for(int i=0; i<G.X; i++){
		for(int j=0; j<G.Y; j++){
		int k = 3*(i*G.Y+j);
			if(G.value[i][j] == V_WALL){
				I[k+0] = 0;
				I[k+1] = 0;
				I[k+2] = 0;
			}
			else {
				if(G.mark[i][j] & M_USED){
					I[k+0] = 0;
					I[k+1] = 255;
					I[k+2] = 0;
					if(G.mark[i][j] & M_PATH){
						I[k+0] = 0;
						I[k+1] = 0;
						I[k+2] = 255;
					}
				}
				else{
					I[k+0] = 255;
					I[k+1] = 255;
					I[k+2] = 255;
				}
			}
		}
	}
	int k = 3*(G.start.x*G.Y+G.start.y);
	I[k+0] = 255;
	I[k+1] = 0;
	I[k+2] = 0;
	k = 3*(G.end.x*G.Y+G.end.y);
	I[k+0] = 255;
	I[k+1] = 0;
	I[k+2] = 0;
}


// Alloue une grille aux dimensions x,y
// ainsi que son image
grid allocGrid(int x, int y){
  grid G;
  G.X = x;
  G.Y = y;
  G.value = malloc(x*sizeof(*(G.value)));
  G.mark = malloc(x*sizeof(*(G.mark)));

  for(int i=0; i<x; i++){
    G.value[i] = malloc(y*sizeof(*(G.value[i])));
    G.mark[i] = calloc(y, sizeof(*(G.mark[i])));
  }

  gridImage = malloc(3*x*y*sizeof(GLubyte));
  return G;
}


// Libère les pointeurs alloués par allocGrid()
void freeGrid(grid G){
  for(int i=0; i<G.X; i++){
    free(G.value[i]);
    free(G.mark[i]);
  }
  free(G.value);
  free(G.mark);
  free(gridImage);
}


// Grille aléatoire de dimensions x,y
// à partir d'un labyrinthe aléatoire
grid initGrid(int x, int y){
  grid G = allocGrid(x,y); // alloue la grille et son image

  G.start.x = G.X-2;
  G.start.y = G.Y-2;
  G.end.x = 1;
  G.end.y = 1;

  for(int i=0; i<G.X; i++)
    for(int j=0; j<G.Y; j++)
      G.value[i][j] = V_WALL;

  const int size = G.X / 2;
  struct edge_t { int i, j, k, l; } *edges = malloc(2*size*(size-1)*sizeof(*edges));
  int num_edges = 2;
  edges[0] = (struct edge_t) { .i = size-1, .j = 0, .k = size-2, .l = 0 };
  edges[1] = (struct edge_t) { .i = size-1, .j = 0, .k = size-1, .l = 1 };
  int num_v = G.X*G.Y - 1;
  G.value[G.X-2][1] = V_ROOM;
  while(num_edges && num_v){
    int e = random() % num_edges;
    struct edge_t edge = edges[e];
    edges[e] = edges[--num_edges];
    int k = edge.k, l = edge.l;
    if (G.value[k*2+1][l*2+1] == V_WALL){
      num_v--;
      G.value[k*2+1][l*2+1] = V_ROOM;
      G.value[edge.i + k + 1][edge.j + l + 1] = V_ROOM;
      if (edge.k > 0 && G.value[(k-1)*2+1][l*2+1])
	edges[num_edges++] = (struct edge_t) { .i = k, .j = l, .k = k-1, .l = l };
      if (edge.l > 0 && G.value[k*2+1][(l-1)*2+1])
	edges[num_edges++] = (struct edge_t) { .i = k, .j = l, .k = k, .l = l-1 };
      if (edge.k < size-1 && G.value[(k+1)*2+1][l*2+1])
	edges[num_edges++] = (struct edge_t) { .i = k, .j = l, .k = k+1, .l = l };
      if (edge.l < size-1 && G.value[k*2+1][(l+1)*2+1])
	edges[num_edges++] = (struct edge_t) { .i = k, .j = l, .k = k, .l = l+1 };
    }
  }
  free(edges);
  return G;
}


void drawGrid(grid G){
  // saute le dessin si le précédent a été fait il y a moins de 100ms
  static unsigned int last_tick = 0;
  //if(last_tick + 20 > SDL_GetTicks()) return;

  last_tick = SDL_GetTicks();

  handleEvent(false);

  // Efface la fenêtre
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Dessin
  makeImage(G);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, G.X, G.Y, 0, GL_RGB, GL_UNSIGNED_BYTE, gridImage);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glBindTexture(GL_TEXTURE_2D, texName);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(0,     0, 0);
  glTexCoord2f(0.0, 1.0); glVertex3f(0,   G.Y, 0);
  glTexCoord2f(1.0, 1.0); glVertex3f(G.X, G.Y, 0);
  glTexCoord2f(1.0, 0.0); glVertex3f(G.X,   0, 0);
  glEnd();
  glFlush();
  glDisable(GL_TEXTURE_2D);

  // Affiche le résultat après un certain délais
  SDL_Delay(delay);
  SDL_GL_SwapWindow(window);
}

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
	bool has_changed = false;

	if (wait_event)
		SDL_WaitEvent(&e);
	else if (!SDL_PollEvent(&e))
		return has_changed;
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
			}
			break;
		case SDL_MOUSEWHEEL:
			if (e.wheel.y > 0) {
				int x,y;
				SDL_GetMouseState(&x, &y);
				zoomPixelIn(x, y);
				scale *= 2;
			} else if (e.wheel.y < 0) {
				int x,y;
				SDL_GetMouseState(&x, &y);
				zoomPixelOut(x, y);
				scale *= 0.5;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button == SDL_BUTTON_LEFT) {
				double x, y;
				pixelToCoord(e.motion.x, e.motion.y, &x, &y);
				mouse_left_down = true;
			}
			if (e.button.button == SDL_BUTTON_RIGHT)
				mouse_right_down = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button == SDL_BUTTON_LEFT) {
				mouse_left_down = false;
			}
			if (e.button.button == SDL_BUTTON_RIGHT)
				mouse_right_down = false;
			break;
		case SDL_MOUSEMOTION:
			if (mouse_right_down) {
				glTranslatef(e.motion.xrel / scale, e.motion.yrel / scale, 0);
			}
		}
	} while(SDL_PollEvent(&e));

	return has_changed;
}


void initSDLOpenGL(void){
	// Initialisation de SDL
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"TP Techniques algorithmiques",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE
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
	glScalef(scale, scale, 1.0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
