
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static void sable_init ()
{
  unsigned dmax2 = MAX_HEIGHT; 
  
/* 
  for (int y = 0; y < DIM; y++)
    for (int x = 0; x < DIM; x++) {
      ocean[y][x] = 5;
    }
*/ 
//   ocean[0][1] = 4;
 ocean[DIM/2][DIM/2] = dmax2;
 //ocean[DIM/4][(DIM/4)*3] = dmax2;
 //ocean[DIM/4][DIM/4] = dmax2;
}

// callback
float *compute (unsigned iterations)
{
  static int step = 0;
   reset_change();
  for (unsigned i = 0; i < iterations; i++)
    {
        step++;
        for (int y = 0; y < DIM; y++)
	    {
	        for (int x = 0; x < DIM; x++)
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

   for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            old[i][j] = ocean[i][j];
        }
    }
  
    for (int ite = 0; ite < iterations; ite++) {
        for (int x = 0; x < DIM; x++) {
            for (int y = 0; y < DIM; y++) {

                if ( ocean[x+1][y] >= 4) {
                    old[x][y] += 1;
                    old[x+1][y] -= 1;
                    changed[x+1][y] = 1;
                } 
                if ( ocean[x-1][y] >= 4) {
                    old[x][y] += 1;
                    old[x-1][y] -= 1;
                    changed[x-1][y] = 1;
                }
                 if ( ocean[x][y+1] >= 4) {
                    old[x][y] += 1;
                    old[x][y+1] -= 1;
                    changed[x][y+1] = 1;
                }

               if ( ocean[x][y-1] >= 4) {
                    old[x][y] += 1;
                    old[x][y-1] -= 1;
                    changed[x][y-1] = 1;
                }

            }
        }
    }
    
   for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            ocean[i][j] = old[i][j];
        }
    }
    colorize();
    return couleurs;
}


int main (int argc, char **argv)
{
  sable_init ();
  display_init (argc, argv,
		DIM,              // dimension ( = x = y) du tas
		MAX_HEIGHT,       // hauteur maximale du tas
		get,              // callback func
        compute);         // callback func

  return 0;
}
