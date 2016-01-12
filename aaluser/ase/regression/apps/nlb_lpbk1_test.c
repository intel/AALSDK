#include "ase_common.h"

#define DFH                      0x0000
#define AFU_ID_L                 0x0008
#define AFU_ID_H                 0x0010
#define CSR_DFH_RSVD0            0x0018
#define CSR_DFH_RSVD1            0x0020

#define CSR_SCRATCHPAD0          0x100
#define CSR_SCRATCHPAD1          0x104
#define CSR_SCRATCHPAD2          0x108

#define CSR_AFU_DSM_BASEL        0x110
#define CSR_AFU_DSM_BASEH        0x114
#define CSR_SRC_ADDR             0x120
#define CSR_DST_ADDR             0x128
#define CSR_NUM_LINES            0x130
#define CSR_CTL                  0x138
#define CSR_CFG                  0x140
#define CSR_INACT_THRESH         0x148
#define CSR_INTERRUPT0           0x150
#define DSM_STATUS_TEST_COMPLETE 0x40


int main(int argc, char *argv[])
{
  int num_cl;
  if (argc > 1) 
    {
      num_cl = atoi( argv[1] );
    }
  else
    {
      num_cl = 16;
    }
  printf("Num CL = %d\n", num_cl);

  session_init();

  // Port control
  ase_portctrl("AFU_RESET 0");
  ase_portctrl("UMSG_MODE 63");
  usleep(10000);
  ase_portctrl("AFU_RESET 1");
  // usleep(100);
  // sleep(2);

  // Send umsg
  uint64_t umsgdata;
  /* umsgdata = 0xCAFEBABEDECAFBAD; */
  /* umsg_send (1, &umsgdata); */
  /* umsgdata = 0xBABABABADEDEDADE; */
  /* umsg_send (7, &umsgdata); */

  struct buffer_t *dsm, *src, *dst;
  
  dsm = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  src = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  dst = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  
  memset(dsm, '0', sizeof(struct buffer_t));  
  memset(src, '0', sizeof(struct buffer_t));  
  memset(dst, '0', sizeof(struct buffer_t));  
  
  //Assign buffer size
  dsm->memsize  = 2*1024*1024;
  src->memsize = num_cl*64;
  dst->memsize = num_cl*64;

  dsm->is_mmiomap = 0;
  src->is_mmiomap = 0;
  dst->is_mmiomap = 0;

  dsm->is_umas = 0;
  src->is_umas = 0;
  dst->is_umas = 0;
  
  // Allocate buffer
  allocate_buffer(dsm);
  allocate_buffer(src);
  allocate_buffer(dst);
  
  // Print buffer information
  /* ase_buffer_info(dsm); */
  /* ase_buffer_info(src); */
  /* ase_buffer_info(dst); */

  // Write something in src
  // uint64_t *test_data;
  FILE *fp_rand;
  int ret;
  // test_data = (uint64_t*)src->vbase;
  /* *test_data = 0xCAFEBABE; */
  
  fp_rand = fopen("/dev/urandom", "r");
  ret = fread((void *)src->vbase, 1, (size_t)src->memsize, fp_rand);
  if (ret != src->memsize)     
    {
      perror("fread");
      return 1;
    }
  fclose(fp_rand);

  mmio_write32(CSR_AFU_DSM_BASEL, (uint32_t)dsm->fake_paddr);
  mmio_write32(CSR_AFU_DSM_BASEH, (dsm->fake_paddr >> 32));


  uint64_t *data_l, *data_h;
  while(1)
    {
      mmio_read64(0x008, data_l);
      mmio_read64(0x010, data_h);
    }
  /* printf("AFUID = %llx %llx\n",  */
  /* 	 (unsigned long long)*data_h,  */
  /* 	 (unsigned long long)*data_l); */
 
  mmio_write32(CSR_CTL, 0);
  
  mmio_write32(CSR_CTL, 1);
  
  mmio_write64(CSR_SRC_ADDR, (src->fake_paddr >> 6));

  mmio_write64(CSR_DST_ADDR, (dst->fake_paddr >> 6));
  
  mmio_write32(CSR_NUM_LINES, num_cl);

  mmio_write32(CSR_CFG, 0);  

  uint32_t *status_addr = (uint32_t *)((uint64_t)dsm->vbase + DSM_STATUS_TEST_COMPLETE);

  mmio_write32(CSR_CTL, 3);

  while(*status_addr == 0)
    {
      usleep(100);
    }
  
  printf("Test complete\n");

  if (memcmp((char*)src->vbase, (char*)dst->vbase, num_cl*64) == 0)
    printf("Buffers matched\n");
  else
    printf("*** Buffer mismatch ***\n");

  /* deallocate_buffer(dsm); */
  /* deallocate_buffer(src); */
  /* deallocate_buffer(dst); */

  deallocate_buffer_by_index(4);
  deallocate_buffer_by_index(3);
  deallocate_buffer_by_index(2);

  session_deinit();

  return 0;
}
