#include "ase_common.h"

int main()
{
  struct mmio_t *mmio, *copy_mmio;

  mmio = (struct mmio_t *)ase_malloc(sizeof(struct mmio_t));
  copy_mmio = (struct mmio_t *)ase_malloc(sizeof(struct mmio_t));

  char str[ASE_MQ_MSGSIZE];
  memset(str, '\0', ASE_MQ_MSGSIZE);

  mmio->type = MMIO_WRITE;
  mmio->width = MMIO_WIDTH_64;
  mmio->addr = 0x1000;
  mmio->data = 0xDECAFBADCAFEBABE;
  mmio->resp_en = 0;

  memcpy(str, (char*)mmio, sizeof(mmio_t));

  memcpy(copy_mmio, (mmio_t *)str, sizeof(mmio_t));

  printf("copy_mmio => %x %d %x %llx %d\n", 
	 copy_mmio->type,
	 copy_mmio->width,
	 copy_mmio->addr,
	 copy_mmio->data,
	 copy_mmio->resp_en
	 );

  return 0;
}
