## CS3210 Parallel Computing

Done by Aaron Sng of NTU REP (EEE)

## Feedback and Grades for Assignments / Labs
#### Assignment 1:
10.5 / 12

9.5 = 3 (out of 3, testing pthreads) + 1 (out of 3, testing OpenMP) + 1 (out of 1, testcases) + 1.5 (out of 2, implementation explanations in the report & code quality) + 2 (out of 4, measurement and analysis in report) + 1 (out of 1, non-trivial details in report)
(out of 1, bonus for performance analysis, shown in gradebook Bonus)
0 (out of 2, bonus for best speedup, shown in gradebook Bonus)
0 (late penalty 5%/day: days). Comments:
Runtime pthreads, marks: 3
TC1 Xeon (correctness: TRUE): 24.1466667 ms (speedup: 6.06501933)
TC1 i7 (correctness: TRUE): 600 ms (speedup: 0.173505556)
TC2 Xeon (correctness: TRUE): 5.04666667 ms
TC2 i7 (correctness: TRUE): 600 ms
TC3 Xeon (correctness: TRUE): 3.25333333 ms
TC3 i7 (correctness: TRUE): 600 ms
Remarks threads implementation: Encountered TLE when running pthreads implementation with 8 threads on Core i7 server
Runtime Openmp, marks: 1
TC1 Xeon (correctness: TRUE): 22.7733333 ms (speedup: 6.43076698)
TC1 i7 (correctness: TRUE): 26.89 ms (speedup: 3.871451593)
TC2 Xeon (correctness: TRUE): 4.73 ms
TC2 i7 (correctness: TRUE): 5.63 ms
TC3 Xeon (correctness: FALSE): 3.06 ms
TC3 i7 (correctness: FALSE): 3.496666667 ms
Remarks OpenMP implementation:
Report comments:
Conscientious report, but could have provided a more detailed description of your OMP and Pthreads implementations.
Good design considerations
Good that you covered hyperthreading as well
Good number of measurements taken.

Updated 31 Oct:
+1 for implementation

Mean: 10.31, Median: 11, 25th - 75th Percentile: 10 - 11.5

#### Lab 1:
2 / 2

Implementations are ok, be careful when you implement a infinite looping program - the only way to terminate it is via sending signal and doing so will skip all the cleanup you do at the end of the program. You can look into using signal catcher for this.
Good explanation for ex9 as well. Maybe you can try increasing the limits to see whether the same observation holds?
