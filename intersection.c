#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "arrivals.h"
#include "intersection_time.h"
#include "input.h"

#define H(i,j) (i + j*100)

static pthread_mutex_t mutex[8] = PTHREAD_MUTEX_INITIALIZER;

/* 
 * curr_arrivals[][][]
 *
 * A 3D array that stores the arrivals that have occurred
 * The first two indices determine the entry lane: first index is Side, second index is Direction
 * curr_arrivals[s][d] returns an array of all arrivals for the entry lane on side s for direction d,
 *   ordered in the same order as they arrived
 */
static Arrival curr_arrivals[4][4][20];

/*
 * semaphores[][]
 *
 * A 2D array that defines a semaphore for each entry lane,
 *   which are used to signal the corresponding traffic light that a car has arrived
 * The two indices determine the entry lane: first index is Side, second index is Direction
 */
static sem_t semaphores[4][4];

/*
 * supply_arrivals()
 *
 * A function for supplying arrivals to the intersection
 * This should be executed by a separate thread
 */
static void* supply_arrivals()
{
  fprintf(stderr, "Supplyer: Starting Thread...\n");
  int t = 0;
  int num_curr_arrivals[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

  // for every arrival in the list
  for (int i = 0; i < sizeof(input_arrivals)/sizeof(Arrival); i++)
  {
    // get the next arrival in the list
    Arrival arrival = input_arrivals[i];
    // wait until this arrival is supposed to arrive
    sleep(arrival.time - t);
    t = arrival.time;
    // store the new arrival in curr_arrivals
    curr_arrivals[arrival.side][arrival.direction][num_curr_arrivals[arrival.side][arrival.direction]] = arrival;
    fprintf(stderr, "New Arrival at Intersection, ID:%d Side:%d Direction:%d\n", arrival.id, arrival.side, arrival.direction);
    num_curr_arrivals[arrival.side][arrival.direction] += 1;
    // increment the semaphore for the traffic light that the arrival is for
    fprintf(stderr, "Supplyer: Sending Semaphore Signal...\n");
    sem_post(&semaphores[arrival.side][arrival.direction]);
    semaphores[arrival.side][arrival.direction].__align += (arrival.id);
    
  }
  fprintf(stderr, "Supplyer: Thread Finished\n");

  return(0);
}


/*
 * manage_light(void* arg)
 *
 * A function that implements the behaviour of a traffic light
 */
void* manage_light(void* arg) {
  while (get_time_passed() < END_TIME){
    int* num = (int*)arg;
    int i = num[0];
    int j = num[1];
    //fprintf(stderr, "Light %d %d: Waiting for Semaphore Signal...\n", i, j);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += END_TIME;
    int result = sem_timedwait(&semaphores[i][j], &ts);
    if(result == -1 && errno == ETIMEDOUT) {
      break;
    } else {
        fprintf (stderr,"%d %d: Car Arrived. Requesting Mutexes...\n", i, j);
        //Locking Mutexes
        switch (H(i,j)){                      //DOES NOT WORK, MUTEX LOGIC IS NOT CORRECT
          //Turning Right or U-Turn
          case H(2,2):
            pthread_mutex_lock(&mutex[5]);
          break;
          case H(3,2):
            pthread_mutex_lock(&mutex[6]);
          break;
          case H(2,3):
            pthread_mutex_lock(&mutex[6]);
          break;
          case H(1,2):
            pthread_mutex_lock(&mutex[4]);
          break;
          //Turning Left
          case H(3,0):
            pthread_mutex_lock(&mutex[3]);
            pthread_mutex_lock(&mutex[4]);
            pthread_mutex_lock(&mutex[1]);
          break;
          case H(2,0):
            pthread_mutex_lock(&mutex[2]);
            pthread_mutex_lock(&mutex[3]);
            pthread_mutex_lock(&mutex[7]);
          break;
          case H(1,0):
            pthread_mutex_lock(&mutex[0]);
            pthread_mutex_lock(&mutex[2]);
            pthread_mutex_lock(&mutex[6]);
          break;
          //Going Straight
          case H(1,1):
            pthread_mutex_lock(&mutex[1]);
            pthread_mutex_lock(&mutex[3]);
            pthread_mutex_lock(&mutex[7]);
            break;
          case H(3,1):
            pthread_mutex_lock(&mutex[0]);
            pthread_mutex_lock(&mutex[2]);
            pthread_mutex_lock(&mutex[5]);
          break;
          case H(2,1):
            pthread_mutex_lock(&mutex[0]);
            pthread_mutex_lock(&mutex[1]);
            pthread_mutex_lock(&mutex[4]);
          break;
          default:
          fprintf(stderr, "ERROR: Incorrect Arrival Data\n");
          exit(1);
        }

        fprintf (stderr,"%d %d: Locked Mutexes\n", i, j);
        long carID = semaphores[i][j].__align;
        fprintf(stderr, "CARID: %ld \n", carID);
        semaphores[i][j].__align -= carID;
        printf("traffic light %d %d turns green at time %d for car %ld\n", i, j, get_time_passed(), carID);
        sleep(CROSS_TIME);
        //Unlocking Mutexes
        switch (H(i,j)){                      //DOES NOT WORK, MUTEX LOGIC IS NOT CORRECT
          //Turning Right or U-Turn
          case H(2,2):
            pthread_mutex_unlock(&mutex[5]);
          break;
          case H(3,2):
            pthread_mutex_unlock(&mutex[6]);
          break;
          case H(2,3):
            pthread_mutex_unlock(&mutex[6]);
          break;
          case H(1,2):
            pthread_mutex_unlock(&mutex[4]);
          break;
          //Turning Left
          case H(3,0):
            pthread_mutex_unlock(&mutex[3]);
            pthread_mutex_unlock(&mutex[4]);
            pthread_mutex_unlock(&mutex[1]);
          break;
          case H(2,0):
            pthread_mutex_unlock(&mutex[2]);
            pthread_mutex_unlock(&mutex[3]);
            pthread_mutex_unlock(&mutex[7]);
          break;
          case H(1,0):
            pthread_mutex_unlock(&mutex[0]);
            pthread_mutex_unlock(&mutex[2]);
            pthread_mutex_unlock(&mutex[6]);
          break;
          //Going Straight
          case H(1,1):
            pthread_mutex_unlock(&mutex[1]);
            pthread_mutex_unlock(&mutex[3]);
            pthread_mutex_unlock(&mutex[7]);
            break;
          case H(3,1):
            pthread_mutex_unlock(&mutex[0]);
            pthread_mutex_unlock(&mutex[2]);
            pthread_mutex_unlock(&mutex[5]);
          break;
          case H(2,1):
            pthread_mutex_unlock(&mutex[0]);
            pthread_mutex_unlock(&mutex[1]);
            pthread_mutex_unlock(&mutex[4]);
          break;
          default:
          fprintf(stderr, "ERROR: Incorrect Arrival Data\n");
          exit(1);
        }
        fprintf (stderr,"%d %d: Unlocking Mutex...\n", i, j);
        printf("traffic light %d %d turns red at time %d\n", i, j, get_time_passed());
        
        fprintf(stderr, "Timer: %d\n", get_time_passed());
    }
  };
  free(arg);
  return(0);
}


int main(int argc, char * argv[])
{
  // create semaphores to wait/signal for arrivals
  fprintf(stderr, "Main: Booting Up...\n");
  fprintf(stderr, "Main: Creating Semaphores...\n");
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_init(&semaphores[i][j], 0, 0);
    }
  }
  fprintf(stderr, "Main: Semaphores Created\n");
  // start the timer
  start_time();

  pthread_t supplyThread;
  pthread_t lightThreads[4][4];

  // Create thread for supply_arrivals
  fprintf(stderr, "Main: Creating SupplyThread...\n");
  pthread_create(&supplyThread, NULL, supply_arrivals, NULL);

  // Create thread for manage_light
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int* num = malloc(sizeof(int) * 2); // Allocate memory for two integers
            if (num == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            num[0] = i; // Assign the values
            num[1] = j;
            fprintf(stderr, "Main: Creating LightThreads...\n");
            pthread_create(&lightThreads[i][j], NULL, manage_light, num);
        }
    }

  // TODO: wait for all threads to finish
  pthread_join(supplyThread, NULL);
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
    pthread_join(lightThreads[i][j], NULL);
    }
  }
  // destroy semaphores
  fprintf(stderr, "Main: Destroying Semaphores...\n");
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      sem_destroy(&semaphores[i][j]);
    }
  }
  fprintf(stderr, "Main: Finished! Shutting Down...\n");
}
