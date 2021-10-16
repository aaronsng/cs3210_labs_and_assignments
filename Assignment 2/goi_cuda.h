#ifndef GOI_CUDA_H
#define GOI_CUDA_H

int goi(int nThreads, int nGenerations, const int *startWorld, int nRows, int nCols, int nInvasions, const int *invasionTimes, int **invasionPlans, dim3 gridDim_, dim3 blockDim_);

#endif
