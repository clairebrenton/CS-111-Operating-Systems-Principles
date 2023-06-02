## UID: 105452312

(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

# Hey! I'm Filing Here

This code implements a basic ext2 filesystem image with a root directory, a lost+found directory, and hello-world file, and a hello symbolic link.

## Building

We build the program by running make which creates the 'ext2-create' executable

## Running

To compile, run the executable ./ext2-create which creates the cs111-base.img file by calling various functions to create the components of the filesystem and populates it with the necessary data to represent the ext2 file system. 

To mount, we create a 'mnt' directory with 'mkdir mnt'. Then call 'sudo mount -o loop cs111-base.img mnt' which mounts the directory and allows us to investigate the contents. 


The output of 'ls -ain' is as follows:
total 7
     2 drwxr-xr-x 3    0    0 1024 May 31 20:11 .
942145 drwxr-xr-x 3 1000 1000 4096 May 31 20:11 ..
    13 lrw-r--r-- 1 1000 1000   11 May 31 20:11 hello -> hello-world
    12 -rw-r--r-- 1 1000 1000   12 May 31 20:11 hello-world
    11 drwxr-xr-x 2    0    0 1024 May 31 20:11 lost+found


## Cleaning up

To unmount, we use the command 'sudo umount mnt'. To remove the directory we use 'rmdir mnt' and then to clean up all binary files run 'make clean'.

Explain briefly how to unmount the filesystem, remove the mount directory, and
clean up all binary files.
