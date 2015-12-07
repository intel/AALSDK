#include "ase_common.h"

#define NLB_DSM_BASEL_OFF 0x1A00
#define NLB_DSM_BASEH_OFF 0x1A04

int main()
{

  uint32_t *data32;
  uint64_t *data64;
  uint32_t dsm_base_addr, dsm_base_addrl, dsm_base_addrh;

  session_init();

  struct buffer_t *dsm;
  dsm  = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  dsm->memsize  = 2*1024*1024;
  allocate_buffer(dsm);
  dsm_base_addr = (uint64_t)dsm->fake_paddr;
  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;

  mmio_write32(NLB_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  mmio_write32(NLB_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);


  /* mmio_write32(0x1a00, 0xCAFEBABE); */

  mmio_read32(0x1a00, data32);

  sleep(1);

  /* mmio_write64(0x1a00, 0xFEEDFACEDEBAFCAD); */

  mmio_read64(0x1a00, data64);

  sleep(1);

  session_deinit();

  return 0;
}
