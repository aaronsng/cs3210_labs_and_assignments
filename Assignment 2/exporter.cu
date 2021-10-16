/**
 * NOTE TO STUDENTS: you should not need to modify this file.
 * You can if you wish to, but we will NOT consider it when grading, even if you mention it in your report.
 * 
 * This file is simply an exporter library we wrote to let you export your simulation states to the GOI visualizer.
 * Usage:
 *  1) Call initWorldExporter once with an open file with write permissions.
 *  2) Call exportWorld whenever you wish to write a world state to the file specified in step 1.
 */

#include <stdlib.h>
#include "exporter.h"
#include "sb/sb.h"

#define JSON_KEY "\"world\""

FILE *exportFile = NULL;

/**
 * Initializes the world exporter with the input file.
 * 
 * If input file is NULL or initWorldExporter has not been called, then calls to exportWorld
 * will do nothing.
 * 
 * Otherwise, subsequent calls to exportWorld will export to the input file.
 */
void initWorldExporter(FILE *file) {
    exportFile = file;
}

/**
 * returns the value at the input row and col of the input grid, if valid.
 * 
 * -1 is returned if row or col is out of bounds (as specified by nRows and nCols).
 */
int exporterGetValueAt(const int *grid, int nRows, int nCols, int row, int col)
{
    if (row < 0 || row >= nRows || col < 0 || col >= nCols)
    {
        return -1;
    }

    return *(grid + (row * nCols) + col);
}

/**
 * Exports the input world.
 * 
 * Requires that initWorldExporter be called prior with a valid file.
 */
void exportWorld(const int *world, int nRows, int nCols)
{
    if (exportFile == NULL)
    {
        return;
    }

    StringBuilder *sb = sb_create();
    if (sb == NULL)
    {
        fprintf(stderr, "Error: out of memory!\n");
        return;
    }

    sb_append(sb, "{");
    sb_append(sb, JSON_KEY);
    sb_append(sb, ":[");
    for (int row = 0; row < nRows; row++)
    {
        sb_append(sb, "[");
        for (int col = 0; col < nCols; col++)
        {
            sb_appendf(sb, "%d", exporterGetValueAt(world, nRows, nCols, row, col));
            if (col != nCols - 1) {
                sb_append(sb, ",");
            }
        }
        sb_append(sb, "]");
        if (row != nRows - 1) {
            sb_append(sb, ",");
        }
    }
    sb_append(sb, "]}\n");

    char *s = sb_concat(sb);
    if (s == NULL)
    {
        fprintf(stderr, "Error: out of memory!\n");
        sb_free(sb);
        return;
    }

    if (fputs(s, exportFile) == EOF)
    {
        fprintf(stderr, "Error: cannot export to file.\n");
        free(s);
        sb_free(sb);
        return;
    }

    free(s);
    sb_free(sb);
}