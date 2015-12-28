#include "ase_common.h"

int main()
{
  session_init();

  struct buffer_t *test;
  test = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  test->memsize = 2*1024*1024;
  allocate_buffer(test);

  sleep(2);
  deallocate_buffer(test);

  session_deinit();

  return 0;
}
