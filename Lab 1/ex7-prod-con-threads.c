/*************************************************************************************************************************
 * Name: Sng Li Wen, Aaron, Student ID: A0242334N
 * ex7-prod-con-threads.c
 * To test producer consumer using threads. Propagation delay to add a sleep in every time a process runs
 * Vary it so you can verify the output and validity of the implementation
 * Compile: gcc -pthread -o ex7 ex7-prod-con-threads.c
 * Run: ./ex7 <BUFFER_SIZE> <PROP_DELAY>
 *************************************************************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 

#define PRODUCERS 2
#define CONSUMERS 1

// Buffer variables
int* producer_buffer;
int f = -1, r = -1;
int BUFFER_SIZE, PROP_DELAY;

// pthread variables
pthread_mutex_t lock; // bad naming welp
pthread_cond_t items, spaces; 

int size(void) {
  if (r > f)
    return r - f + 1;
  else if ((r == -1) || (f == -1))
    return 0;
  else if (r == f)
    return 1;
  else
    return BUFFER_SIZE - (f - r) + 1;
}

void load(void) {
  if (r == -1) { // empty
    r = f = 0;
  } else {
    int r_ = r;
    r++;
    r = r % BUFFER_SIZE;
    if (r == f) // full illegal instruction
      return;
  } 
  producer_buffer[r] = (rand() % 10) + 1;
}

void unload(void) {
  if (r == -1)
    return; // empty don't load anything
 
  if (r == f) {
    r = f = -1;
  } else { 
    f++;
    f = f % BUFFER_SIZE;
  }

}
void* producer(void* threadid)
{
  long tid = (long) threadid;

  while(1) {
    sleep(PROP_DELAY);
    pthread_mutex_lock(&lock);
    while (size() == BUFFER_SIZE) {
      printf("Waiting for spaces\n");
      pthread_cond_wait(&spaces, &lock);
    }
    
    /********** Critical Section Starts Here **************/
    load();
    printf("Producer %ld produced %d, buffer now %d\n", tid, producer_buffer[r], size());
    /********** Critical Section Ends Here ****************/
    
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&items);
  }
  pthread_exit(NULL);
}
void* consumer(void* threadid)
{
  long tid = (long) threadid;

  while(1) {
    sleep(PROP_DELAY);
    pthread_mutex_lock(&lock);
    while (size() == 0) { 
      printf("Waiting for items\n");
      pthread_cond_wait(&items, &lock);
    }

    /********** Critical Section Starts Here **************/
    int dequeued_value = producer_buffer[f];
    unload();
    printf("Consumer %ld consumed %d, buffer now %d\n", tid, dequeued_value, size());
    /********** Critical Section Ends Here   **************/

    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&spaces);
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
      printf("Set the buffer size and test size!\n");
      printf("./ex7 <BUFFER_SIZE> <PROP_DELAY>\n");
      exit(-1);
    }

    BUFFER_SIZE = atoi(argv[1]);
    PROP_DELAY = atof(argv[2]);
    
    producer_buffer = malloc(BUFFER_SIZE * sizeof(int));

    pthread_t producer_threads[PRODUCERS];
    pthread_t consumer_threads[CONSUMERS];
    int producer_threadid[PRODUCERS];
    int consumer_threadid[CONSUMERS];

    /*********** Initialise pthread variables ************/
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&items, NULL);
    pthread_cond_init(&spaces, NULL);
    r = f = -1;

    int rc;
    int t1, t2;
    for (t1 = 0; t1 < PRODUCERS; t1++) {
        int tid = t1;
        producer_threadid[tid] = tid;
        printf("Main: creating producer %d\n", tid);
        rc = pthread_create(&producer_threads[tid], NULL, producer,
                (void*) producer_threadid[tid]);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t2 = 0; t2 < CONSUMERS; t2++) {
        int tid = t2;
        consumer_threadid[tid] = tid;
        printf("Main: creating consumer %d\n", tid);
        rc = pthread_create(&consumer_threads[tid], NULL, consumer,
                (void*) consumer_threadid[tid]);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    /********* Join threads and check timing **********/
    for (t1 = 0; t1 < PRODUCERS; t1++)
        pthread_join(producer_threads[t1], NULL);

    for (t2 = 0; t2 < CONSUMERS; t2++)
        pthread_join(consumer_threads[t2], NULL);

    /******************* Clean up *********************/
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&items);
    pthread_cond_destroy(&spaces);
    pthread_exit(NULL);
}
