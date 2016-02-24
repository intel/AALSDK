#include "ase_common.h"

int main()
{
  session_init();

  struct buffer_t *test;
  test = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  test->memsize = 2*1024*1024;

  while(1)
    {
      allocate_buffer(test, NULL);
      usleep(10000);
      deallocate_buffer(test);
      usleep(10000);
    }

  session_deinit();

  return 0;
}
