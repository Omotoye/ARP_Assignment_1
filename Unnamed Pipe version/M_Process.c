#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>
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

int main(int argc, char *argv[])
{
    int status = 0; // to take the return value of the child process

    //Pipe Creation
    int fil_des[3][2];
    for (int i = 0; i < 3; i++)
    {
        if ((pipe(fil_des[i])) == -1)
        {
            perror("Pipe Creation");
            exit(EXIT_FAILURE);
        }
    }

    // Converting the file descriptors to strings
    char fil_des_00[2];
    char fil_des_01[2];
    char fil_des_10[2];
    char fil_des_11[2];
    char fil_des_20[2];
    char fil_des_21[2];
    sprintf(fil_des_00, "%d", fil_des[0][0]);
    sprintf(fil_des_01, "%d", fil_des[0][1]);
    sprintf(fil_des_10, "%d", fil_des[1][0]);
    sprintf(fil_des_11, "%d", fil_des[1][1]);
    sprintf(fil_des_20, "%d", fil_des[2][0]);
    sprintf(fil_des_21, "%d", fil_des[2][1]);

    pid_t receiver_pid = fork();
    if (receiver_pid == -1)
    {
        perror("Fork: Receiver");
    }
    else if (receiver_pid == 0)
    {
        /*** Receiver Process ***/

        if ((execlp("./receiver", "./receiver", fil_des_00, fil_des_01,
                    fil_des_10, fil_des_11, fil_des_20, fil_des_21, (char *)NULL)) == -1)
        {
            perror("Exec Receiver");
        }

        exit(0);
    }
    else
    {
        /*** M Process ***/

        kill(getpid(), SIGSTOP); // Stop the process to wait for data to be ready from the receiver

        // Closing Pipes that are not in use
        close(fil_des[0][0]);
        close(fil_des[0][1]);
        close(fil_des[1][0]);
        close(fil_des[1][1]);
        close(fil_des[2][1]);

        // Creating a file for logging Output Data
        FILE *filePointer;
        filePointer = fopen("log.txt", "w");
        int tot_mes_resc = MAX_QUEUESIZE;

        // Pipe Use
        compute_data computed_data;

        if ((read(fil_des[2][0], &computed_data, sizeof(compute_data))) == -1)
        {
            perror("Pipe 3 read");
        }

        printf("\nBelow is the date analyzes for the whole process\n");
        fprintf(filePointer, "\nBelow is the date analyzes for the whole process\n");
        printf("======================================================\n\n");
        fprintf(filePointer, "======================================================\n\n");
        printf("Number of G1 Cycle: %d\n", computed_data.g1_total);
        fprintf(filePointer, "Number of G1 Cycle: %d\n", computed_data.g1_total);
        printf("Number of G2 Cycle: %d\n", computed_data.g2_total);
        fprintf(filePointer, "Number of G2 Cycle: %d\n", computed_data.g2_total);
        printf("Number of Messages Received: %d\n", tot_mes_resc);
        fprintf(filePointer, "Number of Messages Received: %d\n", tot_mes_resc);
        printf("Offset delay in G1 Cycle(usec): %ld\n", computed_data.total_offset_g1);
        fprintf(filePointer, "Offset delay in G1 Cycle(usec): %ld\n", computed_data.total_offset_g1);
        printf("Offset delay in G2 Cycle(usec): %ld\n", computed_data.total_offset_g2);
        fprintf(filePointer, "Offset delay in G2 Cycle(usec): %ld\n", computed_data.total_offset_g2);
        printf("Average delay(latency) between generation and reception of messages(usec): %d\n", (int)(computed_data.total_latency / tot_mes_resc));
        fprintf(filePointer, "Average delay(latency) between generation and reception of messages(usec): %d\n", (int)(computed_data.total_latency / tot_mes_resc));
        printf("Estimated bandwidth between G and R(bit/s): %ld", (tot_mes_resc * sizeof(message) / ((computed_data.total_latency) / MAX_QUEUESIZE)));
        fprintf(filePointer, "Estimated bandwidth between G and R(bit/s): %d", (int)(tot_mes_resc * sizeof(message) / ((computed_data.total_latency) / MAX_QUEUESIZE)));

        close(fil_des[2][0]);

        // Return value from child process.
        if (wait(&status) == -1) // Error Management for the wait function
        {
            printf("The Receiver has terminated with an error code %d\n", status);
        }
    }
    return 0;
}
