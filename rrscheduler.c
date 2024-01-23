/* Konstantinos Anastasopoulos, 1093320 */
/* Charalampos Anastasiou, 1093316 */
/* Filippos Merkourios Dalas, 1096314 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>


//process descriptor struct with basic info about each process
struct Process {
    char name[50];
    pid_t pid;
    time_t enter;
    char state[10];
    struct Process* next;
    struct Process* previous;
};

//global variables that set the boundaries of the queue
struct Process* tail = NULL;
struct Process* head = NULL;

//pipe to send messages from the main program to the signal handler
int pipe_fd[2];

//function that adds another process descriptor struct to the queue
void enqueue(struct Process* process, struct Process** head) {
    if (*head != NULL) { //the queue is not empty
        struct Process* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = process;
        process->previous = temp;
    } else { //the queue is empty
        *head = process;
    }
}

//function that removes the front process descriptor struct from the queue
struct Process* dequeue(struct Process** head) {
    if (*head != NULL) { //the queue is not empty
        struct Process* process = *head;
        *head = process->next;

        if (*head != NULL) { //there is another node in the queue after the front
            (*head)->previous = NULL;
        }

        return process;
    } else { //the queue is empty
        return NULL;
    }
}


//signal handler function that prints information about the terminated child process
void child_handler() {

    struct Process terminated_process; //info about the terminated child to be stored here

    if (read(pipe_fd[0], &terminated_process, sizeof(struct Process)) == -1) { //read from pipe
        perror("read from pipe");
        exit(EXIT_FAILURE);
    }

    printf("process id: %d\n", terminated_process.pid); //print info
    printf("path/name: %s\n", terminated_process.name);
    printf("state: %s\n", terminated_process.state);
}

int main(int argc, char* argv[]) { //read the arguments from the command line
    if (argc != 4 || strcmp(argv[1], "RR") != 0) {
        printf("Run with: ./scheduler <algorithm_name> <quantum> <text_file>\n");
        return EXIT_FAILURE;
    }

    printf("\n");

    //open the text file for reading
    FILE* text_file = fopen(argv[3], "r");
    if (text_file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char name[50]; //name of the executable read from the text file

    while (fscanf(text_file, "%s\n", name) == 1) { //read from the text file
        struct Process* process = (struct Process*)malloc(sizeof(struct Process)); //memory allocation
        if (process == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(process->name, name); //initialization of the process descriptor
        process->pid = -1;
        process->enter = time(NULL);
        strcpy(process->state, "NEW");

        enqueue(process, &head); //adding the process descriptor struct to the queue
    }


    if (fclose(text_file) == EOF) { //closing the text file
        perror("fclose");
        return EXIT_FAILURE;
    }



    char *ptr;
    //nanosleep
    struct timespec remaining = {strtol(argv[2], &ptr, 10), 0};
    struct timespec request = {strtol(argv[2], &ptr, 10), 0};


    //creation of a pipe for communication between the parent process and the signal handler
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    //setting up the signal handler struct
    struct sigaction sact;
    sact.sa_handler = child_handler;
    sact.sa_flags = 0;

    //assigning the signal handler function to the SIGCHLD signal
    if (sigaction(SIGCHLD, &sact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    //getting the start time of the program
    time_t start = time(NULL);
    struct Process* temp=(struct Process*)malloc(sizeof(struct Process));

    while (head != NULL) { //the queue still has nodes in it

        int finished = nanosleep(&remaining, &request);
        
        if(finished == 0){

          kill(temp->pid, SIGSTOP);
          strcpy(temp->state, "STOPPED");
          enqueue(temp, &head);
      }
      else
      {
            struct Process* process = dequeue(&head); //remove a node from the queue
            temp=process;  //pointer to descriptor of currently running process

            if(strcmp(temp->state, "STOPPED")==0){
            kill(temp->pid, SIGCONT);
            }
            else{
              strcpy(process->state, "RUNNING"); //update the status of the process to running
              pid_t pid = fork(); //creating the process

              if (pid == -1) {
                  perror("fork");
                  exit(EXIT_FAILURE);

              }

              else if (pid == 0) { //child process

                execl(process->name, process->name, NULL); //run the executable within the process

              }

              else { //parent process

                  strcpy(process->state, "EXITED"); //update the process status to exited

                  process->pid = pid; //update the process id to the correct one

                  //write the process information to the pipe
                  if (write(pipe_fd[1], process, sizeof(struct Process)) == -1) {
                      perror("write to pipe");
                      exit(EXIT_FAILURE);
                  }

                  if (waitpid(pid, NULL, 0) == -1) { //wait for the child process to finish
                      perror("waitpid");
                      exit(EXIT_FAILURE);
                  }

                  time_t exit_time = time(NULL); //get the time in the end of the child process

                  printf("elapsed time: %ld seconds\n\n", exit_time - process->enter);
              }

            }

      }

      time_t end = time(NULL); //get the time at the end of all executions

      printf("total elapsed time: %ld seconds\n\n", end - start);

}

    //close the pipe
    if (close(pipe_fd[1]) == -1) {
        perror("close pipe write end");
        exit(EXIT_FAILURE);
    }

    if (close(pipe_fd[0]) == -1) {
        perror("close pipe read end");
        exit(EXIT_FAILURE);
    }

    printf("scheduler exits\n\n");

    return EXIT_SUCCESS;
}
