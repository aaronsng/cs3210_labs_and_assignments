#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "util.h"
#include "exporter.h"
#include "settings.h"

// including the "dead faction": 0
#define MAX_FACTIONS 10

// this macro is here to make the code slightly more readable, not because it can be safely changed to
// any integer value; changing this to a non-zero value may break the code
#define DEAD_FACTION 0

// global variables made known to the rest of the program and of little security concern
int nRows_, nCols_, rowWidth_, nGenerations_, nInvasions_, nThreads_, currentGeneration_;
int * world_;
int ** invasionPlans_;
const int * invasionTimes_;
bool proceed = false;

bool * divided_completed_;
int * divided_thread_completed_;
int * divided_death_toll_;

int * rowStartCollection_;
int * rowEndCollection_;

// death toll due to fighting
int deathToll = 0;

/**
 * Specifies the number(s) of live neighbors of the same faction required for a dead cell to become alive.
 */
bool isBirthable(int n)
{
    return n == 3;
}

/**
 * Specifies the number(s) of live neighbors of the same faction required for a live cell to remain alive.
 */
bool isSurvivable(int n)
{
    return n == 2 || n == 3;
}

/**
 * Specifies the number of live neighbors of a different faction required for a live cell to die due to fighting.
 */
bool willFight(int n) {
    return n > 0;
}

/**
 * Computes and returns the next state of the cell specified by row and col based on currWorld and invaders. Sets *diedDueToFighting to
 * true if this cell should count towards the death toll due to fighting.
 * 
 * invaders can be NULL if there are no invaders.
 */
int getNextState(const int *currWorld, const int *invaders, int nRows, int nCols, int row, int col, bool *diedDueToFighting)
{
    // we'll explicitly set if it was death due to fighting
    *diedDueToFighting = false;

    // faction of this cell
    int cellFaction = getValueAt(currWorld, nRows, nCols, row, col);

    // did someone just get landed on?
    if (invaders != NULL && getValueAt(invaders, nRows, nCols, row, col) != DEAD_FACTION)
    {
        *diedDueToFighting = cellFaction != DEAD_FACTION;
        return getValueAt(invaders, nRows, nCols, row, col);
    }

    // tracks count of each faction adjacent to this cell
    int neighborCounts[MAX_FACTIONS];
    memset(neighborCounts, 0, MAX_FACTIONS * sizeof(int));

    // count neighbors (and self)
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int faction = getValueAt(currWorld, nRows, nCols, row + dy, col + dx);
            if (faction >= DEAD_FACTION)
            {
                neighborCounts[faction]++;
            }
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

/* NOTE: Modification made here */
bool doneOrNot(int numThreads, bool * arr) 
{
    for (int i = 0; i < numThreads; i++) 
    {
        if (!arr[i])
            return false;
    }
    return true;
}


bool generationSet(int numThreads, int * arr) 
{
    for (int i = 0; i < numThreads; i++) 
    {
        if (arr[i] != currentGeneration_ + 1)
            return false;
    }
    return true;
}

void clearDone(int numThreads, bool * arr) 
{
    for (int i = 0; i < numThreads; i++) 
    {
        arr[i] = false;
    }
}

void printArr(int numThreads, int * arr) 
{
    printf("Array: ");
    for (int i = 0; i < numThreads; i++) 
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void * worker(void* tid) 
{
    long t_id = (long) tid;

    // Begin simulating
    int invasionIndex = 0;
    int currentGeneration = 0;

    int rowStart = rowStartCollection_[t_id];
    int rowEnd = rowEndCollection_[t_id];
    printf("Thread %ld, rowStart: %d, rowEnd: %d\n", t_id, rowStart, rowEnd);
    for (currentGeneration = 0; currentGeneration < nGenerations_; currentGeneration++)
    {           
        // is there an invasion this generation?
        int *inv = NULL;

        if (invasionIndex < nInvasions_ && (currentGeneration + 1) == invasionTimes_[invasionIndex])
        {
            // we make a copy because we do not own invasionPlans
            inv = malloc(sizeof(int) * nRows_ * nCols_);
            if (inv == NULL)
            {
                free(world_);
                return -1;
            }
            
            // copy invasion plans into a new world
            for (int row = rowStart; row < rowEnd; row++)
            {
                for (int col = 0; col < nCols_; col++)
                {
                    setValueAt(inv, rowEnd, nCols_, row, col, getValueAt(invasionPlans_[invasionIndex], rowEnd, nCols_, row, col));
                }
            }
            invasionIndex++;
        }

        // create the next world state
        int *wholeNewWorld = malloc(sizeof(int) * nRows_ * nCols_);
        if (wholeNewWorld == NULL)
        {
            if (inv != NULL)
            {
                free(inv);
            }
            free(world_);
            return -1;
        }

        // get new states for each cell
        for (int row = rowStart; row < rowEnd; row++)
        {
            for (int col = 0; col < nCols_; col++)
            {
                bool diedDueToFighting;
                int nextState = getNextState(world_, inv, nRows_, nCols_, row, col, &diedDueToFighting); 
                setValueAt(wholeNewWorld, rowEnd, nCols_, row, col, nextState);

                if (diedDueToFighting)
                {
                    divided_death_toll_[t_id] = divided_death_toll_[t_id] + 1;
                }
            }
        }

        if (inv != NULL)
        {
            free(inv);
        }

        // synchronisation construct here. the worker thread 'sends' the result to the master thread 
        divided_thread_completed_[t_id] = currentGeneration + 1;
        while (!generationSet(nThreads_, divided_thread_completed_));

        // update world, no need for mutex
        for (int row = rowStart; row < rowEnd; row++)
        {
            for (int col = 0; col < nCols_; col++)
            {
                setValueAt(world_, rowEnd, nCols_, row, col, getValueAt(wholeNewWorld, rowEnd, nCols_, row, col));
            }
        }
        free(wholeNewWorld);

        // updates that have finished collating and sends the result to the master
        divided_completed_[t_id] = true;
        while(currentGeneration_ != divided_thread_completed_[t_id]);
    }

    pthread_exit(NULL);
}
/* NOTE: Modification ends here */

/**
 * The main simulation logic.
 * 
 * goi does not own startWorld, invasionTimes or invasionPlans and should not modify or attempt to free them.
 * nThreads is the number of threads to simulate with. It is ignored by the sequential implementation.
 */
int goi(int nThreads, int nGenerations, const int *startWorld, int nRows, int nCols, int nInvasions, const int *invasionTimes, int **invasionPlans)
{
    /* NOTE: Modification made here */
    pthread_t world_copiers[nThreads];
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    int tid;

    divided_completed_ = malloc(sizeof(bool) * nThreads);
    for (tid = 0; tid < nThreads; tid++) {
        divided_completed_[tid] = false;
    }

    divided_thread_completed_ = malloc(sizeof(int) * nThreads);
    for (tid = 0; tid < nThreads; tid++) {
        divided_thread_completed_[tid] = false;
    }

    divided_death_toll_ = malloc(sizeof(int) * nThreads);
    for (tid = 0; tid < nThreads; tid++) {
    	divided_death_toll_[tid] = 0;
    }

    // Set global variables
    nRows_ = nRows;
    nCols_ = nCols;
    nThreads_ = nThreads;
    nGenerations_ = nGenerations;
    nInvasions_ = nInvasions;
    invasionPlans_ = invasionPlans;
    invasionTimes_ = invasionTimes;
    currentGeneration_ = 0;
    /* NOTE: Modification ends here */

    // init the world!
    // we make a copy because we do not own startWorld (and will perform free() on world)
    world_ = malloc(sizeof(int) * nRows * nCols);
    if (world_ == NULL)
    {
        return -1;
    }
    
    for (int row = 0; row < nRows; row++)
    {
        for (int col = 0; col < nCols; col++)
        {
            setValueAt(world_, nRows, nCols, row, col, getValueAt(startWorld, nRows, nCols, row, col));
        }
    }

    /* NOTE: Modification made here */
    // Parallelisable segment here, will adopt master-worker structure
    // This feature of copying one world over to another is common across the assignment. 
    // In this assignment, I'll decompose the problem into smaller segments to favour the 
    // row-order major hence increase spatial locality
    // Numerically subdivide the tasks across the threads initiated by the program. 

    /* --------- PARALLEL PROGRAM IMPLEMENTATION --------- */
    // I've decided to proceed with the master-worker parallel programming paradigm.
    // Here the main program will be the main thread, where the others will be the worker threads.
    // The row-wise distribution of data is a pseudo implementation of data distribution. Hence,
    // the synchronisation construct of receiving result and printing matrix is only required to be 
    // implemented here.

    // Find the ceiling of the row width. This is to ensure that the whole matrix is covered by 
    // parallel program when divided.
    rowStartCollection_ = malloc(sizeof(int) * nThreads);
    rowEndCollection_ = malloc(sizeof(int) * nThreads);
  
    int baseRowWidth = nRows / nThreads;
    int remnant = nRows % nThreads;
    int startIndex, endIndex = 0;
    for (int i = 0; i < nThreads; i++)
    {
        startIndex = endIndex;
        if (i < (nThreads - remnant))
        {
            endIndex = (startIndex + baseRowWidth) > nRows ? nRows : (startIndex + baseRowWidth);
        }
        else
        {
            endIndex = (startIndex + baseRowWidth + 1) > nRows ? nRows : (startIndex + baseRowWidth + 1);
        }

        rowStartCollection_[i] = startIndex;
        rowEndCollection_[i] = endIndex;
    }

    for (tid = 0; tid < nThreads; tid++) 
    {
        int rc = pthread_create(&world_copiers[tid], NULL, worker, (void*) tid);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    /* NOTE: Modification ends here */

#if PRINT_GENERATIONS
    printf("\n=== WORLD 0 ===\n");
    printWorld(world_, nRows, nCols);
#endif

#if EXPORT_GENERATIONS
    exportWorld(world_, nRows, nCols);
#endif

    /* MODIFICATION MADE HERE */
    //nGenerations_ = 1;
    while (currentGeneration_ < nGenerations_) {
        if (generationSet(nThreads, divided_thread_completed_)) 
        {
            // Collate data from all the threads
            while (!doneOrNot(nThreads, divided_completed_));

#if PRINT_GENERATIONS
            printf("\n=== WORLD %d ===\n", (currentGeneration_ + 1));
            printWorld(world_, nRows, nCols);
#endif

#if EXPORT_GENERATIONS
            exportWorld(world_, nRows, nCols);
#endif
            clearDone(nThreads_, divided_completed_);
            currentGeneration_++;            
        }
    }

    // Await termination
    for (tid = 0; tid < nThreads; tid++) 
        pthread_join(world_copiers[tid], NULL);

		for (tid = 0; tid < nThreads; tid++) 
				deathToll += divided_death_toll_[tid];

#if PRINT_GENERATIONS
    printf("\n=== WORLD Final ===\n");
    printWorld(world_, nRows, nCols);
#endif
    /* MODIFICATION ENDS HERE*/

    free(world_);

    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds * 1e-9;
    printf("Time taken %.2f\n", elapsed);

    return deathToll;
}
