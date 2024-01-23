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


//global variable to store pid of process requesting i/o
pid_t temp;
int flag_running_process;

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


//signal handler function right after request of i/o
void start_io_handler() {
         if (head != NULL){
                flag_running_process=1;
                struct Process* process = dequeue(&head); //remove a node from the queue

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


             if (waitpid(pid, NULL, 0) == -1) { //wait for the child process to finish
               perror("waitpid");
               exit(EXIT_FAILURE);
               }

               printf("process id: %d\n", process->pid); //print info
               printf("path/name: %s\n", process->name);
               printf("state: %s\n", process->state);

           }

           time_t exit_time = time(NULL); //get the time in the end of the child process

           printf("elapsed time: %ld seconds\n\n", exit_time - process->enter);

}
      }



//signal handler function after i/o is completed
void end_io_handler(){
            pid_t pid_io = temp;

            waitpid(pid_io, NULL, WUNTRACED);     //waits for child(i/o completed) to raise(SIGSTOP)

        if(flag_running_process == 1){
                while(waitpid(-1, NULL, WNOHANG)) {
                    sleep(0);
                }
        kill(pid_io, SIGCONT); //tells the child that completed i/o to continue running

        flag_running_process = 0; //flag reset
        }
}

int main(int argc, char* argv[]) { //read the arguments from the command line
    if (argc != 3 || strcmp(argv[1], "FCFS") != 0) {
        printf("Run with: ./scheduler <algorithm_name> <text_file>\n");
        return EXIT_FAILURE;
    }

    printf("\n");

    //open the text file for reading
    FILE* text_file = fopen(argv[2], "r");
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
//assigning the signal handler function to the SIGUSR1 signal
if (signal(SIGUSR1, start_io_handler) == SIG_ERR) {
    perror("signal");
    exit(EXIT_FAILURE);
  }


//assigning the signal handler function to the SIGUSR2 signal
if (signal(SIGUSR2, end_io_handler) == SIG_ERR) {
    perror("signal");
    exit(EXIT_FAILURE);
  }

//getting the start time of the program
time_t start = time(NULL);

while (head != NULL) { //the queue still has nodes in it

                struct Process* process = dequeue(&head); //remove a node from the queue

                strcpy(process->state, "RUNNING"); //update the status of the process to running

                pid_t pid = fork(); //creating the process
                temp = pid;
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


                        if (waitpid(pid, NULL, 0) == -1) { //wait for the child process to finish
                                perror("waitpid");
                                exit(EXIT_FAILURE);
                                }
        printf("process id: %d\n", process->pid); //print info
        printf("path/name: %s\n", process->name);
        printf("state: %s\n", process->state);

}

                        time_t exit_time = time(NULL); //get the time in the end of the child process

                        printf("elapsed time: %ld seconds\n\n", exit_time - process->enter);

        }

time_t end = time(NULL); //get the time at the end of all executions

printf("total elapsed time: %ld seconds\n\n", end - start);



 printf("scheduler exits\n\n");

 return EXIT_SUCCESS;
}
