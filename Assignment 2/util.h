#ifndef UTIL_H
#define UTIL_H

int getValueAtHost(const int *grid, int nRows, int nCols, int row, int col);
void setValueAtHost(int *grid, int nRows, int nCols, int row, int col, int val);
void printWorld(const int *world, int nRows, int nCols);

/* MODIFICATION STARTS HERE */
typedef struct World_Deets {
    int * world;
    int * invaders;
    const int * og_world;
    int * death_toll;
    int rowWidth;
} WorldVector;
/* MODIFICATION ENDS HERE */

#endif
