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
#include <sys/inotify.h>
#include <limits.h>


#define MEM_NAME "/test"
#define MEM_SIZE 1024*1024
#define NUM_INTS MEM_SIZE/sizeof(uint64_t)
#define BASE_NUM 0xcafebabedecafbad

#define MAX_EVENTS  1024
#define LEN_NAME    16
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) 

/////////////////////////////////////////////////////////////////
int  polling;
void sig_callback( int sig )
{
  polling = 0;
}

int main()
{
  int fd;
  void *vbase;
  int ret;
  uint64_t index;
  uint64_t data_found;
  int length;

  fd = shm_open (MEM_NAME, O_CREAT|O_RDWR|O_NONBLOCK, S_IREAD|S_IWRITE);
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

  printf("Testing the event service now...\n");
  int ifd;
  ifd = inotify_init1( IN_NONBLOCK );
  if (ifd == -1) 
    perror("inotify_init");
  
  int wd;
  wd = inotify_add_watch(ifd, "/dev/shm/test", IN_MODIFY);
  perror("inotify_add_watch");
  if (wd == -1)
    {
      printf("Couldn't add watch to\n");
      return -1;
    }
  else
    {
      printf("Watching...");
    }

  int i;
  char buffer[BUF_LEN];

  fd_set watch_set;
  // use select watch list for non-blocking inotify read
  FD_ZERO( &watch_set );
  FD_SET( ifd, &watch_set );


  /* struct pollfd pfd = { fd, POLLIN, 0 }; */
  /* int ret = poll(&pfd, 1, 50);  // timeout of 50ms */
  polling = 1;
  //  signal( SIGINT, sig_callback );
  while(polling)
    {
      printf("Waiting...\n");
      sleep(1);
      i = 0;
      select( ifd+1, &watch_set, NULL, NULL, NULL );
      length = read( fd, buffer, BUF_LEN );  
 
      if ( length < 0 ) {
        perror( "read" );
      }  

      while ( i < length ) 
	{
	  struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	  if ( event->len ) 
	    {
	      if ( event->mask & IN_CREATE) {
		if (event->mask & IN_ISDIR)
		  printf( "The directory %s was Created.\n", event->name );       
		else
		  printf( "The file %s was Created with WD %d\n", event->name, event->wd );       
	      }
	      
	      if ( event->mask & IN_MODIFY) {
		if (event->mask & IN_ISDIR)
		  printf( "The directory %s was modified.\n", event->name );       
		else
		  printf( "The file %s was modified with WD %d\n", event->name, event->wd );       
	      }
	      
	      if ( event->mask & IN_DELETE) {
		if (event->mask & IN_ISDIR)
		  printf( "The directory %s was deleted.\n", event->name );       
	      else
		printf( "The file %s was deleted with WD %d\n", event->name, event->wd );       
	      }      
	      
	      i += EVENT_SIZE + event->len;
	    }
	}
    }
  
  // Deallocate and close
  ret = munmap((void*)vbase, (size_t) MEM_SIZE);
  if(0 != ret) {
    perror("munmap");
  }

  ret = shm_unlink(MEM_NAME);
  if (0 != ret )
    perror("shm_unlink");

  return 0;
}
