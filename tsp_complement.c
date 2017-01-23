static void drawPath(point *V, int n, int *path, int len);

/* ==== Programmation dynamique ====*/

static int NextSet(unsigned S, int n){
/*
  Renvoie le plus petit entier immédiatement supérieure à S>0 et qui a
  soit le même poids que S (c'est-à-dire le même nombre de 1 dans son
  écriture binaire que S), soit le poids de S plus 1 s'il n'y en a pas
  de même poids. La fonction renvoie 0 si S est le dernier entier sur
  n bits, soit S=2^n-1. Pensez à inclure <string.h>.
*/
  int p1 = ffs(S);
  int p2 = ffs(~(S >> p1)) + p1 - 1;
  if (p2 - p1 + 1 == n) return 0; // cas S=2^n
  if (p2 == n) return (1 << (p2 - p1 + 2)) - 1; // cas: poids(S)+1
  int mask = (-1) << p2;
  return (S & mask) | 1 << p2 | ((1 << (p2 - p1)) - 1); // cas: poids(S)
}

#define IN_SET(S,i)  ((S) & (1 << (i))) // est-ce que i est dans S ?
#define ADD_SET(S,i) (S | (1 << (i)))   // ajoute i à S
#define DEL_SET(S,i) (S & ~(1 << (i)))  // supprimer i de S

/* une cellule de la table */
typedef struct{
  int v;
  double d;
} cell;

static double tsp_prog_dyn(point *V, int n, int *Q){
/*
  Version programmation dynamique du TSP. Le résultat doit être écrit
  dans la permutation Q. On renvoie la valeur de la tournée minimum ou
  -1 s'il y a eut une erreur.

  Attention ! Par rapport au cours, il est plus malin de commencer la
  tournée à partir du sommet d'indice n-1 (au lieu de 0). Pourquoi ?

  Donc D[t][S] = coût minimum d'un chemin allant de V[n-1] à V[t] qui
  visite tous les sommets d'indice dans S.

  Il sera intéressant de dessiner à chaque fois que possible le chemin
  courant avec drawPath()
*/


  dyn_dist **D;
  int S;

  ...
    
    do{
      
      ...
      
         drawPath(V, n, Q, i+1); // dessine le chemin courant

      ...
	
    }while ((S = nextSet(S, n-1)) && running);

    // ne pas reconstruire la permutation si le calcul a été interrompu
    if(!running){
      free ...
      return -1;
    }

    // tournée_min

    
    free ...
    return ...;
}


static void drawPath(point *V, int n, int *path, int len) {
	// Saute le dessin si le précédent a été fait il y a moins de 20ms
	static unsigned int last_tick = 0;
	if (last_tick + 20 > SDL_GetTicks())
		return;
	last_tick = SDL_GetTicks();

	// Gestion de la file d'event
	handleEvent(false);

	// Efface la fenêtre
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Dessin …
	// Choisir la couleur blanche
	selectColor(0.0f, 1.0f, 0.0f);
	for (int i = 0 ; i < len-1 ; i++)
		drawLine(V[path[i]], V[path[i+1]]);
	// Rouge
	selectColor(1.0f, 0.0f, 0.0f);
	for (int i = 0 ; i < n ; i++)
		drawPoint(V[i]);

	// Affiche le dessin
	SDL_GL_SwapWindow(window);
}
