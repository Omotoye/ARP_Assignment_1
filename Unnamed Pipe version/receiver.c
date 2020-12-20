#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#define MAX_QUEUESIZE 1000000

typedef struct
{
    time_t timestamp;
    char g;
    int x;
} message;

typedef struct
{
    time_t total_offset_g1, total_offset_g2, total_latency;
    int g1_total, g2_total;
} compute_data;

int full_queue_handler(int y, pid_t gen_1_pid); // function definition for full queue handler

int main(int argc, char *argv[])
{
    // Converting the file descriptors into integers
    int fil_des_00 = atoi(argv[1]); // to read from Pipe 1
    int fil_des_01 = atoi(argv[2]); // to write to Pipe 1
    int fil_des_10 = atoi(argv[3]); // to read from Pipe 2
    int fil_des_11 = atoi(argv[4]); // to write to Pipe 2
    int fil_des_20 = atoi(argv[5]); // to read from Pipe 3
    int fil_des_21 = atoi(argv[6]); // to write to Pipe 3

    pid_t gen_1_pid = fork();
    if (gen_1_pid == -1)
    {
        perror("gen_1_pid");
        return 2;
    }
    else if (gen_1_pid == 0)
    {
        /*** Generator 1 Process ***/
        if ((execlp("./generator_1", "./generator_1", argv[1], argv[2],
                    argv[3], argv[4], argv[5], argv[6], (char *)NULL)) == -1)
        {
            perror("Exec Generator 1");
        }
        exit(0);
    }
    else
    {
        /*** Reciever Process ***/

        message messages;

        int offset;
        double total_latency = 0;
        int i = 0;
        int y = 0;
        int g1_total = 0;
        int g2_total = 0;
        unsigned long int first_g1_time = 0;
        unsigned long int first_g2_time = 0;
        unsigned long int first_gen_time = 0;
        unsigned long int last_g1_time = 0;
        unsigned long int last_g2_time = 0;
        unsigned long int last_gen_time = 0;
        unsigned long int last_recv_time = 0;
        unsigned long int difference = 0;
        unsigned long int total_offset_g1;
        unsigned long int total_offset_g2;
        int status = 0;

        // Closing unused Pipes
        close(fil_des_01);
        close(fil_des_11);
        close(fil_des_20);

        // Creating the Pipe Selector
        fd_set rfds;
        struct timeval tv;
        int retval;

        tv.tv_sec = 0;
        tv.tv_usec = 1;
        time_t resv_time; // time of message reception

        int rand_fd[2]; // file descriptor array to pick random file descriptor during select()
        rand_fd[0] = fil_des_00;
        rand_fd[1] = fil_des_10;

        while (1)
        {
            full_queue_handler(y, gen_1_pid); // returns 0 if the queue is full and returns 1 when the queue is not yet full

            FD_ZERO(&rfds);
            FD_SET(fil_des_00, &rfds);
            FD_SET(fil_des_10, &rfds);
            struct timeval tmv;
            retval = select(6, &rfds, NULL, NULL, &tv);

            if (retval == -1)
                perror("select()");
            else if (retval)
            {
                y++; // to count how many times data is ready, this is useful for the full_queue function
                if ((FD_ISSET(fil_des_00, &rfds)) && (FD_ISSET(fil_des_10, &rfds)))
                {
                    // Pick a random file descriptor
                    int rand_pick = rand() % (2);
                    if ((read(rand_fd[rand_pick], &messages, sizeof(message))) == -1)
                    {
                        perror("Read Gen 1 or Gen 2");
                    }
                    gettimeofday(&tmv, NULL);
                    resv_time = tmv.tv_usec;
                    if (messages.timestamp > resv_time)
                    {
                        difference = 1000000 - messages.timestamp + resv_time;
                    }
                    else
                    {
                        difference = tmv.tv_usec - (messages.timestamp);
                    }
                    total_latency += difference;
                }
                else if (FD_ISSET(fil_des_00, &rfds))
                {
                    // Read from first pipe
                    if ((read(fil_des_00, &messages, sizeof(message))) == -1)
                    {
                        perror("Read Gen 1");
                    }
                    gettimeofday(&tmv, NULL);
                    resv_time = tmv.tv_usec;
                    if (messages.timestamp > resv_time)
                    {
                        difference = 1000000 - messages.timestamp + resv_time;
                    }
                    else
                    {
                        difference = tmv.tv_usec - (messages.timestamp);
                    }
                    total_latency += difference;
                }
                else if (FD_ISSET(fil_des_10, &rfds))
                {
                    // Read from the second pipe
                    if ((read(fil_des_10, &messages, sizeof(message))) == -1)
                    {
                        perror("Read Gen 2");
                    }
                    gettimeofday(&tmv, NULL);
                    resv_time = tmv.tv_usec;
                    if (messages.timestamp > resv_time)
                    {
                        difference = 1000000 - messages.timestamp + resv_time;
                    }
                    else
                    {
                        difference = tmv.tv_usec - (messages.timestamp);
                    }
                    total_latency += difference;
                }

                // Computing data from messages
                if (i == 0)
                {
                    first_gen_time = messages.timestamp;
                }
                else if (i == MAX_QUEUESIZE - 1)
                {
                    last_gen_time = messages.timestamp;
                    last_recv_time = resv_time;
                }
                if (messages.g == 'A')
                {
                    g1_total++;
                    first_g1_time = last_g1_time;
                    last_g1_time = messages.timestamp;
                    if (first_g1_time > last_g1_time && messages.x != 1)
                    {
                        offset = 1000000 - first_g1_time + last_g1_time;
                    }
                    else if (first_g1_time < last_g1_time && messages.x != 1)
                    {
                        offset = last_g1_time - first_g1_time;
                    }

                    total_offset_g1 += offset;
                }
                else if (messages.g == 'B')
                {
                    g2_total++;
                    first_g2_time = last_g2_time;
                    last_g2_time = messages.timestamp;
                    if (first_g2_time > last_g2_time)
                    {
                        offset = 1000000 - first_g2_time + last_g2_time;
                    }
                    else
                    {
                        offset = last_g2_time - first_g2_time;
                    }
                    total_offset_g2 += offset;
                }
                i++; // for the computing loop
                printf("The message %d from %c was created at %ld and received at %ld\n", messages.x, messages.g, messages.timestamp, resv_time);
            }
            else
            {
                printf("No data within 1 microseconds seconds.\n");
                usleep(10);
            }
            if ((full_queue_handler(y, gen_1_pid))) // Checks if the queue is full so as to stop the infinite loop
            {
                break;
            }
        }

        // Putting computed data in a struct to send to the M Process
        compute_data computed_data;

        computed_data.total_offset_g1 = total_offset_g1;
        computed_data.total_offset_g2 = total_offset_g2;
        computed_data.total_latency = total_latency;
        computed_data.g1_total = g1_total;
        computed_data.g2_total = g2_total;

        // Write the computed data struct on pipe 3 for M process to read
        if ((write(fil_des_21, &computed_data, sizeof(compute_data))) == -1)
        {
            perror("Write Pipe 3");
        }

        // Sending a SIGCONT signal to M process to continue is process
        kill(getppid(), SIGCONT);

        // Closing all used Pipes
        close(fil_des_00);
        close(fil_des_10);
        close(fil_des_21);

        // Waiting for child process
        if (wait(&status) == -1) // Error Management for the wait function
        {
            printf("Generator 1 has terminated with an error code %d\n", status);
        }
    }

    exit(EXIT_SUCCESS);
}

int full_queue_handler(int y, pid_t gen_1_pid)
{
    int ret;
    if (y == MAX_QUEUESIZE)
    {
        kill(gen_1_pid, SIGKILL);
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}
