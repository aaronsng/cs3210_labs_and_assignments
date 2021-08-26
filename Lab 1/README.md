## Compilation Instructions
#### Exercise 7
cd to the directory and run the following command:
```bash
gcc -pthread -o ex7 ex7-prod-con-threads.c
```

In the same directory, run the following command to run the code.
```bash
./ex7 <BUFFER_SIZE> <PROP_DELAY>
```
Replace `BUFFER_SIZE` with the size of the buffer used in the producer consumer, replace `PROP_DELAY` with the timing you'd like to introduce in the loop. This is to allow you to run the code and verify the validity of implementation rather than scrolling through all the spam verbose. I recommend a value of 1.0.

#### Exercise 8
cd to the directory and run the following command:
```bash
gcc -pthread -o ex8 ex8-prod-con-processes.c
```

In the same directory, run the following command to run the code.
```bash
./ex8 <BUFFER_SIZE> <PROP_DELAY>
```
Replace `BUFFER_SIZE` with the size of the buffer used in the producer consumer, replace `PROP_DELAY` with the timing you'd like to introduce in the loop. This is to allow you to run the code and verify the validity of implementation rather than scrolling through all the spam verbose. I recommend a value of 1.0.
#### Exercise 9
cd to the directory and run the following command:
```bash
gcc -pthread -o ex9-processes ex9-prod-con-processes.c
gcc -pthread -o ex9-threads ex9-prod-con-threads.c
```

In the same directory, run the following command to run the code.
```bash
./ex9-processes <BUFFER_SIZE> <PRODUCER_LIMIT> <CONSUMER_LIMIT>
./ex9-threads <BUFFER_SIZE> <PRODUCER_LIMIT> <CONSUMER_LIMIT>
```
Replace `BUFFER_SIZE` with the size of the buffer used in the producer consumer, replace `PRODUCER_LIMIT` and `CONSUMER_LIMIT` with the value as desired (1000 for producers and 2000 for consumers as stated in the lab sheet)

## ANSWER TO EXERCISE 9
Empirically running the code, a speedup factor of 50 - 100x is observed for running the process implementation over the thread implementation. This evidence shows that the context switching savings made with multithreading are much lesser than the parallelism achieved through multiprocessing. Even with varying these parameters (limit for production and consumption) lower, the speed up factor remains.

My explanation for the following observation is as follows:
pthreads user library creates user-level threads, which become solely reliant on the OS level scheduler which are not context aware. Hence, user-level threads will be held lower in priority and well not be executed in time. The process implementation however allows the OS to be aware of such processes and can delegate resources as required, especially in using the multiple cores available in PC 014 that I'm using. Hence, with better scheduling and possible processor level parallelism, can speedup the execution of the producer-consumer problem. This far supersedes the context-switching savings made with multithreading, hence as observed.

## Known bugs (and idk or can't explain why it happens)
- For exercise 8, it'll occasionally fail to instantiate the child process and terminate.
