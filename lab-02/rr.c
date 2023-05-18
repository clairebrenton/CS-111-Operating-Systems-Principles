#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers; // this means 

  /* Additional fields here */
  u32 time_left;
  u32 last_wait; // Time it began waiting (resets after every execution period)
  bool response_time_calculated;
  u32 index;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

// helper function
i32 arrival_sorting(const void* a, const void* b){
  const struct process* p1 = (const struct process *) a;
  const struct process* p2 = (const struct process *) b;
  i32 difference = p2->arrival_time - p1->arrival_time;
  if (difference != 0){
    return difference;
  }
  else{
    return p2->index - p1->index; }
}


int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */

  if (quantum_length <= 0)
    exit(EINVAL);
    
  for (u32 i = 0; i < size; i++){
    data[i].index = i;
  }
  qsort(data, size, sizeof(struct process), arrival_sorting);

  u32 num_finished_processes = 0;
  u32 current_time = 0; 
  
  bool *arrived = (bool *) malloc(size * sizeof(bool));
  for (u32 i = 0; i < size; i++){
    arrived[i] = false;
  }
  
  struct process_list to_be_added;
  TAILQ_INIT(&to_be_added);

  while(true){
    i32 time_to_run = 1; 
    for (i32 i = 0; i < size; i++){
      if (data[i].arrival_time <= current_time && !arrived[i]){
        arrived[i] = true;
        data[i].time_left = data[i].burst_time;
        data[i].last_wait = data[i].arrival_time;
        data[i].response_time_calculated = false;
        TAILQ_INSERT_HEAD(&to_be_added, &data[i], pointers);
      }}
    while (!TAILQ_EMPTY(&to_be_added)){
      struct process *current = TAILQ_FIRST(&to_be_added);
      TAILQ_REMOVE(&to_be_added, current, pointers);
      TAILQ_INSERT_TAIL(&list, current, pointers);}

    if (!TAILQ_EMPTY(&list)){
      struct process *current = TAILQ_FIRST(&list);
      TAILQ_REMOVE(&list, current, pointers);
      
     // fprintf(stderr, "Running program %d\n The current time is %d\n The remaining time is %d\n", 
     // current->pid, current_time, current->time_left);

      if (current->time_left < quantum_length){
        time_to_run = current->time_left;}
      else{
        time_to_run = quantum_length;}
      
      if (!current->response_time_calculated){
        total_response_time += current_time - current->arrival_time;
        current->response_time_calculated = true;}
      
      total_waiting_time += current_time - current->last_wait;
      current->last_wait = current_time + time_to_run;
      current->time_left -= time_to_run; 
      
      if (current->time_left > 0){
        TAILQ_INSERT_TAIL(&to_be_added, current, pointers); }
      else{
        num_finished_processes++;
     }}
  
      current_time += time_to_run;
       if (num_finished_processes == size){
      break;}
 }
  free(arrived);

  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}


