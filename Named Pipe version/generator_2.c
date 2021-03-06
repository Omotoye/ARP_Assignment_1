#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

typedef struct
{
   time_t timestamp;
   char g;
   int x;
} message; // the struct data type for the message to be sent

int main(int argc, char *argv[])
{
   /*** Generator 2 ***/
   message message_g2; // creating an instance of the message struct
   int x = 0;          // Message 2 sequence number
   char g = 'B';

   // Opening fifo to write
   int fil_des = open("g2r_fifo", O_WRONLY);
   if (fil_des == -1){
      perror("FIFO Open");
   }

   // Generator 2 Identifier

   // Message Generator Loop
   while (1)
   {
      x++; // Increase the sequence number by one every iteration
      struct timeval tv;

      // Message construction
      message_g2.g = g;
      message_g2.x = x;
      gettimeofday(&tv, NULL);
      message_g2.timestamp = tv.tv_usec; // localtime in microseconds

      // Writing on Pipe 2
      if ((write(fil_des, &message_g2, sizeof(message))) == -1)
      {
         perror("Generator 2 write has failed");
      }

      // Delay
      int offset = 10;
      int rand_num = offset + (rand() % (offset + 1)); // Random number from 0 to offset
      usleep(rand_num);                                // sleep for rand_num amount of microseconds
   }

   // Closing Used Pipes
   close(fil_des);
   exit(0);
}
