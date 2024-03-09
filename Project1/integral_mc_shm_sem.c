//Konstantinos Anastasopoulos, 1093320
//Charalampos Anastasiou, 1093316
//Filippos Merkourios Dalas, 1096314

#define _SVID_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/mman.h>

sem_t *mutex; //global semaphore used to protect the critical section

void *create_and_map(char *fname, int *pfd, int size) //memory allocation
{
	int fd; //file descriptor
	void *map; //mmap return value to be stored here
	int result; //stretching variable

	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, (mode_t)0600); //opening file descriptor
	if (fd == -1) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}

	result = lseek(fd, size-1, SEEK_SET); //adjusting the file size
	if (result == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
	}

	result = write(fd, "", 1); //terminating with an extra byte
	if (result != 1) {
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //mapping of the shared memory
	if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	*pfd = fd;

	return map;
}

void unmap_and_close(void *map, int fd, int size) //unmap the memory
{
    if (munmap(map, size) == -1) {
        perror("Error un-mmapping the file");
    }

    close(fd); //closing the file descriptor
}

double get_wtime(void) //getting the current time
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec+(double)t.tv_usec*1.0e-6;
}

double f(double x) //function to be integrated
{
    return sin(cos(x));
}

int cores_count(void) //counting the number of cores the current processor has
{
    int cores=sysconf(_SC_NPROCESSORS_ONLN);
    if(cores<1)
        return -1;
    else
        return cores;
}

void worker(int id, int workers, double *shared_val) //worker function computing the partial integral
{
    double a=0.0; //start of the interval
    double b=1.0; //end of the interval
    unsigned long const n=24e7; //number of subintervals
    const double dx=(b-a)/n; //width of each subinterval
    double result=0; //partial result calculated by each worker
    const long tseed=10; //seed value to get a deterministic sequence of pseudorandom numbers
    srand48(tseed+id); //each worker gets different seed
    for(unsigned long i=id; i<n; i+=workers) //random values put in the function
    {
        double xi=drand48();
        result+=f(xi);
    }
    result*=dx;
    sem_wait(mutex); //DOWN operation
    *shared_val+=result; //partial result stored in the shared memory
    sem_post(mutex); //UP operation
}

int main(void) //main function
{
    double ref=0.73864299803689018; //approximate value of the integral calculated by Wolfram Alpha
    double t0, t1; //variables that store time, their difference tells the time it took to compute the integral
    double integral=0; //value of the integral
    int pid; //id of the processes
    int workers=0; //the number of worker processes
    int cores=cores_count(); //the number of cores
    int fd; //the file descriptor
    double *shared_val=0; //the shared memory value

    while(workers<=0 || workers>cores) //the user cannot enter a number of workers greater that the number of cores
    {
        printf("Enter the number of workers to compute the integral. It has to be at most equal to the number of available processor cores:\n");
        scanf("%d", &workers);
    }

    shared_val = create_and_map("data.bin", &fd, sizeof(double)); //creating the shared memory

    mutex=sem_open("/mutex", O_CREAT | O_EXCL, 0644, 1); //initializing the semaphore

    t0=get_wtime(); //getting the time in the start of the computation

    for(int i=0; i<workers; i++) //creating and using the workers
    {
        pid=fork();
        if(pid==0)
        {
            worker(i+1, workers, shared_val);
            unmap_and_close(shared_val, fd, sizeof(double));
            exit(0);
        }
    }

        for(int i=0; i<workers; i++) //the parent process waits for all workers to finish
    {
        waitpid(-1, NULL, 0);
    }

    sem_unlink("/mutex"); //closing the semaphore
    sem_close(mutex);

    integral=*shared_val; //the parent calculates the integral
    t1=get_wtime(); //getting the time in the end of the computation
    unmap_and_close(shared_val, fd, sizeof(double));
    unlink("data.bin");
    printf("time=%lf seconds\n result=%.16lf \n relative error=%e \n absolute error=%e \n", t1-t0, integral, fabs(integral-ref)/ref, fabs(integral-ref));

    return 0;
} //compile using this: gcc -std=c99 -o integral_mc_shm_sem integral_mc_shm_sem.c -lm -D_POSIX_C_SOURCE=200809L -pthread
