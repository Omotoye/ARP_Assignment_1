#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

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

    // Converting the file descriptors into integers
    int fil_des_00 = atoi(argv[1]); // to read from Pipe 1
    int fil_des_01 = atoi(argv[2]); // to write to Pipe 1
    int fil_des_10 = atoi(argv[3]); // to read from Pipe 2
    int fil_des_11 = atoi(argv[4]); // to write to Pipe 2
    int fil_des_20 = atoi(argv[5]); // to read from Pipe 3
    int fil_des_21 = atoi(argv[6]); // to write to Pipe 3

    gen_2_Pid = fork();
    if (gen_2_Pid == -1)
    {
        perror("Generator 2");
        return 1;
    }
    else if (gen_2_Pid == 0)
    {
        /*** Generator 2 Process ***/

        // Closing unused pipes
        close(fil_des_00);
        close(fil_des_01);
        close(fil_des_10);
        close(fil_des_20);
        close(fil_des_21);

        if ((execlp("./generator_2", "./generator_2", argv[4], (char *)NULL)) == -1)
        {
            perror("Exec Generator 2");
        }

        exit(-1);
    }
    else
    {
        /*** Generator 1 Process ***/

        // Closing Unused Pipes
        close(fil_des_00);
        close(fil_des_10);
        close(fil_des_11);
        close(fil_des_20);
        close(fil_des_21);

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
            if ((write(fil_des_01, &message_g1, sizeof(message))) == -1)
            {
                perror("Generator 1 write has failed");
            }

            // Delay
            int offset = 10;
            int rand_num = offset + (rand() % (offset + 1)); // Random number from 0 to offset
            usleep(rand_num);                                // Puts the program to sleep in rand_num amount of microseconds
        }

        // Closing Used Pipe
        close(fil_des_01);

        // Return value from waited process.
        if (wait(&status) == -1) // Error Management for the wait function
        {
            printf("Generator 2 has terminated with an error code %d\n", status);
        }
    }
    return 0;
}
