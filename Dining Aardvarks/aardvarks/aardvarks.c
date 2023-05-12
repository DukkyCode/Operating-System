#include "/comp/111/assignments/aardvarks/anthills.h"
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>


#define TRUE 1
#define FALSE 0

int initialized=FALSE; // semaphores and mutexes are not initialized

// define your mutexes and semaphores here
// they must be global variables.
sem_t anthill[ANTHILLS];
sem_t ants[ANTHILLS];
struct timespec req, rem;
int i, aardvark_count;

//Function to sleep 1.2 seconds
void waitonesec(){
	req.tv_sec = 1;
	req.tv_nsec = 200000000;

        //Sleep for 1.5 seconds
        if(nanosleep(&req, &rem) != 0){
                perror("Fail to sleep");
                exit(1);
        }
}

//Function to wait for 1.1 seconds and increment the semaphore
void *wait_to_post(void *sem_hill){
	int hill = *(int *)sem_hill;

	waitonesec();

	if(sem_post(&anthill[hill]) != 0){
		perror("Fail to increment semaphore");
                exit(1);
        }

	return NULL;
}

//My slurp function that keep tracks of the anthill semaphore and ants semaphore
void my_slurp(char aname, int hill){

	if(sem_trywait(&anthill[hill]) == 0){
		if(sem_trywait(&ants[hill]) == 0){
			pthread_t thread;
			pthread_create(&thread, NULL, wait_to_post, &hill);

			int slurp_cond = slurp(aname, hill);
			pthread_join(thread, NULL);

			if(!slurp_cond){
				sem_post(&ants[hill]);
			}
		}
		else{
			sem_post(&anthill[hill]);
		}

	}
	//return NULL;
}


// first thread initializes mutexes
void *aardvark(void *input) {
    char aname = *(char *)input;

    // first caller needs to initialize the mutexes!
    pthread_mutex_lock(&init_lock);

    if (!initialized++) { // this succeeds only for one thread
        // initialize your mutexes and semaphores here
	// Semaphore and ants per hill initialization
	for(i = 0; i < ANTHILLS; i++){
		if(sem_init(&anthill[i], 0 , AARDVARKS_PER_HILL) != 0) {
			perror("Fail to initiazlize semaphore");
		}

       		if(sem_init(&ants[i], 0 , ANTS_PER_HILL) != 0) {
                        perror("Fail to initiazlize semaphore");
                }
	}

    }

    pthread_mutex_unlock(&init_lock);

    // now be an aardvark
    while (chow_time()) {
        int hill = rand() % ANTHILLS;
	my_slurp(aname, hill);
    }

    //Cleaning up
    pthread_mutex_lock(&init_lock);
    aardvark_count++;

    if(aardvark_count == AARDVARKS){
	//Destroy semaphores
	for (i = 0; i < ANTHILLS; i++) {
		//printf("Destroying semaphores");
		sem_destroy(&anthill[i]);
		sem_destroy(&ants[i]);
        }
	aardvark_count++;
    }


    pthread_mutex_unlock(&init_lock);

    return NULL;
}
