#include "ase_common.h"
#include "nlbv11_common.h"

int main()
{
  session_init();
  
  struct buffer_t *dsm, *umas;
  unsigned char *dsm_status_addr;
  uint32_t *dsm_afuid_addr;
  uint32_t csr_cfg_val;
  
  // NLB registers
  uint32_t dsm_base_addr, dsm_base_addrl, dsm_base_addrh;
  uint32_t nlb_src_addr;
  uint32_t nlb_dst_addr;

  dsm  = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  umas = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  
  dsm->memsize  = 4*1024;
  allocate_buffer(dsm);

  umas_init(0xFFFFFFFF);

  // Calc register values and set them
  dsm_base_addr = (uint64_t)dsm->fake_paddr;
  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;
  dsm_afuid_addr  = (uint32_t*)(dsm->vbase + DSM_AFU_ID);
  dsm_status_addr = (unsigned char*)(dsm->vbase + DSM_STATUS);

  printf("Setting address registers...\n");
  csr_write(NLB_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(NLB_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);
  printf("DSM_BASE  = %x\n", (uint32_t)dsm_base_addr);  
  printf("DSM_BASEH = %x\n", (uint32_t)dsm_base_addrh);  
  printf("DSM_BASEL = %x\n", (uint32_t)dsm_base_addrl);      

  while(*dsm_afuid_addr==0){ };
  if( *dsm_afuid_addr==(uint32_t)NLB_v1_1_L 
   && *(dsm_afuid_addr+0x1)==(uint32_t)(NLB_v1_1_L>>32)
   && *(dsm_afuid_addr+0x2)==(uint32_t)(NLB_v1_1_H)
   && *(dsm_afuid_addr+0x3)==(uint32_t)(NLB_v1_1_H>>32)
   )
  {
        printf("\n*** NLB v1.1 AFU DISCOVERED ***\n");
  } else
  {
        printf("\n*** Invalid AFU %x %x %x %x***\n", *(dsm_afuid_addr+0x3), *(dsm_afuid_addr+0x2), *(dsm_afuid_addr+0x1), *dsm_afuid_addr);
        printf("\n*** Expected ID %lx %lx***\n", NLB_v1_1_H, NLB_v1_1_L);
        return 1;
  }
  
  csr_write(NLB_CTL_OFF, 0x00000000);
  // Sending reset
  printf("Sending a reset... \n");
  csr_write(NLB_CTL_OFF, 0x00000001);


  char umsg_data[64];
  int ii;
  for(ii = 0; ii < 64; ii++)
    umsg_data[ii] = ii;

  printf("Sendign UMSG\n");
  umsg_send(0, umsg_data);
  sleep(1);
  umsg_send(0, umsg_data);
  /* umsg_send(1, umsg_data); */
  /* umsg_send(2, umsg_data); */
  /* umsg_send(3, umsg_data); */
  /* umsg_send(4, umsg_data); */
  /* umsg_send(5, umsg_data); */
  /* umsg_send(6, umsg_data); */
  /* umsg_send(7, umsg_data); */

  // Deallocate
  umas_deinit();
  deallocate_buffer(dsm);

  session_deinit();

  return 0;
}

