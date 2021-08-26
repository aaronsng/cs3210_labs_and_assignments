/*************************************************************************************************************************
 * Name: Sng Li Wen, Aaron, Student ID: A0242334N 
 * ex8-prod-con-processes.c
 * To test producer consumer using processes and semaphores. Propagation delay to add a sleep in every time a process runs
 * Vary it so you can verify the output and validity of the implementation
 * Compile: gcc -pthread -o ex8 ex8-prod-con-processes.c
 * Run: ./ex8 <BUFFER_SIZE> <PROP_DELAY>
 *************************************************************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define DEBUG 0

int size(int * f, int * r, int buffer_size) {
  if (*r > *f)
    return *r - *f + 1;
  else if ((*r == -1) || (*f == -1))
    return 0;
  else if (*r == *f)
    return 1;
  else
    return buffer_size - (*f - *r) + 1;
}

void load(int * f, int * r, int buffer_size) {
  // printf("load f = %d, r = %d\n", *f, *r);
  if (*r == -1) { // empty
    *r = *f = 0;
  } else {
    int r_ = *r;
    *r = *r + 1;
    *r = *r % buffer_size;
    if (*r == *f) // full illegal instruction
      return;
  }
}

void unload(int * f, int * r, int buffer_size) {
  // printf("unload f = %d, r = %d\n", *f, *r);
  if (*r == -1)
    return; // empty don't load anything

  if (*r == *f) {
    *r = *f = -1;
  } else {
    *f = *f + 1;
    *f = *f % buffer_size;
  }

}

void print_queue(int * f, int * r, int * p, int buffer_size) 
{
    int i = size(f, r, buffer_size);
    int j = *f;
    while (i != 0) 
    {
        printf("%d ", p[j]);
        j++;
        j = j % buffer_size;
        i--;
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    /*      loop variables          */
    int i, buffer_size;
    double prop_delay;
    /*      shared memory key       */
    key_t shmkey, frontkey, backkey;
    /*      shared memory id        */
    int shmid, frontid, backid;
    /*      synch semaphore         */ /*shared */
    sem_t* mutex;
    sem_t* spaces;
    sem_t* items;
    /*      fork pid                */
    pid_t pid;
    /*      fork count              */
    unsigned int n;
    /*      semaphore value         */
    unsigned int value, spaces_, items_;
    /*      shared variables        */ /*shared */
    int* p;
    int* f;
    int* r;
    
    if (argc < 3) {
        printf("Not enough arguments\n");
        printf("./ex8 <BUFFER_SIZE> <DELAY>");
        exit(-1);
    }

    buffer_size = atoi(argv[1]);
    prop_delay = atof(argv[2]);

    /* initialize a shared variable in shared memory */
    shmkey = ftok("/dev/null", 5); /* valid directory name and a number */
    shmid = shmget(shmkey, buffer_size * sizeof(int), 0644 | IPC_CREAT);
    if (shmid < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    frontkey = ftok("/dev/null", 4); /* valid directory name and a number */
    frontid = shmget(frontkey, sizeof(int), 0644 | IPC_CREAT);
    if (frontid < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    backkey = ftok("/dev/null", 3); /* valid directory name and a number */
    backid = shmget(backkey, sizeof(int), 0644 | IPC_CREAT);
    if (backid < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    p = (int*) shmat(shmid, NULL, 0); /* attach p to shared memory */
    f = (int*) shmat(frontid, NULL, 0);
    r = (int*) shmat(backid, NULL, 0);

    /********************************************************/
    n = 3;
    value = 1;
    spaces_ = buffer_size;
    items_ = 0;
    *f = -1; 
    *r = -1;

    /* initialize semaphores for shared processes */
    mutex = sem_open("mutex", O_CREAT | O_EXCL, 0644, value);
    spaces = sem_open("spaces", O_CREAT | O_EXCL, 0644, spaces_);
    items = sem_open("items", O_CREAT | O_EXCL, 0644, items_);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    printf("Semaphore initialized.\n\n");

    /* fork child processes */
    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            /* check for error      */
            sem_unlink("mutex");
            sem_close(mutex);
            sem_unlink("spaces");
            sem_close(spaces);
            sem_unlink("items");
            sem_close(items);
            /* unlink prevents the semaphore existing forever */
            /* if a crash occurs during the execution         */
            printf("Fork error.\n");
        } else if (pid == 0)
            break; /* child processes */
    }

    /******************************************************/
    /******************   PARENT PROCESS   ****************/
    /******************************************************/
    if (pid != 0) {
        /* wait for all children to exit */
        while (pid = waitpid(-1, NULL, 0)) {
            if (errno == ECHILD)
                break;
        }
        printf("\nParent: All children have exited.\n");

        /* shared memory detach */
        shmdt(p);
        shmctl(shmid, IPC_RMID, 0);
        shmdt(f);
        shmctl(frontid, IPC_RMID, 0);
        shmdt(r);
        shmctl(backid, IPC_RMID, 0);

        /* cleanup semaphores */
        sem_unlink("mutex");
        sem_close(mutex);
        sem_unlink("spaces");
        sem_close(spaces);
        sem_unlink("items");
        sem_close(items);
        /* unlink prevents the semaphore existing forever */
        /* if a crash occurs during the execution         */
        exit(0);
    }

    /******************************************************/
    /******************   CHILD PROCESS   *****************/
    /******************************************************/

    // HARD CODING 
    // i = 0  :  Producer
    // i = 1  :  Producer
    // i = 2  :  Consumer

    else {
        if (i == 0) {
            while(1) {
                sleep(prop_delay);
                sem_wait(spaces);
                sem_wait(mutex);

                /************* CRITICAL SECTION STARTS HERE *************/
                load(f, r, buffer_size);
                p[*r] = rand() % 10 + 1;
                printf("Producer %d produced %d, buffer now %d\n", i, p[*r], size(f, r, buffer_size));

                if (DEBUG)
                    printf("使いました %d\n", i);
                /************* CRITICAL SECTION ENDS HERE   *************/

                sem_post(mutex);
                sem_post(items);
            }
        } else if (i == 1) {
            while(1) {
                sleep(prop_delay);
                sem_wait(spaces);
                sem_wait(mutex);

                /************* CRITICAL SECTION STARTS HERE *************/
                load(f, r, buffer_size);
                p[*r] = rand() % 10 + 1;
                printf("Producer %d produced %d, buffer now %d\n", i, p[*r], size(f, r, buffer_size));

                if (DEBUG)
                    printf("使いました %d\n", i);
                /************* CRITICAL SECTION ENDS HERE   *************/

                sem_post(mutex);
                sem_post(items);
            }
        } else if (i == 2) {
            while (1) {
                sleep(prop_delay);
                sem_wait(items);
                sem_wait(mutex);

                /************** CRITICAL SECTION STARTS HERE ************/
                int _f = *f;
                unload(f, r, buffer_size);

                printf("Consumer %ld consumed %d, buffer now %d\n", i, p[_f], size(f, r, buffer_size));
                if (DEBUG)
                    printf("食べています %d\n", i); 
                /************* CRITICAL SECTION ENDS HERE   *************/

                sem_post(mutex);
                sem_post(spaces);
            }
        }
        exit(0);
    }
}
