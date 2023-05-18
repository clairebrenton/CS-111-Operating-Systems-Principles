## UID: 105452312

(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

# You Spin Me Round Robin

This program implements Round Robin scheduling for a set of processes using the TAILQ data structure and outputs the average waitings time and response time of the processes based on a given quantum.

## Building

You build the program by calling 'make' which compiles the source code into an object file, rr.o, and links them to create the executable 'rr'.

## Running

We can run the command ./rr processes.txt 3. We get the expected output of the average waiting time being 7.00 and the average response time being 2.75.

## Cleaning Up

To clean up the binary files, run 'make clean' which executes the clean target in the Makefile and removes the object file and the executable.

We see in the Makefile that ituses the rm command to remove rr.o and the 'rr' command.
