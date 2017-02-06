/*
**  Quelques routines
*/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define SWAP(x,y,z)  (z)=(x),(x)=(y),(y)=(z) /* échange x et y, via z */
#define CHRONOMAX 10

extern char *TopChrono(const int i){
/*
  Met à jour le chronomètre interne numéro i (i=0..CHRNONMAX-1) et
  renvoie sous forme de char* le temps écoulé depuis le dernier appel
  à la fonction pour le même chronomètre. La précision dépend du temps
  mesuré. Il varie entre la seconde et le 1/1000 de seconde. Plus
  précisément le format est le suivant:

  1d00h00'  si le temps est > 24h (précision: 1')
  1h00'00"  si le temps est > 60' (précision: 1s)
  1'00"0    si le temps est > 1'  (précision: 1/10s)
  1"00      si le temps est > 1"  (précision: 1/100s)
  0"000     si le temps est < 1"  (précision: 1/1000s)

  Pour initialiser et mettre à jour tous les chronomètres (dont le
  nombre vaut CHRONOMAX), il suffit d'appeler une fois la fonction,
  par exemple avec TopChrono(0). Si i<0, alors les pointeurs alloués
  par l'initialisation sont désalloués. La durée maximale est limitée
  à 100 jours. Si une erreur se produit (durée supérieure ou erreur
  avec gettimeofday()), alors on renvoie la chaîne "--error--".
*/
  if(i>=CHRONOMAX) return "--error--";

  /* variables globales, locale à la fonction */
  static int first=1; /* =1 ssi c'est la 1ère fois qu'on exécute la fonction */
  static char *str[CHRONOMAX];
  static struct timeval last[CHRONOMAX],tv;
  int j;

  if(i<0){ /* libère les pointeurs */
    if(!first) /* on a déjà alloué les chronomètres */
      for(j=0;j<CHRONOMAX;j++)
	free(str[j]);
    first=1;
    return NULL;
  }

  /* tv=temps courant */
  int err=gettimeofday(&tv,NULL);

  if(first){ /* première fois, on alloue puis on renvoie TopChrono(i) */
    first=0;
    for(j=0;j<CHRONOMAX;j++){
      str[j]=malloc(10); // assez grand pour "--error--", "99d99h99'" ou "23h59'59""
      last[j]=tv;
    }
  }

  /* t=temps en 1/1000" écoulé depuis le dernier appel à TopChrono(i) */
  long t=(tv.tv_sec-last[i].tv_sec)*1000L + (tv.tv_usec-last[i].tv_usec)/1000L;
  last[i]=tv; /* met à jour le chrono interne i */
  if((t<0L)||(err)) t=LONG_MAX; /* temps erroné */

  /* écrit le résultat dans str[i] */
  for(;;){ /* pour faire un break */
    /* ici t est en millième de seconde */
    if(t<1000L){ /* t<1" */
      sprintf(str[i],"0\"%03li",t);
      break;
    }
    t /= 10L; /* t en centième de seconde */
    if(t<6000L){ /* t<60" */
      sprintf(str[i],"%li\"%02li",t/100L,t%100L);
      break;
    }
    t /= 10L; /* t en dixième de seconde */
    if(t<36000L){ /* t<1h */
      sprintf(str[i],"%li'%02li\"%li",t/360L,(t/10L)%60L,t%10L);
      break;
    }
    t /= 10L; /* t en seconde */
    if(t<86400L){ /* t<24h */
      sprintf(str[i],"%lih%02li'%02li\"",t/3600L,(t/60L)%60L,t%60L);
      break;
    }
    t /= 60L; /* t en minute */
    if(t<144000){ /* t<100 jours */
      sprintf(str[i],"%lid%02lih%02li'",t/1440L,(t/60L)%24L,t%60L);
      break;
    }
    /* error ... */
    sprintf(str[i],"--error--");
  }

  return str[i];
}


extern int NextPerm(int *P,const int n,const int *C){
/*
  Génère, à partir d'une permutation P, la prochaine dans l'ordre
  lexicographique suivant les contraintes définies par le tableau C
  (voir ci-après). Mettre C=NULL s'il n'y a pas de contrainte
  particulière. On renvoie 1 si la prochaine permutation à pu être
  déterminée et 0 si la dernière permutation a été atteinte. Dans ce
  cas la permutation la plus petite selon l'ordre lexicographique est
  renvoyée. On permute les éléments de P que si leurs positions sont
  entre C[j] et C[j+1] (exclu) pour un certain indice j.

  Ex: si C={2,3,5}. Les permutations sous la contrainte C sont:
  (on peut permuter les indices {0,1}{2}{3,4})

                 0 1 2 3 4 (positions dans P)
	      P={a,b,c,d,e}
	        {b,a,c,d,e}
		{a,b,c,e,d}
		{b,a,c,e,d}

  Evidemment, il y a beaucoup moins de permutations dès que le nombre
  de contraintes augmente. Par exemple, si C contient k intervalles de
  même longueur, alors le nombre de permutations sera de (n/k)!^k au
  lieu de n!. Le rapport des deux est d'environ k^n.

  Concrêtement, pour:
  - n=9 et k=3, on a 216 permutations au lieu de 362.880 (k^n=19.683)
  - n=12 et k=3, on a 13.824 permutations au lieu de 479.001.600 (k^n=531.441)

  Le dernier élément de C doit être égale à n-1 (sentinelle), le
  premier étant omis car toujours = 0. Donc C est un tableau à au plus
  n éléments. Si C=NULL, alors il n'y a pas de contrainte
  particulière, ce qui est identique à poser C[0]=n.

  On se base sur l'algorithme classique (lorsque C=NULL, sinon on
  l'applique sur l'intervalle de positions [C[j],C[j+1][):

  1. Trouver le plus grand index i tel que P[i] < P[i+1].
     S'il n'existe pas, la dernière permutation est atteinte.
  2. Trouver le plus grand indice j tel que P[i] < P[j].
  3. Echanger P[i] avec P[j].
  4. Renverser la suite de P[i+1] jusqu'au dernier élément.

*/
  int i,j,a,b,c,T[1];

  if(C==NULL){
    T[0]=n;
    C=T;
  }

  b=C[i=j=0]; /* j=indice de la prochaine valeur à lire dans C */
  c=-1;

  /* étape 1: on cherche l'intervalle [a,b[ avec i tq P[i]<P[i+1] */
 etape1:
  for(a=i;i<b-1;i++) if(P[i]<P[i+1]) c=i; /* on a trouvé un i tq P[i]<P[i+1] */
  if(c<0){ /* le plus grand i tq P[i]<[i+1] n'existe pas */
    for(i=a;i<b;i++) P[i]=i; /* on réinitialise P[a]...P[b-1] */
    if(b==n) return 0; /* alors on a fini d'examiner C */
    b=C[++j]; /* b=nouvelle borne */
    goto etape1;
  }
  i=c; /* i=le plus grand tq P[i]<P[i+1] avec a<=i,i+1<b */

  /* étape 2: cherche j=le plus grand tq P[i]<P[j] */
  for(j=i+1;j<b;j++) if(P[i]<P[j]) c=j;
  j=c;

  /* étape 3: échange P[i] et P[j] */
  SWAP(P[i],P[j],c);

  /* étape 4: renverse P[i+1]...P[b-1] */
  for(++i,--b;i<b;i++,b--) SWAP(P[i],P[b],c);

  return 1;
}


extern inline int NextPermutation(int *P,const int n){
  return NextPerm(P,n,NULL);
}

extern void Permute(point *T,const int n){
/*
  Permute aléatoirement les n premiers éléments de T.
*/
  int i,j;
  point k;
  for(i=0;i<n;i++){
    j=i+(random()%(n-i));
    SWAP(T[i],T[j],k);
  }
}
