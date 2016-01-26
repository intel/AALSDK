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
#include <math.h>
#include <sys/resource.h>

#define NAME "/test"

int main()
{
  mqd_t rx, tx;
  struct rlimit rl;

  //  put_timestamp();

  // Get time limit on cpu
  /* getrlimit (RLIMIT_MSGQUEUE, &rl); */
  /* printf("Default value: %lld\n", (long long int)rl.rlim_cur); */
  /* printf("Maximum value: %lld\n", (long long int)rl.rlim_max); */

  /* printf("Setting rlim_infinity...\n"); */
  /* rl.rlim_cur = RLIM_INFINITY; */
  /* rl.rlim_max = RLIM_INFINITY; */
  /* setrlimit (RLIMIT_AS, &rl); */
  /* setrlimit (RLIMIT_CORE, &rl); */
  /* setrlimit (RLIMIT_CPU, &rl); */
  /* setrlimit (RLIMIT_DATA, &rl); */
  /* setrlimit (RLIMIT_FSIZE, &rl); */
  /* setrlimit (RLIMIT_LOCKS, &rl); */
  /* setrlimit (RLIMIT_MEMLOCK, &rl); */
  /* setrlimit (RLIMIT_MSGQUEUE, &rl); */
  /* setrlimit (RLIMIT_NICE, &rl); */
  /* setrlimit (RLIMIT_STACK, &rl); */

  /* // Message queue */
  /* printf("Setting up message queues...\n"); */
  /* tx = mqueue_create("/test.", O_CREAT|O_WRONLY|O_EXCL); */
  /* rx = mqueue_create("/test.", O_CREAT|O_RDONLY); */

  /* printf ("tx_fd = %d\n", tx); */
  /* printf ("rx_fd = %d\n", tx); */

  /* mq_unlink ("/test"); */

  rx = mq_open( "/test", O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, NULL);
  if (rx == -1)
    {
      perror("RX: mq_open");
      /* ase_error_report ("mq_open", errno, ASE_OS_MQUEUE_ERR); */
      exit(1);
    }
  
  mq_close(rx);
  mq_unlink(NAME);

  printf("\n");
  return 0;
}

