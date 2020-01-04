#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

// shortening struct timeval
#define TM struct timeval

// function declarations
double time_to_ms(TM tm);       // convert struct timeval to ms
void write_ms(const double ms); // write ms to the shared memory
double read_ms();               // read ms from the shared memory

// constants for shared memory
const int SIZE = 1;
const char *name = "os";

// MAIN
int main(int argc, char *argv[]){
    pid_t pid;

    pid = fork();

    if (pid < 0){
		fprintf(stderr, "Fork Failed");
		return 1;
	}
	else if (pid == 0){
        TM tm1;

        // get time before executing command
        gettimeofday(&tm1, NULL);
        double ms = time_to_ms(tm1);

		// producer - save time to the shared memory
        write_ms(ms);

        // execute the command
        execvp(argv[1], argv + 1);
	}
	else{
        TM tm2;
        wait(NULL);

        // get time after child process ends
        gettimeofday(&tm2, NULL);
        double ms2 = time_to_ms(tm2);

        // consumer - get time from the shared memory
        double ms1 = read_ms();

        // calculate elapsed time
        int timedif = (int)(ms2 - ms1);

        printf("Elapsed time: %dms\n", timedif);
	}

	return 0;
}

// convert struct timeval to milliseconds
double time_to_ms(TM tm){
    return (double)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

// producer
void write_ms(const double ms){
    // shared memory file descriptor
    int shm_fd;

    // pointer to shared memory object
    void *ptr;

    // create the shared memory object
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    // configure the size of the shared memory object
    ftruncate(shm_fd, SIZE);

    // memory map the shared memory object
    ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // writing ms to shared memory as char *
    sprintf(ptr, "%f", ms);
    ptr += 1;
}

// consumer
double read_ms(){
    // shared memory file descriptor
    int shm_fd;

    // pointer to shared memory object
    void *ptr;

    // open the shared memory object
    shm_fd = shm_open(name, O_RDONLY, 0666);

    // memory map the shared memory object
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // read from the shared memory object
    char *ms_str = (char *)ptr;

	// convert ms_str to double
    double ms = strtod(ms_str, 0);

    // remove the shared memory object
    shm_unlink(name);

    return ms;
}
