#include "ase_common.h"

int main()
{
  session_init();

  uint64_t *data_l, *data_h;

  mmio_read64(0x008, data_l);
  mmio_read64(0x010, data_h);

  printf("AFUID = %llx %llx", 
	 (unsigned long long)*data_h, 
	 (unsigned long long)*data_l);

  session_deinit();

  return 0;
}
