// C Program to demonstrate
// use of nanosleep
#include <stdio.h>
#include <time.h>

int main()
{
        struct timespec remaining, request = {2, 0 };

        printf("Taking a nap...\n");
        int response = nanosleep(&request, &remaining);

        if (response == 0) {
                printf("Nap was Successful .\n");
        }
        else {
                printf("Nap was Interrupted.\n");
        }
return 0;
}
