#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>   
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>       
#include <ctype.h>         
#include <mqueue.h>        // Message queue setup
#include <errno.h>         // Error management
#include <signal.h>        // Used to kill simulation             
#include <pthread.h>       // DPI uses a csr_write listener thread
#include <sys/resource.h>  // Used to get/set resource limit
#include <sys/time.h>      // Timestamp generation

#define MEM_NAME "/largemem"
#define MEM_SIZE 32*1024*1024*1024
#define NUM_INTS MEM_SIZE/sizeof(uint64_t)
#define BASE_NUM 0xcafebabedecafbad

int main()
{
#if 1
  int fd;
  void *vbase;
  int ret;
  uint64_t index;
  uint64_t data_found;
  struct stat statbuf;

  fd = shm_open (MEM_NAME, O_CREAT|O_RDWR, S_IREAD|S_IWRITE);
  if(fd < 0) 
    {
      perror("shm_open");
      exit(1);
    }

  vbase = (void *) mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(vbase == (void *) MAP_FAILED) {
      perror("mmap");
      exit(1);
  }

  // Extend memory to required size
  ret = ftruncate(fd, (off_t)MEM_SIZE); 
  if(0 != ret) {
    perror("ftruncate");
  }

  printf("vbase = %p\n", vbase);

  // Memtest - write
  printf("Writing.... ");
  for(index = 0; index < NUM_INTS; index++)
    *((uint64_t*)((uint64_t)vbase + sizeof(uint64_t)*index)) = BASE_NUM + index;
  printf("DONE\n");

  // Memtest - read
  printf("Reading.... ");
  for(index = 0; index < NUM_INTS; index++)
    {
      data_found = *((uint64_t*)((uint64_t)vbase + sizeof(uint64_t)*index));
      if ( data_found != (BASE_NUM + index))
	printf("\nERROR @ index = %016x, data found = %016x", index, data_found);
    }
  printf("DONE\n");
  
  // Deallocate and close
  ret = munmap((void*)vbase, (size_t) MEM_SIZE);
  if(0 != ret) {
    perror("munmap");
  }

  ret = shm_unlink(MEM_NAME);
  if (0 != ret )
    perror("shm_unlink");

#else
  unsigned long value;
  value = HeapMax();
  printf("max heap size = %016x", value);
#endif  
  return 0;
}
