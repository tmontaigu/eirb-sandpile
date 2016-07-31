
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <omp.h>
#include <sys/time.h>

#define DIM 128
#define MAX_HEIGHT 99999
#include <math.h>

#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))


unsigned ocean[DIM+1][DIM+1];
unsigned old[DIM+1][DIM+1];
unsigned changed[DIM][DIM];


// vecteur de pixel renvoyé par compute  
struct {
    float R, G, B;
} couleurs[DIM][DIM];

void reset_color(int i, int j) {
    couleurs[i][j].R = 0.0;
    couleurs[i][j].G = 0.0;
    couleurs[i][j].B = 0.0;
}

/*
 * Blue : 3 grains
 * Purple : 2 grains
 * Red : 1 grain
 * Green : That case of sand gave away 4 grains
 */
void colorize(){
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            reset_color(i,j);

            if (ocean[i][j] == 3) {
             couleurs[i][j].B = 1.0;
            }
            else if (ocean[i][j] == 2) {
                couleurs[i][j].B = 0.5;
                couleurs[i][j].R = 0.5;
            }
            else if (ocean[i][j] == 1) {
                couleurs[i][j].R = 1.0;
            }


            if (changed[i][j] == 1) {
                couleurs[i][j].G = 0.75;
            }
            
        }
    }    
}

void reset_change() {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++){ 
            changed[i][j] = 0;
        }
    }
}
// callback
unsigned get (unsigned x, unsigned y)
{
    return ocean[y][x];
}

/*
 * Initialse le tas de sable
 *  0 -> 4 grains sur la case centrale (pour tester sur un terrain 3x3)
 *  1 -> Case centrale sers de source (MAX_HEIGHT grains)
 *  2 -> 3 sources
 *  3 -> Configuration homogene
 */
static void sable_init (int config)
{
    for (int i = 0; i < DIM+1; i++) {
        for (int j = 0; j < DIM+1; j++) {
            ocean[i][j] = 0;
        }
    }

    if (config == 3) { 
        for (int y = 0; y < DIM; y++) {
            for (int x = 0; x < DIM; x++) {
                ocean[y][x] = 5;
            }
        }
    }
    else if (config == 0) {
        ocean[DIM/2][DIM/2] = 4;
    }
    else if (config == 1) {
        ocean[DIM/2][DIM/2] = MAX_HEIGHT;
    }
    else if (config == 2) {
        ocean[DIM/2][DIM/2] = MAX_HEIGHT;
        ocean[DIM/4][(DIM/4)*3] = MAX_HEIGHT;
        ocean[DIM/4][DIM/4] = MAX_HEIGHT;
    }
}

/*
 * Itere sur toutes les cases, chaque grains donne a ses voisins, et s'enlève lui même 4 grains.
 */
float *sequentiel_1 (unsigned iterations)
{
    static int step = 0;
    reset_change();
    for (unsigned i = 0; i < iterations; i++) {
        step++;
        for (int x = 0; x < DIM; x++) {
            for (int y = 0; y < DIM; y++) {
                if (ocean[x][y] >= 4) {
                        
                    ocean[x+1][y] += 1;		  
                    ocean[x-1][y] += 1;
                    ocean[x][y+1] += 1;
                    ocean[x][y-1] += 1;

                    ocean[x][y] -= 4;
                    changed[x][y] = 1;
                }
	        }
	    }
    }
    colorize();
    return couleurs;
}


/*
 * Chaque cases ne donne plus de grains mais regarde ses voisins et s'ajoute le bon nombre de grains
 * les cases s'enlèvent elles mêmes les grains.
 */
float *sequentiel_2(unsigned iterations) {
    reset_change();

   for (int i = 0; i < DIM+1; i++) {
        for (int j = 0; j < DIM+1; j++) {
            old[i][j] = ocean[i][j];
        }
    }
  
    for (int ite = 0; ite < iterations; ite++) {

        for (int x = 0; x < DIM; x++) {
            for (int y = 0; y < DIM; y++) {

               if (ocean[x+1][y] >= 4) {
                    old[x][y] += 1;
                    changed[x+1][y] = 1;
               } 
               if (ocean[x-1][y] >= 4) {
                    old[x][y] += 1;
                    changed[x-1][y] = 1;
               }
               if (ocean[x][y+1] >= 4) {
                    old[x][y] += 1;
                    changed[x][y+1] = 1;
               }

               if (ocean[x][y-1] >= 4) {
                    old[x][y] += 1;
                    changed[x][y-1] = 1;
               }
               if (ocean[x][y] >= 4)
                    old[x][y] -= 4;
            }
        }
    }
    
   for (int i = 0; i < DIM+1; i++) {
        for (int j = 0; j < DIM+1; j++) {
            ocean[i][j] = old[i][j];
        }
   }
    colorize();
    return couleurs;
}

/*
 * Parallelisation simple de la première version sequentielle
 */
float *naive_para(unsigned iterations) {
   
    static int step = 0;
    reset_change();
    for (unsigned i = 0; i < iterations; i++)
        {
            step++;

        #pragma omp parallel for schedule(dynamic,1)

                for (int x = 0; x < DIM; x++)
                {
                    for (int y = 0; y < DIM; y++)
                    {
                        if (ocean[x][y] >= 4)
                        {
                            ocean[x+1][y] += 1;		  
                            ocean[x-1][y] += 1;
                            ocean[x][y+1] += 1;
                            ocean[x][y-1] += 1;

                            ocean[x][y] -= 4;
                            changed[x][y] = 1;
                        }
                    }
            }
           
        }
   colorize();
   return couleurs;
 
}



/*
 * Tache OpenMP:
 * x,y : Coordonnées de la cases qui concerne la tache.
 * p : nombre d'iteration
 *
 * La tache va calculer l'éboulement des grains dans un rayon p, pour connaitre l'état du grain en x,y au bout de p iteration.
 */
void create_sand_task(int x, int y, int p) {
    #pragma omp task  shared(ocean,old)
    {
        unsigned mine[DIM+1][DIM+1];
        int minx = x - p;
        int maxx = x + p;
        int miny = y -p;
        int maxy = y + p;

        if (minx < 0)
            minx = 0;
        if (maxx > DIM);
            maxx = DIM;
        

        if (miny < 0)
            miny = 0;
        if (maxy > DIM);
            maxy = DIM;
        
        for (int i = 0; i < DIM+1; i++) {
            for (int j = 0; j < DIM+1; j++) {
                mine[i][j] = ocean[i][j];
            }
        }
        
        for (int i = minx; i < maxx; i++) {
            for (int j = miny; j < maxy; j++) {
                if (mine[i][j] >= 4) {
                    mine[i+1][j] += 1;		  
                    mine[i-1][j] += 1;
                    mine[i][j+1] += 1;
                    mine[i][j-1] += 1;
                        
                    mine[i][j] -= 4;
                    changed[i][j] = 1;
                }
            }
        }

        old[x][y] = mine[x][y];
    }
}

float *para_task(unsigned iterations) {
   
    static int step = 0;
    reset_change();

    for (int i = 0; i < DIM+1; i++) {
        for (int j = 0; j < DIM+1; j++) {
            old[i][j] = ocean[i][j];
        }
    }
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            for (int x = 0; x < DIM; x++) {
                for (int y = 0; y < DIM; y++) {
                    create_sand_task(x,y,iterations);
                }
            }
        }
     
    
        #pragma omp taskwait
        for (int i = 0; i < DIM+1; i++) {
            for (int j = 0; j < DIM+1; j++) {
                ocean[i][j] = old[i][j];
            }
        }

    }

    colorize();
    return couleurs;

}



/*
 * options disponibles:
 *   Configuation initiale :
 *      -s : 1 source
 *      -h : homogène (toutes les cases on 5 grains)
 *   Fonctions de calculs :
 *      -f n ou n est le numéro de la fonctions :
 *         0 : Sequentiel 1
 *         1 : Sequentiel 2
 *         2 : Parallelisation "naive"
 *         3 : Paralellisation avec tâches
 */
int main (int argc, char **argv)
{
    typedef float* (*compute_function) (unsigned);
    compute_function func[4];
    func[0] = sequentiel_1;
    func[1] = sequentiel_2;
    func[2] = naive_para;
    func[3] = para_task;
    
    int value = NULL;
    int config = 0;
    int displayFlag = 1;
    int c;
    
    while ((c = getopt(argc, argv, "xf:hs:")) != -1) {
        switch (c) {
        case 's':
            config = atoi(optarg);
            break;
        case 'h':
            config = 3 ;
            break;

        case 'f':
            value = atoi(optarg);
            break;
        case 'x':
            displayFlag = 0;
            break;
        default:
            value = 0;
            return 0;
            
        }

    }

    sable_init(config);
    if (displayFlag) {
        display_init (argc, argv,
          DIM,              // dimension ( = x = y) du tas
          MAX_HEIGHT,       // hauteur maximale du tas
          get,              // callback func
         func[value]);         // callback func
    }
    else {
        volatile computeTime;
        while (1) {
            struct timeval t1,t2;
            gettimeofday (&t1,NULL);
            func[value](1);
            gettimeofday (&t2,NULL);
            computeTime = TIME_DIFF(t1,t2);
            printf("1 Iteration : %d (ms)\n", computeTime);
        }
    }


    return 0;
}
