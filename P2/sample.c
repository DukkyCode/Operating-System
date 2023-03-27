#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#define FILE_SIZE 4096
#define ALIGNMENT 512

int main()
{
    char *buffer, *aligned_buffer;
    int fd, ret;
    struct timeval start, end, diff;
    char filename[] = "/tmp/testfile";

    // Allocate memory for the buffer and align it
    if (posix_memalign((void **)&buffer, ALIGNMENT, FILE_SIZE + ALIGNMENT) != 0)
    {
        perror("posix_memalign");
        exit(EXIT_FAILURE);
    }
    aligned_buffer = buffer + ALIGNMENT - ((uintptr_t)buffer % ALIGNMENT);

    // Open the file with O_DIRECT flag
    fd = open(filename, O_CREAT | O_RDWR | O_DIRECT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Measure time for direct write to file
    gettimeofday(&start, NULL);
    ret = write(fd, aligned_buffer, FILE_SIZE);
    if (ret == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &diff);
    printf("Direct write took %ld.%06ld seconds\n", diff.tv_sec, diff.tv_usec);

    // Move file pointer back to the beginning of the file
    lseek(fd, 0, SEEK_SET);

    // Measure time for direct read from file
    gettimeofday(&start, NULL);
    ret = read(fd, aligned_buffer, FILE_SIZE);
    if (ret == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &diff);
    printf("Direct read took %ld.%06ld seconds\n", diff.tv_sec, diff.tv_usec);

    // Write to file without O_DIRECT flag
    ret = write(fd, buffer, FILE_SIZE);
    if (ret == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Move file pointer back to the beginning of the file
    lseek(fd, 0, SEEK_SET);

    // Measure time for read from file with disk page cache
    gettimeofday(&start, NULL);
    ret = read(fd, buffer, FILE_SIZE);
    if (ret == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &diff);
    printf("Read with page cache took %ld.%06ld seconds\n", diff.tv_sec, diff.tv_usec);

    // Close the file and delete it
    close(fd);
    if (unlink(filename) != 0)
    {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    // Free the memory buffer
    free(buffer);

    return 0;
}
