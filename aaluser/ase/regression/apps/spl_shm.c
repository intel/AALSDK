#include "ase_common.h"

int main()
{
  struct buffer_t *dsm, *afu_cxt; // , *spl_pt, *spl_cxt;

  uint64_t *afu_vbase;

  dsm     = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  afu_cxt = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  /* spl_pt  = (struct buffer_t *)malloc(sizeof(struct buffer_t)); */
  /* spl_cxt = (struct buffer_t *)malloc(sizeof(struct buffer_t)); */

  // Allocate memory
  dsm->memsize     = 64*1024;
  afu_cxt->memsize = 1024; // 8*1024*1024;

  uint32_t dsm_base_addr, dsm_base_addrl, dsm_base_addrh;
  uint32_t *dsm_afuid_addr;

  allocate_buffer(dsm);
  allocate_buffer(afu_cxt);

  afu_vbase = (uint64_t *) afu_cxt->vbase;
  // Memset to '0'
  memset ((void*)dsm->vbase, 0, dsm->memsize);
  memset ((void*)afu_cxt->vbase, 0, afu_cxt->memsize);

  /*
   * Set up page table & context
   */
  spl_driver_reset(dsm);
  spl_driver_dsm_setup(dsm);
  spl_driver_afu_setup(dsm);
  printf("Building SPL page table... ");
  setup_spl_cxt_pte (dsm, afu_cxt);

  afu_vbase[1] = (uint64_t)afu_vbase + 0x80;
  afu_vbase[2] = (uint64_t)afu_vbase + 0xc0;
  afu_vbase[3] = 1;

  afu_vbase[16] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[17] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[18] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[19] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[20] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[21] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[22] = 0xAFAFAFAFAFAFAFAF;
  afu_vbase[23] = 0xAFAFAFAFAFAFAFAF;
  
  afu_vbase[24] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[25] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[26] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[27] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[28] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[29] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[30] = 0xBEBEBEBEBEBEBEBE;
  afu_vbase[31] = 0xBEBEBEBEBEBEBEBE;

  spl_driver_start((uint64_t*)afu_cxt->vbase);

  while (afu_vbase[8] == 0);

  // CSR setup 
  //  while(1);

  // Deallocate buffers
  deallocate_buffer(dsm);
  deallocate_buffer(afu_cxt);

  return 0;
}


