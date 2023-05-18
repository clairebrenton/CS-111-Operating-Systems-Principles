## UID: 105452312

(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

# A Kernel Seedling

This kernel module creates a virtual file /proc/count that, when read or opened, displays the current number of running processes.

## Building

To build the kernel module, we run the command “make.” This command invokes the “make” utility which reads the Makefile and executes the default target of 

default:
$(MAKE) -C $(KDIR) M=$$PWD modules

The -C flag makes the kernel source directory the current working directory. We see the ‘modules’ target is specified here.

Then the kernel build system will compile the proc_count.c file into a kernel object file proc_count.o and then link it to create a kernel module file proc_count.ko, which is why after we run ‘make’, we see the proc_count.ko file appears in the lab0 directory.

## Running

After building, we can run the kernel module by first loading the module into the kernel by running “sudo insmod proc_count.ko.” We can verify the module is loaded by inspecting lsmod.

Now we read the /proc/count file by running the “cat /proc/count” command to read the virtual /proc/count file, we can expect that this file will show an integer value representing the number of currently running processes.


## Cleaning Up

To remove the kernel module we need to unload it from the kernel with the command “sudo rmmod proc_count”. This command will call the __exit function proc_count_exit in the module which uses the proc_remove() function to remove the /proc/count entry.


## Testing
Version 5.14.8-arch-1 (confirmed by Victor that this is ok).
