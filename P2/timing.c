#define _GNU_SOURCE

#include <sys/resource.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

long double elapsed_time(struct timeval start, struct timeval end){
	//Get the Elapsed seconds and microseconds
	long double seconds = end.tv_sec - start.tv_sec;
	long double useconds = end.tv_usec - start.tv_usec;

	if(useconds < 0){
		useconds += 1000000;
		seconds -=1;
	}

	long double ret = seconds * 1000000.0 + useconds;
	return ret;
}


//Main Function
int main(int argc, char const *argv[]){
	(void) argc;
	(void) argv;

	//Declaring variables
	int loop_time = 50000;			//Loop time
	int i;
	struct rusage start_time, end_time;

	//Measuring an empty loop
	getrusage(RUSAGE_SELF, &start_time);

	for(i = 0; i < loop_time; i++){
		//Empty Loop
	}

	getrusage(RUSAGE_SELF, &end_time);

	long double emptyLoop_utime = elapsed_time(start_time.ru_utime, end_time.ru_utime);
	long double emptyLoop_stime = elapsed_time(start_time.ru_stime, end_time.ru_stime);

	//Allocating One page of memory with mmap()
	getrusage(RUSAGE_SELF, &start_time);
	void *ptr;

        for(i = 0; i < loop_time; i++){
                //mmap()
		ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);

        }

	getrusage(RUSAGE_SELF, &end_time);

	//Calculating the mmap Average User time and system time
	long double mmap_utime = (elapsed_time(start_time.ru_utime, end_time.ru_utime) - emptyLoop_utime) / loop_time;
	long double mmap_stime = (elapsed_time(start_time.ru_stime, end_time.ru_stime) - emptyLoop_stime) / loop_time;

	printf("mmap() User Time: %Lf us\n", mmap_utime);
        printf("mmap() System Time: %Lf us\n", mmap_stime);

	//Lock a mutex with pthread_mutex_lock()
	//Mutex Variables
	pthread_mutex_t mutex[loop_time];

	for(i = 0; i < loop_time; i++){
		pthread_mutex_init(&mutex[i], NULL);
	}

	//Lock and Measure
	getrusage(RUSAGE_SELF, &start_time);

	for(i = 0; i < loop_time; i++){
		//Locking mechanism
		pthread_mutex_lock(&mutex[i]);
	}

	getrusage(RUSAGE_SELF, &end_time);

        //Calculating the mmap Average User time and system time
        long double lock_utime = (elapsed_time(start_time.ru_utime, end_time.ru_utime) - emptyLoop_utime) / loop_time;

	if(lock_utime < 0){
		lock_utime = fabsl(lock_utime);
	}

        long double lock_stime = (elapsed_time(start_time.ru_stime, end_time.ru_stime) - emptyLoop_stime) / loop_time;

	if(lock_stime < 0){
		lock_stime = fabsl(lock_stime);
	}


        printf("pthread_mutex_lock() User Time: %Lf us\n", lock_utime);
        printf("pthread_mutex_lock() System Time: %Lf us\n", lock_stime);

	//Unlock the mutexes after measurements
        for(i = 0; i < loop_time; i++){
                pthread_mutex_unlock(&mutex[i]);
        }

	////Part B:
	//Variables
	struct timeval start, end;
	char *buffer = NULL;
	char filename[] = "/tmp/TestAssignment2";

	//Measuring Wall-Clock time of an empty loop
	gettimeofday(&start, NULL);

	for(i = 0; i < loop_time; i++){
		//Empty Loop
	}

	gettimeofday(&end, NULL);

	long double emptyLoop_wtime = elapsed_time(start, end);

	//Writing 4096 bytes to /tmp
	int fd = open(filename, O_CREAT | O_RDWR | O_DIRECT | O_SYNC, 0666);

	if (fd < 0) {
        	perror("Failed to open file");
        	exit(1);
    	}

	//Align Memory Buffer
    	if (posix_memalign((void**)&buffer, 512, 4096) != 0) {
        	perror("Failed to allocate aligned memory");
        	exit(1);
    	}

	memset(buffer, 1, 4096);

	//Measuring the wall clock time
	gettimeofday(&start, NULL);

	for(i = 0; i < loop_time; i++){
		write(fd, buffer, 4096);
	}

	gettimeofday(&end, NULL);

	long double writeDirect_wtime = (elapsed_time(start, end) - emptyLoop_wtime)/ loop_time;

	printf("Writing 4096 bytes(bypassing the disk page cache): %Lf us\n", writeDirect_wtime);

	//Cleaning up
	free(buffer);
	close(fd);

	//if (unlink(filename) != 0){
        	//perror("Fail to Unlink the file");
        	//exit(1);
    	//}

	//Reading 4096 Bytes directly to the disk page cache
	fd = open(filename, O_CREAT | O_RDWR | O_DIRECT | O_SYNC, 0666);
	char *buffer1 = NULL;

        if (fd < 0) {
                perror("Failed to open file");
                exit(1);
        }

        //Align Memory Buffer
        if (posix_memalign((void**)&buffer1, 512, 4096) != 0) {
                perror("Failed to allocate aligned memory");
                exit(1);
        }

        memset(buffer1, 1, 4096);

	//Measuring the wall-clock Time

        gettimeofday(&start, NULL);

        for(i = 0; i < loop_time; i++){
                read(fd, buffer1, 4096);
        }

        gettimeofday(&end, NULL);

        long double readDirect_wtime = (elapsed_time(start, end) - emptyLoop_wtime)/ loop_time;

        printf("Reading 4096 bytes (bypassing the disk page cache): %Lf us\n", readDirect_wtime);

        //Cleaning up
        free(buffer1);
        close(fd);

	//if (unlink(filename) != 0){
        	//perror("Fail to Unlink the file");
        	//exit(1);
    	//}


	//Writing 4096 bytes to the disk page cache
    	fd = open(filename, O_CREAT | O_WRONLY, 0666);
	char *buffer2;
    	if (fd < 0) {
        	perror("Failed to open file");
        	exit(1);
    	}

	//Allocating Memory
    	buffer2 = malloc(4096);
    	if (buffer2 == NULL) {
        	perror("Failed to allocate memory");
        	exit(1);
    	}
	memset(buffer2, 1, 4096);

	//Measuring Wall Clock time
        gettimeofday(&start, NULL);

        for(i = 0; i < loop_time; i++){
                write(fd, buffer2, 4096);
        }

        gettimeofday(&end, NULL);

        long double write_wtime = (elapsed_time(start, end) - emptyLoop_wtime)/ loop_time;

        printf("Writing 4096 bytes to disk page cache: %Lf us\n", write_wtime);

        //Cleaning up
        free(buffer2);
        close(fd);

        //if (unlink(filename) != 0){
                //perror("Fail to Unlink the file");
                //exit(1);
        //}

        //Reading 4096 bytes to the disk page cache
        fd = open(filename, O_CREAT | O_RDONLY, 0666);
        char *buffer3;
        if (fd < 0) {
                perror("Failed to open file");
                exit(1);
        }

        //Allocating Memory
        buffer3 = malloc(4096);
        if (buffer3 == NULL) {
                perror("Failed to allocate memory");
                exit(1);
        }

        memset(buffer3, 1, 4096);

        //Measuring Wall Clock time
        gettimeofday(&start, NULL);

        for(i = 0; i < loop_time; i++){
                read(fd, buffer3, 4096);
        }

        gettimeofday(&end, NULL);

        long double read_wtime = (elapsed_time(start, end) - emptyLoop_wtime)/ loop_time;

        printf("Reading 4096 bytes to disk page cache: %Lf us\n", read_wtime);

        //Cleaning up
        free(buffer3);
        close(fd);

        if (unlink(filename) != 0){
                perror("Fail to Unlink the file");
                exit(1);
        }


	return 0;
}









