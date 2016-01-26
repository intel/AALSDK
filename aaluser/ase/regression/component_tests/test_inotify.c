#include <stdio.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <limits.h>


// hard coded directory and file to watch. don't do this in production code
#define DIR_TO_WATCH  "/dev/shm"
#define FILE_TO_WATCH "/dev/shm/test"

#define EVENT_SIZE (sizeof (struct inotify_event))

// define large enough buffer
#define EVENT_BUFFER_LENGTH (1024 * EVENT_SIZE + NAME_MAX + 1)

void print_event(struct inotify_event *event) {

  if (event->mask & IN_CREATE)
    printf("file created in directory\n");
  if (event->mask & IN_DELETE)
    printf("file deleted in directory\n");
  if (event->mask & IN_ACCESS)
    printf("file accessed\n");
  if (event->mask & IN_MODIFY)
    printf("file accessed\n");
  if (event->mask & IN_CLOSE)
    printf("file closed after reading or writing \n");
  if (event->mask & IN_OPEN)
    printf("file opened\n");

  if (event->len)
    printf("name: %s\n", event->name);

}

int main(int argc, char** argv) {

  int notify_fd;
  int watch_fd;
  long input_len;
  char *ptr;
  char buffer[EVENT_BUFFER_LENGTH];
  struct inotify_event *event;

  notify_fd = inotify_init();
  if (notify_fd < 0) {
    perror("cannot init inotify");
    exit(EXIT_FAILURE);
  }

  watch_fd = inotify_add_watch(notify_fd, DIR_TO_WATCH, IN_CREATE | IN_DELETE);
  if (watch_fd < 0) {
    perror("cannot add directory");
    exit(EXIT_FAILURE);
  }
  // watch_fd = inotify_add_watch(notify_fd, FILE_TO_WATCH, IN_ACCESS | IN_CLOSE | IN_OPEN);
  watch_fd = inotify_add_watch(notify_fd, FILE_TO_WATCH, IN_ACCESS | IN_MODIFY);
  if (watch_fd < 0) {
    perror("cannot add file");
    exit(EXIT_FAILURE);
  }

  while (1) 
    {
      input_len = read(notify_fd, buffer, EVENT_BUFFER_LENGTH);
      if (input_len <= 0) 
	{
	  perror("error reading from inotify fd");
	  exit(EXIT_FAILURE);
	}
      
      ptr = buffer;
      while (ptr < buffer + input_len) 
	{
	  event = (struct inotify_event *) ptr;
	  print_event(event);
	  ptr += sizeof (struct inotify_event) +event->len;
	}
    }
  
  exit(EXIT_SUCCESS);
}

