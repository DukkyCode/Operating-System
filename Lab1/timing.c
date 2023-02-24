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

pthread_mutex_t mutex;
/*Main Function*/
int main (int argc, char const *argv[]){
	// Declare Variables
	(void)argc;
	(void)argv;
	struct rusage usage;
	struct timeval start_user, start_system, end_user, end_system;

	// Measuring mmap() Reousrce Usage
	getrusage(RUSAGE_SELF, &usage);
	start_user = usage.ru_utime;
	start_system = usage.ru_stime;

	void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);

	getrusage(RUSAGE_SELF, &usage);
        end_user = usage.ru_utime;
	end_system = usage.ru_stime;

	if(ptr == MAP_FAILED){
		perror("Fail to allocate mmap");
		exit(1);
	}

	long mmap_user_sec  = end_user.tv_sec - start_user.tv_sec;
	long mmap_user_usec = end_user.tv_usec - start_user.tv_usec;

        long mmap_system_sec  = end_system.tv_sec - start_system.tv_sec;
        long mmap_system_usec = end_system.tv_usec - start_system.tv_usec;

	// Print User time and System time to allocate mmap
	printf("mmap() User time: %ld.%.06ld s\n", mmap_user_sec, mmap_user_usec);
	printf("mmap() System time: %ld.%.06ld s\n", mmap_system_sec, mmap_system_usec);

	//Mutex Initialization
	int mut_ret = pthread_mutex_init(&mutex, NULL);
	if (mut_ret != 0){
		perror("Fail to Initialize Mutex");
		exit(1);
	}

	// Measuring Locking the thread_mutex
        getrusage(RUSAGE_SELF, &usage);
        start_user = usage.ru_utime;
        start_system = usage.ru_stime;


	int lock_ret = pthread_mutex_lock(&mutex);

        if (lock_ret != 0){
                perror("Fail to lock Mutex");
                exit(1);
        }

        getrusage(RUSAGE_SELF, &usage);
        end_user = usage.ru_utime;
        end_system = usage.ru_stime;

        long lock_user_sec  = end_user.tv_sec - start_user.tv_sec;
        long lock_user_usec = end_user.tv_usec - start_user.tv_usec;

        long lock_system_sec  = end_system.tv_sec - start_system.tv_sec;
        long lock_system_usec = end_system.tv_usec - start_system.tv_usec;

        // Print User time and System time to Lock the Mutex
        printf("pthread_mutex_lock() User time: %ld.%.06ld s\n", lock_user_sec, lock_user_usec);
        printf("pthread_mutex_lock() System time: %ld.%.06ld s\n", lock_system_sec, lock_system_usec);

        // Unlock the Mutex
        int unlock_ret = pthread_mutex_unlock(&mutex);

        if (unlock_ret != 0){
                perror("Fail to unlock Mutex");
                exit(1);
        }

        // Destroy the Mutex & Deallocate Memory
        int destroy_ret = pthread_mutex_lock(&mutex);

        if (destroy_ret != 0){
                perror("Fail to Destroy Mutex");
                exit(1);
        }

        int unmap = munmap(ptr, 4096);
        if (unmap != 0) {
                perror("Fail to deallocate mmap");
                exit(1);
        }

	//Second Part:
	//Create a file desciptor in the temp directory
	//Variables for Part II
	struct timeval start, end;
	long time_sec, time_usec;

	char *buffer = NULL;
	int file_ret;
	char filename[] = "/tmp/TestAssignment2";
	/////////////////////////////////////////////////
	//Write 4096 bytes to /tmp
	int fd = open(filename, O_CREAT | O_WRONLY | O_DIRECT | O_SYNC, 0666);
   	if (fd < 0) {
        	perror("Failed to open file");
        	exit(1);
    	}

	//Align Memory Buffer
    	if (posix_memalign((void**)&buffer, 512, 4096) != 0) {
        	perror("Failed to allocate aligned memory");
        	exit(1);
    	}

	//Write Execution
	gettimeofday(&start, NULL);
	file_ret = write(fd, buffer, 4096);
	gettimeofday(&end, NULL);

	time_sec = end.tv_sec - start.tv_sec;
	time_usec = end.tv_usec - start.tv_usec;

	printf("Time to write 4096 bytes to /tmp: %ld.%06ld\n", time_sec, time_usec);

	if(file_ret < 0){
		perror("Fail to write to file");
		exit(1);
	}
	//Cleaning up
	free(buffer);
	close(fd);
	if (unlink(filename) != 0){
        	perror("Fail to Unlink the file");
        	exit(1);
    	}
	////////////////////////////////////////////
        //Read 4096 bytes from /tmp
        fd = open(filename, O_CREAT | O_RDWR | O_DIRECT | O_SYNC, 0666);
        if (fd < 0) {
                perror("Failed to open file");
                exit(1);
        }

        //Align Memory Buffer
        if (posix_memalign((void**)&buffer, 512, 4096) != 0) {
                perror("Failed to allocate aligned memory");
                exit(1);
        }

        //Executing Read 
        gettimeofday(&start, NULL);
        file_ret = read(fd, buffer, 4096);
        gettimeofday(&end, NULL);

        time_sec = end.tv_sec - start.tv_sec;
        time_usec = end.tv_usec - start.tv_usec;

        printf("Time to read 4096 bytes to /tmp: %ld.%06ld\n", time_sec, time_usec);

        if(file_ret < 0){
                perror("Fail to write to file");
                exit(1);
        }

	//Cleaning up
        free(buffer);
        close(fd);
        if (unlink(filename) != 0){
                perror("Fail to Unlink the file");
                exit(1);
        }

	////////////////////////////////////////////////////
    	// Writing 4096 Bytes to the disk page cache
    	fd = open(filename, O_CREAT | O_WRONLY, 0666);
    	if (fd < 0) {
        	perror("Failed to open file");
        	exit(1);
    	}
	//Allocating Memory
    	buffer = malloc(4096);
    	if (buffer == NULL) {
        	perror("Failed to allocate memory");
        	exit(1);
    	}
        //Executing Write
        gettimeofday(&start, NULL);
        file_ret = write(fd, buffer, 4096);
        gettimeofday(&end, NULL);

        time_sec = end.tv_sec - start.tv_sec;
        time_usec = end.tv_usec - start.tv_usec;

        printf("Time to write 4096 bytes to disk page cache: %ld.%06ld\n", time_sec, time_usec);

        if(file_ret < 0){
                perror("Fail to write to file");
                exit(1);
        }

        //Cleaning up
        free(buffer);
        close(fd);
        if (unlink(filename) != 0){
                perror("Fail to Unlink the file");
                exit(1);
        }
        ////////////////////////////////////////////////////
        // Reading 4096 Bytes to the disk page cache
        fd = open(filename, O_CREAT | O_RDWR, 0666);
        if (fd < 0) {
                perror("Failed to open file");
                exit(1);
        }
        //Allocating Memory
        buffer = malloc(4096);
        if (buffer == NULL) {
                perror("Failed to allocate memory");
                exit(1);
        }
        //Executing Read
        gettimeofday(&start, NULL);
        file_ret = read(fd, buffer, 4096);
        gettimeofday(&end, NULL);

        time_sec = end.tv_sec - start.tv_sec;
        time_usec = end.tv_usec - start.tv_usec;

        printf("Time to read 4096 bytes to disk page cache: %ld.%06ld\n", time_sec, time_usec);

        if(file_ret < 0){
                perror("Fail to write to file");
                exit(1);
        }

        //Cleaning up
        free(buffer);
        close(fd);
        if (unlink(filename) != 0){
                perror("Fail to Unlink the file");
                exit(1);
        }

	return 0;
}
