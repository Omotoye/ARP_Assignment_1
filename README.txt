Designed By Omotoye Shamsudeen Adekoya

Documentation for the ARP Assignment 1

Outline
~~~~~~~~

1--> Design Choices
2--> How to Compile
3--> Result Discussion

                                 _____________________
                                 |  DESIGN CHOICES    |   
                                 ---------------------
                                 
                                       [G1]
                                           \
                                            \
                                             \[R]------>[M]
                                             /
                                            /
                                           /
                                       [G2]     

*In the program, two generator processes G1 and G2 are supposed to generate quasi periodic messages to send to 
 process R. Process R will then computed the message and send it to process M for Outputing, making process M the 
 last process to terminal. It is for this reason that i have chosen process M to be the Master Process. The 
 process M then forks a child process which represents the process R, process R forks process G1 and process G1
 forks process G2. The process tree looks something like this (M)--->(R)--->(G1)--->(G2).
*For the pipes, in the unnamed pipe version, three unammed pipes are created in the M process and the file 
 descriptors required by each of the processes are then passed down the heirarchy during the exec process as 
 command line arquments. In the Named pipe version, the only change from the previous architecture is the 
 communication between the Gi processes and the R process. Instead of using two unnamed pipes for the 
 communication, one named pipe was created at the R process and then open for READ_ONLY, the created 
 FIFO(Named Pipe) was opened by each of the Gi processes for WRITE_ONLY.
*Another thing to note is that the process computes its time in microseconds and after 1000000 microseconds
 its 1 second then the microseconds starts from 1 again. The program is going to run for well over 1 second
 which means multiple cycle of microseconds. This has to be watched out for in the latency, offset delay and
 Bandwidth computation. My solution to this is a conditional statement that checks if creation time is higher than 
 reception time. If this is TRUE it means another cycle just started and the difference in time will be calculated 
 as difference = (1000000 - creation_time) + reception time 
*How the program will run: the program will print each message received from the R process in the format
 "The message %d from %c was created at %ld and received at %ld\n", where %d is for the sequence number of
 the message, %c is a char identifying the message, and %ld is for the timestamp of the message. 
 Initially i left the print out of the program, and put just a single print statement that prints, "please wait
 while the program computes", but the program takes a while to finish so i thought it would be a good idea to 
 have a feedback while the program is running so you know the program is working. 
 After the loop is done and all the message has been received and computed by the R process, the R process 
 then sends a kill signal to the G1 process which has a signal handler to handler the sigkill signal, in the 
 handler function of the G1, there are two statements one sends a kill signal to kill the child process which 
 is G2 and the other kills the current pid which is itself.
 After the signal has been handled the R process sends all the computed data to the M process through a pipe
 by the way of struct and the M process does some extra work on the data and Outputs it. This means when you
 run the program you'll see a lot of messages generated and after that the M_process then output the final 
 computation of all the messages. The M process also keeps a log of the computed output, the messages generated 
 is not included in the log file, only the final output is logged.

                                
                                _______________________
                                |   HOW TO COMPILE    |
                                -----------------------

---> to compile the M_Process.c
gcc M_Process.c      (this will save the executable as a.out, which is ok since it's the master process)
---> to compile the receiver.c 
gcc receiver.c -o receiver (the naming of the file MUST NOT BE CHANGED, this is because the executable will
be exec from it's parent process with this name, if the name changes, exec will fail!!!)
---> to compile the generator_1.c 
gcc generator_1.c -o generator_1 (same rule apply)
---> to compile the generator_2.c
gcc generator_2.c -o generator_2 (same rule apply)

run ./a.out  and the process will start execution 

                                _____________________________
                                |    RESULT DISCUSSION      |
                                -----------------------------   
---> Unnamed Pipe Version
Three test was done for different timeout conditions and their average was taken

* test with Gi offset = 10, select timeout = 1, timeout if no data = 10
-> total offset delay in G1: 70000000(microseconds) same for G2 
-> latency: 55
-> Bandwidth: 310000(bit/s)

* test with Gi offset = 10, select timeout = 5, timeout if no data = 20
-> total offset delay in G1: 75000000(microseconds) same for G2 
-> latency: 140
-> Bandwidth: 110000(bit/s)

* test with Gi offset = 20, select timeout = 1, timeout if no data = 10
-> total offset delay in G1: 80000000(microseconds) same for G2 
-> latency: 45
-> Bandwidth: 400000(bit/s)

* test with Gi offset = 20, select timeout = 5, timeout if no data = 20
-> total offset delay in G1: 85000000(microseconds) same for G2 
-> latency: 60
-> Bandwidth: 250000(bit/s)

---> Named Pipe Version
Three test was done for different timeout conditions and their average was taken

* test with Gi offset = 10
-> total offset delay in G1: 450000000(microseconds) same for G2 
-> latency: 25
-> Bandwidth: 700000(bit/s)

* test with Gi offset = 20
-> total offset delay in G1: 470000000(microseconds) same for G2 
-> latency: 22
-> Bandwidth: 750000(bit/s)


                          ****** Discussion *********

From the above result, the version with the Named pipe has a significantly higher Bandwidth than 
any of the versions of the Unnamed pipe but falls short in the delay, it take x5 times longer to 
receiver all the messages as compared to the version with Unnamed pipe because of the high delay
from working with different process through one channel. 
Another point to discuss is the effect of the changes in timeout. The higher the wait time when 
no data is available the higher the latency, but it all depends on the offset delay from the 
generators, if the offset delay in the generator is high enough to compensate for the timeout delay
then the latency will be reduced significantly.

                          ******** Conclusion ********** 

The version with the single Named pipe will be best suited for a system that require very 
quick response to a process

The version with the multiple Unnamed pipe will be best suited for a system that requires 
a simultaneous response to high number of processes.






You can now find this project on my GitHub page that i wil add here later :)
I want to make a change so i can test out my git status code

More random thing to be added just to check the diff command if it also print
changes from more than one file
more things to add
