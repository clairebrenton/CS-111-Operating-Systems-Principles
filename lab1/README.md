## UID: 105452312
(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

## Pipe Up

My program takes a list of command-line arguments and takes the output of each process and redirects it to the input of the following process through setting up in-memory buffers and managing the process’s file descriptors accordingly.

## Building

You build the program by calling ‘make’ which compiles the source code into an object file, pipe.o, and links them to create the executable ‘pipe.’

## Running

We can run the command “./pipe ls head”. ls lists files in the current directory, and its output will be piped to “head” which prints the first 10 lines from ls.

The expected output is the first 10 lines, but since there are only 8 lines in the ls output we should only see the 8 lines, each on a newline.

We get the expected output of:

Makefile
pipe
pipe.c
pipe.c~
pipe.o
__pycache__
README.md
test_lab1.py

## Cleaning up

To clean up binary files, run ‘make clean’ which executes the clean target in the Makefile and removes the object file and the executable.

We see in the Makefile that it uses the rm command to remove ${OBJS} (pipe.o) and ‘pipe.’
