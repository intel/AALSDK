#include "ase_common.h"

#define NLB_DSM_BASEL_OFF 0xA00
#define NLB_DSM_BASEH_OFF 0xA04

#define NLB_v1_1_H         0x0000000000000000
#define NLB_v1_1_L         0x0000000011100181

//                                 Byte Offset              Attribute    Width   Comments
#define      DSM_AFU_ID            0                     // RO           32b     non-zero value to uniquely identify the AFU
#define      DSM_STATUS            0x40                  // RO           512b    test status and error info

#define ENABLE_ALLOC

#define SPL_DSM_BASEL_OFF 0x910
#define SPL_DSM_BASEH_OFF 0x914
#define SPL_CXT_BASEL_OFF 0x918
#define SPL_CXT_BASEH_OFF 0x91c
#define SPL_CH_CTRL_OFF   0x920

#define AFU_DSM_BASEL_OFF 0xA00
#define AFU_DSM_BASEH_OFF 0xA04
#define AFU_CXT_BASEL_OFF 0xA08
#define AFU_CXT_BASEH_OFF 0xA0c


// Populate SPL page table
void create_spl_pagetable_and_context(struct buffer_t *afu_cxt, struct buffer_t *spl_pt, struct buffer_t *spl_cxt) 
{
  uint64_t num_2mb_chunks;
  uint64_t *spl_pt_addr;
  uint64_t afu_cxt_2mb_align;
  int ii;
  uint64_t *spl_cxt_vaddr;

  printf("afu_cxt_size = %d\n", afu_cxt->memsize);
  printf("chunk size   = %d\n", CCI_CHUNK_SIZE);
  
  /*
   * Create Page table
   */
  // Calculate number of 2MB chunks
  num_2mb_chunks = afu_cxt->memsize /(2*1024*1024);
  printf("Num chunks = %d\n", num_2mb_chunks);

  // Allocate SPL page table
  spl_pt->memsize = 64*num_2mb_chunks;
  allocate_buffer(spl_pt);

  // Calculate SPL_PT address and AFU_CXT 2MB bounds
  for(ii = 0; ii < num_2mb_chunks; ii = ii + 1) 
    {
      afu_cxt_2mb_align = (uint64_t)afu_cxt->fake_paddr + (uint64_t)(ii*CCI_CHUNK_SIZE);
      spl_pt_addr       = (uint64_t*)((uint64_t)spl_pt->vbase + CL_BYTE_WIDTH*ii);
      *spl_pt_addr      = afu_cxt_2mb_align;
    }  

  /*
   * SPL context
   */  
  spl_cxt->memsize = 4096; // 2 cache lines // but allocating 4 KB
  allocate_buffer(spl_cxt);
  spl_cxt_vaddr = (uint64_t*)spl_cxt->vbase;
  spl_cxt_vaddr[0] = spl_pt->fake_paddr;
  spl_cxt_vaddr[1] = afu_cxt->vbase;
  spl_cxt_vaddr[2] = (uint64_t)(num_2mb_chunks << 32) | 0x1;
  spl_cxt_vaddr[3] = 1;

}

// main
int main()
{
  struct buffer_t *dsm, *afu_cxt, *spl_pt, *spl_cxt;

  dsm     = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  afu_cxt = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  spl_pt  = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  spl_cxt = (struct buffer_t *)malloc(sizeof(struct buffer_t));

  // Allocate memory
  dsm->memsize     = 64*1024;
  afu_cxt->memsize = 8*1024*1024;

  uint32_t dsm_base_addr, dsm_base_addrl, dsm_base_addrh;
  uint32_t *dsm_afuid_addr;

  allocate_buffer(dsm);
  allocate_buffer(afu_cxt);

  // Print buffer information
  ase_buffer_info(dsm);  
  ase_buffer_info(afu_cxt);

  // Memset to '0'
  memset ((void*)dsm->vbase, 0, dsm->memsize);
  memset ((void*)afu_cxt->vbase, 0, afu_cxt->memsize);

  /*
   * Set up page table & context
   */
  printf("Building SPL page table... ");
  create_spl_pagetable_and_context(afu_cxt, spl_pt, spl_cxt);
  printf(" DONE\n");

  // AFU-handshake
  dsm_base_addr = (uint64_t)dsm->fake_paddr;
  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;
  dsm_afuid_addr  = (uint32_t*)(dsm->vbase + DSM_AFU_ID);

  printf("Setting address registers...\n");
  csr_write(SPL_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(SPL_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);

  csr_write(AFU_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(AFU_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);

  while(*dsm_afuid_addr==0){ };
  /* if( *dsm_afuid_addr==(uint32_t)NLB_v1_1_L  */
  /*     && *(dsm_afuid_addr+0x1)==(uint32_t)(NLB_v1_1_L>>32) */
  /*     && *(dsm_afuid_addr+0x2)==(uint32_t)(NLB_v1_1_H) */
  /*  && *(dsm_afuid_addr+0x3)==(uint32_t)(NLB_v1_1_H>>32) */
  /*  ) */
  /* { */
  /*       printf("\n*** SPL AFU DISCOVERED ***\n"); */
  /* } else */
  /* { */
  /*       printf("\n*** Invalid AFU %x %x %x %x***\n", *(dsm_afuid_addr+0x3), *(dsm_afuid_addr+0x2), *(dsm_afuid_addr+0x1), *dsm_afuid_addr); */
  /*       printf("\n*** Expected ID %lx %lx***\n", NLB_v1_1_H, NLB_v1_1_L); */
  /*       return 1; */
  /* } */
  usleep(50000);
  printf("\n*** AFU ID %x %x %x %x***\n", *(dsm_afuid_addr+0x3), *(dsm_afuid_addr+0x2), *(dsm_afuid_addr+0x1), *dsm_afuid_addr);


  // CSR setup 
  csr_write(SPL_DSM_BASEL_OFF, dsm->fake_paddr);
  csr_write(SPL_DSM_BASEH_OFF, (dsm->fake_paddr >> 32));
  csr_write(SPL_CXT_BASEL_OFF, spl_cxt->fake_paddr);
  csr_write(SPL_CXT_BASEH_OFF, (spl_cxt->fake_paddr >> 32));
  csr_write(AFU_DSM_BASEL_OFF, dsm->fake_paddr);
  csr_write(AFU_DSM_BASEH_OFF, (dsm->fake_paddr >> 32));
  csr_write(AFU_CXT_BASEL_OFF, afu_cxt->vbase);
  csr_write(AFU_CXT_BASEH_OFF, (afu_cxt->vbase >> 32));
  csr_write(SPL_CH_CTRL_OFF, 0x2);

  while(1);

#ifdef ENABLE_ALLOC
  // Deallocate buffers
  deallocate_buffer(dsm);
  deallocate_buffer(afu_cxt);
  deallocate_buffer(spl_pt);
#endif

  return 0;
}


