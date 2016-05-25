
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <omp.h>
//////////////////////////////////////////////////////////////////////////
// Tas de sable "fake" (juste pour tester)

#define DIM 128 
#define MAX_HEIGHT 99999
#include <math.h>


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

// Tas de sable initial
static void sable_init (int config)
{
  
    if (config == 1) { 
        for (int y = 0; y < DIM; y++) {
            for (int x = 0; x < DIM; x++) {
                ocean[y][x] = 5;
            }
        }
    }
    else if (config == 0) {
        ocean[DIM/2][DIM/2] = MAX_HEIGHT;
    }
    else {
        ocean[DIM/2][DIM/2] = MAX_HEIGHT;
        ocean[DIM/4][(DIM/4)*3] = MAX_HEIGHT;
        ocean[DIM/4][DIM/4] = MAX_HEIGHT;
    }
}

// callback
float *compute (unsigned iterations)
{
  static int step = 0;
   reset_change();
  for (unsigned i = 0; i < iterations; i++)
    {
        step++;
        for (int x = 1; x < DIM; x++)
	    {
	        for (int y = 1; y < DIM; y++)
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
//  return DYNAMIC_COLORING; // altitude-based coloring
    colorize();
    return couleurs;
}

float *compute2(unsigned iterations) {
    reset_change();

   for (int i = 1; i < DIM; i++) {
        for (int j = 1; j < DIM; j++) {
            old[i][j] = ocean[i][j];
        }
    }
  
    for (int ite = 0; ite < iterations; ite++) {

        for (int x = 1; x < DIM; x++) {
            for (int y = 1; y < DIM; y++) {

                if (ocean[x+1][y] >= 4) {
                    old[x][y] += 1;
                  //  old[x+1][y] -= 1;
                    changed[x+1][y] = 1;
                } 
                if (ocean[x-1][y] >= 4) {
                    old[x][y] += 1;
                  //  old[x-1][y] -= 1;
                    changed[x-1][y] = 1;
                }
                 if (ocean[x][y+1] >= 4) {
                    old[x][y] += 1;
                //    old[x][y+1] -= 1;
                    changed[x][y+1] = 1;
                }

               if (ocean[x][y-1] >= 4) {
                    old[x][y] += 1;
                  //  old[x][y-1] -= 1;
                    changed[x][y-1] = 1;
                }
                if (ocean[x][y] >= 4)
                    old[x][y] -= 4;
            }
        }
    }
    
   for (int i = 1; i < DIM; i++) {
        for (int j = 1; j < DIM; j++) {
            ocean[i][j] = old[i][j];
        }
    }
    colorize();
    return couleurs;
}

float *naive_para(unsigned iterations) {
   
    static int step = 0;
    reset_change();
    for (unsigned i = 0; i < iterations; i++)
        {
            step++;
            #pragma omp parallel
            #pragma omp for
            for (int x = 1; x < DIM; x++)
                {
                    for (int y = 1; y < DIM; y++)
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
            #pragma omp barrier
        }

   colorize();
   return couleurs;
 
}
        
int main (int argc, char **argv)
{
    float *computesFunctions[4];
    computesFunctions[0] = compute;
    computesFunctions[1] = compute2;
    computesFunctions[2] = naive_para;
    
    char* value = NULL;
    int config = 0;
    int c;
    
    while ((c = getopt(argc, argv, "hs")) != -1) {
        switch (c) {
        case 's':
            config = 0;
            break;
        case 'h':
            config = 1;
            break;
        default:
            return 0;
            
        }

    }
    
    
    sable_init(config);
    display_init (argc, argv,
          DIM,              // dimension ( = x = y) du tas
          MAX_HEIGHT,       // hauteur maximale du tas
          get,              // callback func
          computesFunctions[0]);         // callback func

    return 0;
}
