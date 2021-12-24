## CS3210 Parallel Computing

Done by Aaron Sng under SUSEP

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

#### Assignment 2:
10 / 10

10 = 5 (out of 5, CUDA implementation + submission archive) + 1 (out of 1, testcases) + 2 (out of 2, implementation explanations in the report) + 2 (out of 2, measurement and analysis in report)
0 (out of 2, data/task distribution analysis, shown in gradebook Bonus) 
0 (out of 1, bonus for outstanding analysis/implementation, shown in gradebook Bonus)
0 (out of 2, bonus for best speedup, shown in gradebook Bonus)
10 (late penalty 5%/day: 0 days). 

Comments: 
Only used sample test cases (1)
Correct implementation with good explanation and justification of design decisions and effort made to improve performance (2)
Fair number of measurements taken, varying 2 factors (113 = 4 (out of 4, testing correctnesss) + 2 (out of 2, testing speedup) + 1 (out of 1, testcases) + 2 (out of 2, implementation explanations in the report & code quality) + 3 (out of 3, measurement and analysis in report) + 1 (out of 1, non-trivial details in report) 
1 (out of 1, bonus for performance analysis, shown in gradebook Bonus)
1 (out of 2, bonus for mapreduce comparison, shown in gradebook Bonus)
0 (late penalty 5%/day: 0 days). 
Testcases:
TC0 (correctness: TRUE): 51.4199999999999 s
TC1 (correctness: TRUE): 18.3466666666666 s
TC2 (correctness: TRUE): 18.34 s
TC3 (correctness: TRUE): 13.8233333333333 s
Report comments: 
Has custom test cases.

Design/implementation is clear. Good description of alternative designs/how the final design was arrived at.

Clear analysis of how deadlocks are avoided. 

Thorough measurements taken. Varied a good number of factors. Analysis of these was clear.

This was an excellent report! Several non-trivial points were brought up and explained. Well done.

Bonus: unfortunately, the first point is not accepted as it was already mentioned in the assignment brief. However, the remaining 4 points are correct.

Mean: 
Good analysis of the implementation and its performance, with explanations for each of the trends observed (1)

Implementation mark breakdown:
0.5 (out of 0.5, problem-free submission) + 3.5 (out of 3.5, correctness of test cases 1-7 (0.5 marks each)) + 1 (out of 1, performance and correctness of test case 8)

Mean: 8.7052, Median: 9.5, 25th - 75th Percentile: 8.5 - 10

#### Assignment 3:
13 / 13

13 = 4 (out of 4, testing correctnesss) + 2 (out of 2, testing speedup) + 1 (out of 1, testcases) + 2 (out of 2, implementation explanations in the report & code quality) + 3 (out of 3, measurement and analysis in report) + 1 (out of 1, non-trivial details in report) 
1 (out of 1, bonus for performance analysis, shown in gradebook Bonus)
1 (out of 2, bonus for mapreduce comparison, shown in gradebook Bonus)
0 (late penalty 5%/day: 0 days). 
Testcases:
TC0 (correctness: TRUE): 51.4199999999999 s
TC1 (correctness: TRUE): 18.3466666666666 s
TC2 (correctness: TRUE): 18.34 s
TC3 (correctness: TRUE): 13.8233333333333 s
Report comments: 
Has custom test cases.

Design/implementation is clear. Good description of alternative designs/how the final design was arrived at.

Clear analysis of how deadlocks are avoided. 

Thorough measurements taken. Varied a good number of factors. Analysis of these was clear.

This was an excellent report! Several non-trivial points were brought up and explained. Well done.

Bonus: unfortunately, the first point is not accepted as it was already mentioned in the assignment brief. However, the remaining 4 points are correct.

Mean: 11.6035, Median: 12, 25th - 75th Percentile: 11 - 13

#### Lab 1:
2 / 2

Implementations are ok, be careful when you implement a infinite looping program - the only way to terminate it is via sending signal and doing so will skip all the cleanup you do at the end of the program. You can look into using signal catcher for this.
Good explanation for ex9 as well. Maybe you can try increasing the limits to see whether the same observation holds?

#### Lab 2:
2 / 2

Great report overall. Not much to comment, good job.

#### Lab 4:
2 / 2

Great report with in-depth analysis and explanation. For ex6, you can also consider how different data distribution model affects memory/cache access.

#### Lab 6:
2 / 2

Good job with the benchmarking results and calculation for ex5. Good attempt in explaining your observations.
