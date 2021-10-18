#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include "util.h"
#include "exporter.h"
#include "settings.h"

// including the "dead faction": 0
#define MAX_FACTIONS 10

// this macro is here to make the code slightly more readable, not because it can be safely changed to
// any integer value; changing this to a non-zero value may break the code
#define DEAD_FACTION 0

/*
 * Gets the value at. GPU implementation
 */
__device__ int getValueAtDevice(const int * grid, int nRows, int nCols, int row, int col) 
{
    if (row < 0 || row >= nRows || col < 0 || col >= nCols) 
    {
        return -1; 
    }
    return *(grid + (row * nCols) + col);
}

/*
 * Sets the value at. GPU implementation
 */
__device__ void setValueAtDevice(int * grid, int nRows, int nCols, int row, int col, int val) 
{
    if (row < 0 || row >= nRows || col < 0 || col >= nCols) 
    {
        return; 
    }
    *(grid + (row * nCols) + col) = val;
}

/*
 * Specifies the number(s) of live neighbors of the same faction required for a dead cell to become alive.
 */
__device__ bool isBirthable(int n)
{
    return n == 3;
}

/**
 * Specifies the number(s) of live neighbors of the same faction required for a live cell to remain alive.
 */
__device__ bool isSurvivable(int n)
{
    return n == 2 || n == 3;
}

/**
 * Specifies the number of live neighbors of a different faction required for a live cell to die due to fighting.
 */
__device__ bool willFight(int n) {
    return n > 0;
}

/**
 * Computes and returns the next state of the cell specified by row and col based on currWorld and invaders. Sets *diedDueToFighting to
 * true if this cell should count towards the death toll due to fighting.
 * currWorld is a variable in the global memory.
 * invaders can be NULL if there are no invaders.
 */
__device__ int getNextState(const int *currWorld, const int *invaders, int nRows, int nCols, int row, int col, bool *diedDueToFighting)
{
    // we'll explicitly set if it was death due to fighting
    *diedDueToFighting = false;

    // faction of this cell
    int cellFaction = getValueAtDevice(currWorld, nRows, nCols, row, col);

    // did someone just get landed on?
    if (invaders != NULL && getValueAtDevice(invaders, nRows, nCols, row, col) != DEAD_FACTION)
    {
        *diedDueToFighting = cellFaction != DEAD_FACTION;
        return getValueAtDevice(invaders, nRows, nCols, row, col);
    }

    // tracks count of each faction adjacent to this cell
    int neighborCounts[MAX_FACTIONS];
    memset(neighborCounts, 0, MAX_FACTIONS * sizeof(int));

    // count neighbors (and self)
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int faction = getValueAtDevice(currWorld, nRows, nCols, row + dy, col + dx);
            bool faction_is_dead = faction >= DEAD_FACTION;
            neighborCounts[faction] += faction_is_dead;
        }
    }

    // we counted this cell as its "neighbor"; adjust for this
    neighborCounts[cellFaction]--;

    if (cellFaction == DEAD_FACTION)
    {
        // this is a dead cell; we need to see if a birth is possible:
        // need exactly 3 of a single faction; we don't care about other factions

        // by default, no birth
        int newFaction = DEAD_FACTION;

        // start at 1 because we ignore dead neighbors
        for (int faction = DEAD_FACTION + 1; faction < MAX_FACTIONS; faction++)
        {
            int count = neighborCounts[faction];
            if (isBirthable(count))
            {
                newFaction = faction;
            }
        }

        return newFaction;
    }
    else
    {
        /** 
         * this is a live cell; we follow the usual rules:
         * Death (fighting): > 0 hostile neighbor
         * Death (underpopulation): < 2 friendly neighbors and 0 hostile neighbors
         * Death (overpopulation): > 3 friendly neighbors and 0 hostile neighbors
         * Survival: 2 or 3 friendly neighbors and 0 hostile neighbors
         */

        int hostileCount = 0;
        for (int faction = DEAD_FACTION + 1; faction < MAX_FACTIONS; faction++)
        {
            if (faction == cellFaction)
            {
                continue;
            }
            hostileCount += neighborCounts[faction];
        }

        if (willFight(hostileCount))
        {
            *diedDueToFighting = true;
            return DEAD_FACTION;
        }

        int friendlyCount = neighborCounts[cellFaction];
        if (!isSurvivable(friendlyCount))
        {
            return DEAD_FACTION;
        }

        return cellFaction;
    }
}

/** 
 * Main kernel code: 
 * Breakup the code to the element wise. The kernel will operate element wise operations. 
 * currentGeneration -> Located in global memory
 * invasionPlans -> Located in global memory
 * invasionTimes -> Located in global memory
 */
__global__ void simulate_per_thread(int * world, int * wholeNewWorld, int * invasionPlans, int * nRows, int * nCols, int * row_start_collection, int * row_end_collection, int * col_start_collection, int * col_end_collection, int * death_toll_collection) 
{
    // localised death toll, to minimise branching
    int deathToll_ = 0;

    // blockIndex will divide the problem as in Assignment 1. The general strategy in Assignment 1 were
    // coarsely grained tasks performed over the row
    int blockIndex = blockIdx.x * gridDim.y * gridDim.z + blockIdx.y * gridDim.z + blockIdx.z;
    int rowStart = row_start_collection[blockIndex];
    int rowEnd = row_end_collection[blockIndex];

    // threadIndex will divide the problem further. Now instead of performing over each individual row
    // GPU programming calls for greater granularity, and hence each individual row maybe too big a task
    // Hence, each individual GPU thread will reduce the task further by chopping up the problem with column wise distribution.
    int threadIndex = threadIdx.x * blockDim.y * blockDim.z + threadIdx.y * blockDim.z + threadIdx.z;
    int colStart = col_start_collection[threadIndex];
    int colEnd = col_end_collection[threadIndex];

    //printf("\n%d %d\n", blockIndex, threadIndex);
    //printf("\n%d %d %d %d %d %d\n", rowStart, rowEnd, colStart, colEnd, blockIndex, threadIndex);

    // check if there is an invasion invoked in this kernel call
    for (int row = rowStart; row < rowEnd; row++) 
    {
        for (int col = colStart; col < colEnd; col++)
        {
            bool diedDueToFighting;
            int nextState = getNextState(world, invasionPlans, *nRows, *nCols, row, col, &diedDueToFighting);
            setValueAtDevice(wholeNewWorld, *nRows, *nCols, row, col, nextState);
            
            // This is possible as booleans have a value of 1. reduce branching within kernel as well
            deathToll_ += diedDueToFighting; 
        }
    }
    death_toll_collection[blockIndex * blockDim.x * blockDim.y * blockDim.z + threadIndex] += deathToll_;
}

/**
 * Check CUDA errors
 */
void check_cuda_errors()
{
    cudaError_t rc;
    rc = cudaGetLastError();
    if (rc != cudaSuccess) 
    {
        printf("Last CUDA error: %s\n", cudaGetErrorString(rc));
    }
}

/**
 * The main simulation logic.
 * 
 * goi does not own startWorld, invasionTimes or invasionPlans and should not modify or attempt to free them.
 * nThreads is the number of threads to simulate with. It is ignored by the sequential implementation.
 */
int goi(int nThreads, int nGenerations, const int *startWorld, int nRows, int nCols, int nInvasions, const int *invasionTimes, int **invasionPlans, dim3 gridDim_, dim3 blockDim_)
{ 
    // calculate wall clock
    // struct timespec begin, end;
    // clock_gettime(CLOCK_REALTIME, &begin);

    // send nRows over
    int * d_n_rows;
    cudaMalloc((void**) &d_n_rows, sizeof(int));
    cudaMemcpy(d_n_rows, &nRows, sizeof(int), cudaMemcpyHostToDevice);

    int * d_n_cols;
    cudaMalloc((void**) &d_n_cols, sizeof(int));
    cudaMemcpy(d_n_cols, &nCols, sizeof(int), cudaMemcpyHostToDevice);

    // initialise the world on the device
    int * d_world;
    cudaMalloc((void**) &d_world, nRows * nCols * sizeof(int));
    cudaMemcpy(d_world, startWorld, sizeof(int) * nRows * nCols, cudaMemcpyHostToDevice);

    // start computing the start and end indices of the various rows
    int * d_row_start_collection = NULL;
    int * d_row_end_collection = NULL;
    int blockCount = gridDim_.x * gridDim_.y * gridDim_.z;
    int * row_start_collection;
    int * row_end_collection;
    int baseRowWidth = nRows / blockCount;
    int remnant = nRows % blockCount;
    int startIndex, endIndex = 0;
    cudaMallocHost((void **) &row_start_collection, sizeof(int) * blockCount);
    cudaMallocHost((void **) &row_end_collection, sizeof(int) * blockCount);
    
    for (int i = 0; i < blockCount; i++) 
    {
        startIndex = endIndex;
        if (i < (blockCount - remnant)) 
        {
            endIndex = (startIndex + baseRowWidth) > nRows ? nRows : (startIndex + baseRowWidth);
        }
        else 
        {
            endIndex = (startIndex + baseRowWidth + 1) > nRows ? nRows : (startIndex + baseRowWidth + 1);
        }

        row_start_collection[i] = startIndex;
        row_end_collection[i] = endIndex;
    }

    // malloc and copy over
    cudaMalloc((void**) &d_row_start_collection, blockCount * sizeof(int));
    cudaMemcpy(d_row_start_collection, row_start_collection, blockCount * sizeof(int), cudaMemcpyHostToDevice);
    cudaMalloc((void**) &d_row_end_collection, blockCount * sizeof(int));
    cudaMemcpy(d_row_end_collection, row_end_collection, blockCount * sizeof(int), cudaMemcpyHostToDevice);
    
    // start computing the start and end indices of the various columns
    int * d_col_start_collection = NULL;
    int * d_col_end_collection = NULL;
    int threadCount = blockDim_.x * blockDim_.y * blockDim_.z;
    int * col_start_collection;
    int * col_end_collection;
    int baseColWidth = nCols / threadCount;
    remnant = nCols % threadCount;
    endIndex = 0;
    cudaMallocHost((void **) &col_start_collection, sizeof(int) * threadCount);
    cudaMallocHost((void **) &col_end_collection, sizeof(int) * threadCount);
    
    for (int i = 0; i < threadCount; i++) 
    {
        startIndex = endIndex;
        if (i < (threadCount - remnant)) 
        {
            endIndex = (startIndex + baseColWidth) > nCols ? nCols : (startIndex + baseColWidth);
        }
        else 
        {
            endIndex = (startIndex + baseColWidth + 1) > nCols ? nCols : (startIndex + baseColWidth + 1);
        }

        col_start_collection[i] = startIndex;
        col_end_collection[i] = endIndex;
    }

    // malloc and copy over
    cudaMalloc((void**) &d_col_start_collection, threadCount * sizeof(int));
    cudaMemcpy(d_col_start_collection, col_start_collection, threadCount * sizeof(int), cudaMemcpyHostToDevice);
    cudaMalloc((void**) &d_col_end_collection, threadCount * sizeof(int));
    cudaMemcpy(d_col_end_collection, col_end_collection, threadCount * sizeof(int), cudaMemcpyHostToDevice);

    // create a death_toll_collection
    int * d_death_toll_collection;
    int * death_toll_collection;
    cudaMallocHost((void**) &death_toll_collection, sizeof(int) * threadCount * blockCount);
    cudaMalloc((void**) &d_death_toll_collection, sizeof(int) * threadCount * blockCount);

#if PRINT_GENERATIONS
    printf("\n=== WORLD 0 ===\n");
    printWorld(startWorld, nRows, nCols);
#endif

#if EXPORT_GENERATIONS
    exportWorld(startWorld, nRows, nCols);
#endif

    // Begin simulating
    int invasionIndex = 0;
    
    // initialise transition world
    int * d_temp_world;
    cudaMalloc((void**) &d_temp_world, sizeof(int) * nRows * nCols);
    
    for (int i = 1; i <= nGenerations; i++)
    {
        int * d_invasion = NULL;

#if PRINT_GENERATIONS || EXPORT_GENERATIONS
        int * h_received;
        cudaMallocHost((void**) & h_received, nRows * nCols * sizeof(int));
#endif

        // Check if there's an invasion this particular generation
        if (invasionIndex < nInvasions && i == invasionTimes[invasionIndex])
        {   
            cudaMalloc((void**) &d_invasion, sizeof(int) * nRows * nCols);
            cudaMemcpy(d_invasion, invasionPlans[invasionIndex], sizeof(int) * nRows * nCols, cudaMemcpyHostToDevice);
            invasionIndex++;
        }

        // initialise transition world
        int * d_temp_world;
        cudaMalloc((void**) &d_temp_world, sizeof(int) * nRows * nCols);
        
        // get new states for each cell
        simulate_per_thread<<<gridDim_, blockDim_>>>(d_world, d_temp_world, d_invasion, d_n_rows, d_n_cols, d_row_start_collection, d_row_end_collection, d_col_start_collection, d_col_end_collection, d_death_toll_collection);
        check_cuda_errors();
        //cudaDeviceSynchronize();
        
        // swap the worlds
        cudaFree(d_world);
        d_world = d_temp_world;

#if PRINT_GENERATIONS
        cudaMemcpy(h_received, d_world, nRows * nCols * sizeof(int), cudaMemcpyDeviceToHost);
        printf("\n=== WORLD %d ===\n", i);
        printWorld(h_received, nRows, nCols);
#endif

#if EXPORT_GENERATIONS
        cudaMemcpy(h_received, d_world, nRows * nCols * sizeof(int), cudaMemcpyDeviceToHost);
        exportWorld(h_received, nRows, nCols);
#endif

#if DEBUG
        printf("One generation passed\n");
#endif

        if (d_invasion != NULL)
        {
            cudaFree(d_invasion);
        }

#if PRINT_GENERATIONS || EXPORT_GENERATIONS
        if (h_received != NULL) 
        {
            cudaFreeHost(h_received);
        }
#endif
        
    }
    
    int deathToll = 0;
    
    cudaMemcpy(death_toll_collection, d_death_toll_collection, sizeof(int) * blockCount * threadCount, cudaMemcpyDeviceToHost);
    for (int i = 0; i < (blockCount * threadCount); i++) 
    {
        deathToll += death_toll_collection[i];
    }
    
    // clock_gettime(CLOCK_REALTIME, &end);
    // long seconds = end.tv_sec - begin.tv_sec;
    // long nanoseconds = end.tv_nsec - begin.tv_nsec;
    // double elapsed = seconds + nanoseconds * 1e-9;

    // printf("Time taken %.2f\n", elapsed);

    // free arrays on device
    cudaFree(d_world); 
    cudaFree(d_n_rows);
    cudaFree(d_n_cols);
    cudaFree(d_col_start_collection);
    cudaFree(d_col_end_collection);
    cudaFree(d_row_start_collection);
    cudaFree(d_row_end_collection);
    cudaFree(d_death_toll_collection);

    // free arrays on host
    cudaFreeHost(col_end_collection);
    cudaFreeHost(col_start_collection);
    cudaFreeHost(row_start_collection);
    cudaFreeHost(row_end_collection);
    cudaFreeHost(death_toll_collection);
    
    return deathToll;
}
