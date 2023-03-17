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
int ants[3];
sem_t anthill[3];
pthread_mutex_t mutex_ant, mutex;

struct timespec req, rem;
int i, hill;
int aardvark_count = 0;

// Separate threads to wait for the 2nd second of a successful slurps so that other aardvarks can
// start slurping
void *wait_to_post(void *sem){
	req.tv_sec = 1;
	req.tv_nsec = 550000000;

        //Sleep for 1.5 seconds
        if(nanosleep(&req, &rem) != 0){
                perror("Fail to sleep");
                exit(1);
        }

        //Increment the anthill semaphore so that it is ready for other threads
        if(sem_post((sem_t *) sem) != 0){
                perror("Fail to increment semaphore");
                exit(1);
        }

	return NULL;
}

// first thread initializes mutexes 
void *aardvark(void *input) { 
    char aname = *(char *)input; 
    // first caller needs to initialize the mutexes!
    pthread_mutex_lock(&init_lock); 
    if (!initialized++) { // this succeeds only for one thread
        // initialize your mutexes and semaphores here

	// Mutex initialization
	if(pthread_mutex_init(&mutex_ant, NULL) != 0){
		perror("Fail to initialize mutex");
		exit(0);
	}

        if(pthread_mutex_init(&mutex, NULL) != 0){
                perror("Fail to initialize mutex");
                exit(0);
        }

	// Semaphore and ants per hill initialization
	for(i = 0; i < ANTHILLS; i++){
		if(sem_init(&anthill[i], 0 , AARDVARKS_PER_HILL) != 0) {
			perror("Fail to initiazlize semaphore");
		}
		ants[i] = ANTS_PER_HILL;
	}

    }
    pthread_mutex_unlock(&init_lock);

    // now be an aardvark
    while (chow_time()) { 
        pthread_mutex_lock(&mutex);

	//Select a random hill
        hill = rand() % ANTHILLS;

	//Decrement the anthill semaphore
	if(sem_trywait(&anthill[hill]) != -1){
		//Create the a separate thread to keep track of the second of slurp 
		pthread_t thread;
		pthread_create(&thread, NULL, wait_to_post, &anthill[hill]);

		pthread_mutex_unlock(&mutex);

		int slurp_cond = slurp(aname, hill);

		//Slurping mechanism as it only decrement the remaining ants if slurp is sucessful
		if(slurp_cond == 1){
			pthread_mutex_lock(&mutex_ant);
			ants[hill]--;
			pthread_mutex_unlock(&mutex_ant);

		}

		pthread_join(thread, NULL);
	}
	else{
		pthread_mutex_unlock(&mutex);
	}
    }

    //If it reaches here, check if it is the last thread that finishes
    pthread_mutex_lock(&init_lock);
    aardvark_count++;

    if(aardvark_count == AARDVARKS){
        // Destroy Mutexes
        if(pthread_mutex_destroy(&mutex_ant) != 0){
                perror("Fail to destroy mutex");
                exit(0);
        }

        if(pthread_mutex_destroy(&mutex) != 0){
                perror("Fail to destroy mutex");
                exit(0);
        }

	//Destroy semaphores
	for (i = 0; i < ANTHILLS; i++) {
		sem_destroy(&anthill[i]);
        }
    }


    pthread_mutex_unlock(&init_lock);

    return NULL; 
} 
