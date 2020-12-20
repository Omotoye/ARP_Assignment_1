#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

typedef struct
{
    time_t timestamp;
    char g;
    int x;
} message; // the struct data type for the message to be sent

pid_t gen_2_Pid; // variable to hold process id of Generator 2

// Signal handler that handles a sigkill signal by killing Generator 1 and Generator 2 process
void handle_sigkill(int sig)
{
    kill(gen_2_Pid, SIGKILL);
    kill((getpid()), SIGKILL);
}

int main(int argc, char *argv[])
{
    // Signal Handler
    signal(SIGKILL, &handle_sigkill);

    int x = 0;          // Message sequence number
    char g = 'A';       // Generator 1 Identifier
    message message_g1; // Declaring an instance of message struct
    int status = 0;     // takes the return value of the child process

    gen_2_Pid = fork();
    if (gen_2_Pid == -1)
    {
        perror("Generator 2");
        return 1;
    }
    else if (gen_2_Pid == 0)
    {
        /*** Generator 2 Process ***/

        if ((execlp("./generator_2", "./generator_2", argv[4], (char *)NULL)) == -1)
        {
            perror("Exec Generator 2");
        }

        exit(-1);
    }
    else
    {
        /*** Generator 1 Process ***/

        int fil_des = open("g2r_fifo", O_WRONLY);
        if (fil_des == -1)
        {
            perror("Open FIFO");
        }

        // Message Generator Loop
        while (1)
        {
            x++;
            struct timeval tv;

            // Message construction
            message_g1.g = g;
            message_g1.x = x;
            gettimeofday(&tv, NULL);          // Increase the sequence number by one every iteration
            time_t current_time = tv.tv_usec; // Returns the current local time in microseconds
            message_g1.timestamp = current_time;

            // Writing on Pipe 1
            if ((write(fil_des, &message_g1, sizeof(message))) == -1)
            {
                perror("Generator 1 write has failed");
            }

            // Delay
            int offset = 10;
            int rand_num = offset + (rand() % (offset + 1)); // Random number from 0 to offset
            usleep(rand_num);                                // Puts the program to sleep in rand_num amount of microseconds
        }

        // Closing Used Pipe
        close(fil_des);

        // Return value from waited process.
        if (wait(&status) == -1) // Error Management for the wait function
        {
            printf("Generator 2 has terminated with an error code %d\n", status);
        }
    }
    return 0;
}
