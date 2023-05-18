## Hash Hash Hash
This lab implements two different multi-threading techniques in hash table manipulation, where a single global mutex is used to protect the entire hash table in v1, and a mutex is used for each hash table entry in v2, providing a finer-grained locking and thus allowing for multiple threads to modify the hash table concurrently.

## Building
To build, run the command 'make' which executes the 'all' default target in the Makefile, and creates the hash-table-tester executable by linking the source files with the object files. 

## Running
./hash-table-tester -t 4 -s 50000
Generation: 34,900 usec
Hash table base: 200,139 usec
  - 0 missing

## First Implementation
In version 1, I added a mutex in the hash_table_v1 struct definition, using a global lock for the entire hash table. In the hash_table_v1_add_entry function, I added the mutex lock at line 56 before the search for an existing entry, the update of its value, and the addition of an entry. This protects the entire critical section where changes to the hash table occur, ensuring that no two threads can modify the table at the same time and cause data races. I unlock the mutex on line 62 after updating an entry or adding an entry, allowing for othe threads to acquire the lock once the current thread has finished its actions. I also unlock at line 81 after the updating takes place. I then destroy the mutex at the end to free its space.

### Performance

./hash-table-tester -t 4 -s 50000
Generation: 35,071 usec
Hash table base: 202,228 usec
  - 0 missing
Hash table v1: 434,138 usec
  - 0 missing

Version 1 is slower than the base implementation due to the overhead of adding a mutex. Additionally, a single global mutex is used for the entire hash table, meaning only one thread can modify the table at a time, so other threads have to wait. Although the code in version 1 is thread safe, there is limited concurrency due to the coarse-grain lock and waiting times of the other threads.

## Second Implementation
In version 2, I added a mutex in the hash_table_entry struct, using a finer-grain lock for each hash table entry. I also initialized the mutex for each entry in hash_table_v2_create. In hash_table_v2_add_entry, I added a lock at line 101 before the search/update/adding of an entry. This protects the ciritcal section where modifications to the hash table occur. It also allows other threads to work on different entries, improving concurrency. I then added an unlock on line 107 after changes to an entry were made, allowing other threads to aquire the lock to make changes to that particular hash table entry. I then destroy the mutex for each entry in the hash_table_v2_destroy function at the end to clear the space.

### Performance
./hash-table-tester -t 4 -s 50000
Generation: 35,071 usec
Hash table base: 202,228 usec
  - 0 missing
Hash table v1: 434,138 usec
  - 0 missing
Hash table v2: 63,238 usec
  - 0 missing

This time the speed up is approximately just over three times faster, similar to the number of cores (4). This speed up is owed-to the finer grain lock. Each entry in the hash table has its own mutex, meaning multiple threads can modify different entries in the hash table at the same time without interfering with eachother. This leads to much better performance and high concurrency.

## Cleaning up
To remove the binary files, run the command 'make clean' which removes the object files and the executable by executing the 'clean' target in the Makefile. 
