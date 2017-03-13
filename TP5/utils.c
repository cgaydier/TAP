#include <stdio.h>
#include <stdlib.h>

#include "OpenGL_SDL.h"
#include "variables.h"
#include "utils.h"

RGB color[]={
  {0xE0,0xE0,0xE0}, // V_FREE
  {0x00,0x00,0x00}, // V_WALL
  {0xF0,0xD8,0xA8}, // V_SAND
  {0x00,0x6D,0xBA}, // V_WATER
  {0x7C,0x70,0x56}, // V_MUD
  {0x00,0xA0,0x60}, // V_GRASS
  {0x70,0xE0,0xD0}, // V_TUNNEL
  {0x80,0x80,0x80}, // M_NULL
  {0x12,0x66,0x66}, // M_USED
  {0x08,0xF0,0xF0}, // M_FRONT
  {0x90,0x68,0xF8}, // M_PATH
  {0xFF,0x00,0x00}, // C_START
  {0xFF,0x88,0x28}, // C_END
  {0x99,0xAA,0xCC}, // C_FINAL
  {0xFF,0xFF,0x80}, // C_END_WALL
};

bool mouse_left_down = false;
bool mouse_right_down = false;
bool update = false;
bool running = true;
double scale = 1.0;

#define RAND01  ((double)random()/(double)RAND_MAX)  // réel aléatoire dans [0,1[
#define NCOLOR ((int)(sizeof(color)/sizeof(*color))) // nombre de couleurs dans color[]

// Vrai ssi p est une position de la grille Attention ! cela ne veut
// pas dire que p est un sommet du graphe, car la case peut contenir
// V_WALL.
static inline int inGrid(grid *G, position p){
  return (0<=p.x)&&(p.x<G->X)&&(0<=p.y)&&(p.y<G->Y);
}

// Vrai ssi (i,j) est sur le bord de la grille
static inline int onBorder(grid *G, int i, int j){
  return (i==0)||(j==0)||(i==G->X-1)||(j==G->Y-1);
}

// distance Lmax entre s et t
static inline double distLmax(position s, position t){
  return hypot(t.x-s.x,t.y-s.y);
}

// Construit l'image de la grille à partir de la grille G. Le point
// (0,0) de G correspond au coin en haut à gauche.
//
// +--x
// |
// y
//
void makeImage(grid *G){

  static int cpt; // compteur d'étape lorsqu'on reconstruit le chemin

  RGB *I = gridImage, c;
  int k=0,v,m,f;
  int fin=(G->mark[G->start.x][G->start.y]==M_PATH); // si le chemin a fini d'être construit
  int debut=(G->mark[G->end.x][G->end.y]==M_PATH); // si le chemin commence à être construit
  
  if(fin) update=false;
  if(debut==0) cpt=0;
  if(debut) cpt++;
  if(debut && cpt==1){
    update = true;
    delay = 2*delay + 10; // ralenti l'affichage pour le chemin
    printf("delay: %i\n",delay);
  }
  
  for(int j=0; j<G->Y; j++)
    for(int i=0; i<G->X; i++){
      m=G->mark[i][j];  if((m<0)||(m>=NCOLOR)) m=M_NULL;
      v=G->value[i][j]; if((v<0)||(v>=NCOLOR)) v=V_FREE;
      do{
	if(m==M_PATH){ c=color[m]; break; }
	if(fin){ c=color[v]; break; } // affiche la grille d'origine à la fin
	if(m==M_NULL){ c=color[v]; break; } // si pas de marquage
	if(m==M_USED){
	  // interpolation de couleur entre les couleurs M_USED et
	  // C_FINAL ou bien M_USED et v si on est en train de
	  // reconstruire le chemin
	  position p={.x=i,.y=j};
	  double t=distLmax(G->start,G->end);
	  if(t==0) t=1E-10; // pour éviter la division par 0
	  if(debut){
	    t=0.5*cpt/t;
	    f=v;
	  }else{
	    t=distLmax(G->start,p)/t;
	    f=C_FINAL;
	  }
	  t=fmax(t,0.0), t=fmin(t,1.0);
	  c=color[M_USED];
	  c.R += t*(color[f].R - color[M_USED].R);
	  c.G += t*(color[f].G - color[M_USED].G);
	  c.B += t*(color[f].B - color[M_USED].B);
	  break;
	}
      c=(m==M_NULL)? color[v] : color[m];
      break;
      }while(0);
      I[k++]=c;
    }

  if(inGrid(G,G->start)){
    k=G->start.y*G->X+G->start.x;
    I[k]=color[C_START];
  }

  if(inGrid(G,G->end)){
    v=(G->value[G->end.x][G->end.y]==V_WALL)? C_END_WALL : C_END;
    k=G->end.y*G->X+G->end.x;
    I[k]=color[v];
  }
}

// Alloue une grille aux dimensions x,y
// ainsi que son image. On force x,y>=3.
grid allocGrid(int x, int y){
  grid G;
  position p={-1,-1};
  G.start=G.end=p;
  if(x<3) x=3;
  if(y<3) y=3;
  G.X = x;
  G.Y = y;
  G.value = malloc(x*sizeof(*(G.value)));
  G.mark = malloc(x*sizeof(*(G.mark)));

  for(int i=0; i<x; i++){
    G.value[i] = malloc(y*sizeof(*(G.value[i])));
    G.mark[i] = malloc(y*sizeof(*(G.mark[i])));
    for(int j=0; j<y; j++) G.mark[i][j]=M_NULL; // initialise
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


// Renvoie une grille de dimensions x,y rempli de points aléatoires de
// type et de densité donnés. Le départ et la destination sont
// initialisées aléatroirement dans V_FREE.
grid initGridPoints(int x, int y, int type, double density){
  grid G = allocGrid(x,y); // alloue la grille et son image

  // vérifie que le type est correct
  if((type<0)||(type>=NCOLOR)) type=M_NULL;

  // met les bords et remplit l'intérieur
  for(int i=0;i<x;i++)
    for(int j=0;j<y;j++)
      G.value[i][j]=onBorder(&G,i,j)? V_WALL :
	((RAND01<=density)? type : V_FREE);

  // position start/end aléatoires
  G.start=randomPosition(G,V_FREE);
  G.end=randomPosition(G,V_FREE);
  
  return G;
}

// Grille aléatoire de dimensions x,y à partir d'un labyrinthe
// aléatoire. Par défaut start=en bas à droit, end=en haut à gauche
grid initGridLaby(int x, int y){
  
  // pour garantir des dimensions impaires
  x += ((x&1)==0);
  y += ((y&1)==0);

  // alloue la grille et son image
  grid G = allocGrid(x,y);
  
  // position par défaut
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
  G.value[G.X-2][1] = V_FREE;
  while(num_edges && num_v){
    int e = random() % num_edges;
    struct edge_t edge = edges[e];
    edges[e] = edges[--num_edges];
    int k = edge.k, l = edge.l;
    if (G.value[k*2+1][l*2+1] == V_WALL){
      num_v--;
      G.value[k*2+1][l*2+1] = V_FREE;
      G.value[edge.i + k + 1][edge.j + l + 1] = V_FREE;
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

grid initGridFile(char *file){

  FILE *f=fopen(file,"r");
  if(f==NULL){ printf("Cannot open file \"%s\"\n",file); exit(1); }

  char *L=NULL; // L=buffer pour la ligne de texte à lire
  size_t b=0;   // b=taille du buffer L utilisé (nulle au départ)
  ssize_t n;    // n=nombre de caractères lus dans L, sans le '\0'
  
  // Etape 1: on évalue la taille de la grille. On s'arrête si c'est
  // la fin du fichier ou si le 1ère caractère n'est pas un '#'
  
  int x=0; // x=nombre de caractères sur une ligne
  int y=0; // y=nombre de lignes
  
  while((n=getline(&L,&b,f))>0){
    if(L[0]!='#') break;
    if(L[n-1]=='\n') n--; // se termine par '\n' sauf si fin de fichier
    if(n>x) x=n;
    y++;
  }

  rewind(f);
  grid G=allocGrid(x,y);

  // met des bords et remplit l'intérieur
  for(int i=0;i<x;i++)
    for(int j=0;j<y;j++)
      G.value[i][j]=onBorder(&G,i,j)? V_WALL : V_FREE;

  // Etape 2: on relit le fichier et on remplit la grille
  
  int v;
  for(int j=0;j<y;j++){
    n=getline(&L,&b,f);
    if(L[n-1]=='\n') n--; // enlève le '\n' éventuelle
    for(int i=0;i<n;i++){ // ici n<=x
      switch(L[i]){
      case ' ': v=V_FREE;   break;
      case '#': v=V_WALL;   break;
      case ';': v=V_SAND;   break;
      case '~': v=V_WATER;  break;
      case ',': v=V_MUD;    break;
      case '.': v=V_GRASS;  break;
      case '+': v=V_TUNNEL; break;
      case 's': v=V_FREE; G.start.x=i; G.start.y=j; break;
      case 't': v=V_FREE; G.end.x=i;   G.end.y=j;   break;
      default: v=V_FREE;
      }
      G.value[i][j]=v;
    }
  }

  
  
  free(L);
  fclose(f);
  return G;
}

// Ne touche à au bord de la grille.
void addRandomBlob(grid G, int type, int nb){
  int neighs[8][2] = {
    {0, -1}, {1, 0}, {0, 1}, {-1, 0},
    {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
  };
  for (int i = 0 ; i < nb ; i++)
    G.value[1+random()%(G.X-2)][1+random()%(G.Y-2)] = type;

	for (int it = 0 ; it < G.X && it < G.Y; it++)
		for (int i = 2 ; i < G.X-2 ; i++)
			for (int j = 2 ; j < G.Y-2 ; j++) {
				int n = 0;
				for (int k = 0 ; k < 4; k++)
					if (G.value[i+neighs[k][0]][j+neighs[k][1]] == type)
						n++;
				if (n && random() % ((4-n)*20+1) == 0)
					G.value[i][j] = type;
			}
}


// Renvoie une position aléatoire de la grille qui est uniforme parmi
// toutes les valeurs de la grille du type t (hors les bords de la
// grille). Si aucune case de type t n'est pas trouvée, la
// position (-1,-1) est renvoyée.
position randomPosition(grid G, int t){
  int i,j,c;
  int n;    // n=nombre de cases de type t hors le bord
  int r=-1; // r=numéro aléatoire dans [0,n[
  position p={-1,-1}; // position par défaut
  const int stop=G.X*G.Y; // pour sortir des boucles
  const int x1=G.X-1;
  const int y1=G.Y-1;
  
  // On fait deux parcours: un 1er pour compter le nombre n de cases
  // de type t, et un 2e pour tirer au hasard la position parmi les
  // n. A la fin du premier parcours on connaît le nombre n de cases
  // de type t. On tire alors au hasard un numéro r dans [0,n[. Puis
  // on recommence le comptage (n=0) de cases de type t et on s'arrête
  // dès qu'on arrive à la case numéro r.

  c=0;
  do{
    n=0;
    for(i=1; i<x1; i++)
      for(j=1; j<y1; j++)
	if(G.value[i][j]==t){
	  if(n==r) p.x=i, p.y=j, i=j=stop; // toujours faux au 1er parcours
	  n++;
	}
    c=1-c;
    if(c) r=random()%n;
  }while(c); // vrai la 1ère fois, faux la 2e
  
  return p;
}


void drawGrid(grid G){
  
  static unsigned int last_tick = 0;

  // Saute le dessin si update=false et que le précédent a été fait il
  // y a moins de 20ms = 1/50s

  if((!update) && (last_tick+20>SDL_GetTicks())) return;
  last_tick = SDL_GetTicks();

  handleEvent(false);

  // Efface la fenêtre
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Dessin
  makeImage(&G);
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

  // Affiche le résultat puis attend un certain délais
  SDL_GL_SwapWindow(window);
  SDL_Delay(delay);
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
		  if(e.key.keysym.sym == SDLK_q){
		    running=false;
		    update=false;
		    delay=0;
		    break;
		  }
		  if(e.key.keysym.sym == SDLK_a){
		    if(delay>30) delay-=sqrt(delay);
		    else delay--;
		    if(delay<0) delay=0;
		    break;
		  }
		  if(e.key.keysym.sym == SDLK_z){
		    delay+=sqrt(delay);
		    if(delay>100) delay=100;
		    break;
		  }
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


void init_SDL_OpenGL(void){
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

	mouse_left_down = mouse_right_down = false;  // boutons souris relachés

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

void cleaning_SDL_OpenGL(void){
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
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
