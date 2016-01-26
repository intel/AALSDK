// ***************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (C) 2008-2011 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all  documents related to
// the  source  code  ("Material")  are  owned  by  Intel  Corporation  or its
// suppliers  or  licensors.    Title  to  the  Material  remains  with  Intel
// Corporation or  its suppliers  and licensors.  The Material  contains trade
// secrets  and  proprietary  and  confidential  information  of  Intel or its
// suppliers and licensors.  The Material is protected  by worldwide copyright
// and trade secret laws and treaty provisions. No part of the Material may be
// used,   copied,   reproduced,   modified,   published,   uploaded,  posted,
// transmitted,  distributed,  or  disclosed  in any way without Intel's prior
// express written permission.
//
// No license under any patent,  copyright, trade secret or other intellectual
// property  right  is  granted  to  or  conferred  upon  you by disclosure or
// delivery  of  the  Materials, either expressly, by implication, inducement,
// estoppel or otherwise.  Any license under such intellectual property rights
// must be express and approved by Intel in writing.
//
// Engineer:            Pratik Marolia
// Edited Date:         Tues 24 Jan 2012
// Module Name:         nlb_all_test.c
// Project:             NLB SW- ASE
// Description:         Implements test modes 0,1,2,3 for NLB AFU
//
// ***************************************************************************

#include "ase_common.h"
#include "nlbv11_common.h"

#define M_LPBK1 0
#define M_READ  1
#define M_WRITE 2
#define M_TRPUT 3
#define M_LPBK2 4
#define M_LPBK3 5


// #define NUM_CL_TEST       16
// #define NUM_CL_TEST       32768 // 1024 // 0x8

#define TEST_MODE         M_LPBK1
//#define TEST_MODE         M_READ
// #define TEST_MODE         M_WRITE

#define TEST_WRTHRU_EN    0
#define TEST_DELAY_EN     0
#define TEST_CONT         0
#define TEST_START_DELAY  5
#define TEST_CFG          0
#define TEST_INACT_THRESH 10

int num_cl ;

void init_buff_random(struct buffer_t *inp_buf)
{
  uint32_t *hi_addr, *lo_addr, *ptr;
  int i;
  lo_addr = (uint32_t*)inp_buf->vbase;
  hi_addr = (uint32_t*)((uint64_t)inp_buf->vbase + 64*num_cl); // NUM_CL_TEST);
  
  for(ptr=lo_addr, i= 0 ; ptr < hi_addr; ptr++, i++)
    *ptr = i; // rand();
  
}

void init_dsm(struct buffer_t *inp_buf, uint32_t val)
{
        uint32_t *hi_addr, *lo_addr, *ptr;
        lo_addr = (uint32_t*)inp_buf->vbase;
        hi_addr = (uint32_t*)((uint64_t)inp_buf->vbase + 2*1024*1024);

  for(ptr=lo_addr; ptr < hi_addr; ptr++)
    *ptr = val;

}

int poll_status(unsigned char *dsm_status_addr)
{
        int iter_min;
        int error_set;
	int ii;
          if(TEST_CONT==1)
          {
	    for(iter_min = 0; iter_min < 1000; iter_min++)
	      //   for(iter_min = 0; iter_min < 3; iter_min++)
	      {
		printf("Sleep for %d secs...\n", iter_min);
		/* csr_write(0xFF0, 0xCAFEBABE); */
		/* csr_write(0xFF4, 0xFEEDFACE); */
		sleep(1);
	      }
                
                  // Stop test
                  printf("APP: NLB test stopping... \n");
                  csr_write(NLB_CTL_OFF, 0x00000005);
                  while(*(dsm_status_addr) == 0);
           }
           else
           {
                  // Wait and end test
                  /* while(*(dsm_status_addr) == 0); */

                  while(*(dsm_status_addr) == 0)
		    {		     
		      /* csr_write(0xFF0, 0xCAFEBABE); // Remove me */
		      /* csr_write(0xFF4, 0xFEEDFACE); // Remove me */
		      /* usleep(1000); */
		      /* ase_dump_to_file(buf1, "buf1.dump"); */
		      /* ase_dump_to_file(buf2, "buf2.dump"); */
		    }
           }
           error_set = *(dsm_status_addr+4);
           if(error_set==0)
           {
                printf("\n***Status Flag Set!***\n");
                printf("End Penalty  = %d \n" ,*(dsm_status_addr +28));
                printf("Start Penalty= %d \n" ,*(dsm_status_addr +24));
                printf("Num_Writes = %d   \n" ,*(dsm_status_addr +20));
                printf("Num_Reads  = %d   \n" ,*(dsm_status_addr +16));
                printf("Num_ticks  = %d   \n" ,*(dsm_status_addr +8) );
                printf("ErrorVector= %x   \n"   ,*(dsm_status_addr +4) );
                printf("ErrorInfo  = %llx  \n" ,*(dsm_status_addr +32));
                return 1;
           }
           else
           {
                printf("\n*****ERROR Detected*****\n");
                printf("ErrorVector= %32x \n" ,*(dsm_status_addr +4) );
                printf("ErrorInfo  = %256x\n" ,*(dsm_status_addr +32));
                return 0;
           }    
}



// int main()
int main(int argc, char *argv[])
{
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

  int wait = 0;

  // Buffer Allocation and Initialization
  //--------------------------------------------------------------
  struct buffer_t *dsm, *buf1, *buf2;
  unsigned char *dsm_status_addr;
  uint32_t *dsm_afuid_addr;
  uint32_t csr_cfg_val;
  
  // NLB registers
  uint32_t dsm_base_addr, dsm_base_addrl, dsm_base_addrh;
  uint32_t nlb_src_addr;
  uint32_t nlb_dst_addr;
  
  dsm  = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  buf1 = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  buf2 = (struct buffer_t *)malloc(sizeof(struct buffer_t));

  //Assign buffer size
  dsm->memsize  = 2*1024*1024;
  buf1->memsize = num_cl*64;
  buf2->memsize = num_cl*64;

  // Allocate buffer
  allocate_buffer(dsm);
  allocate_buffer(buf1);
  allocate_buffer(buf2);
 
  // Print buffer information
  ase_buffer_info(dsm);
  ase_buffer_info(buf1);
  ase_buffer_info(buf2);

  switch(TEST_MODE) {
  case M_LPBK1:
  {
          init_buff_random(buf1);
          init_buff_random(buf2);
          // init_dsm(dsm,0);
          break;
  }
  case M_READ:
  {
          init_dsm(dsm,0);
          break;
  }
  case M_WRITE:
  {
          init_dsm(dsm,0);
          break;
  }
  case M_TRPUT:
  {
          init_dsm(dsm,0);
          break;
  }
  default:
  {
        printf("***Invalid Test Mode: %x***\n", TEST_MODE);
        return 0;  
  }
  }

  //--------------------------------------------------------------
  // Test Reset and initialization flow
  //--------------------------------------------------------------
  csr_write(0x280 , 0x01000000);
  usleep(10);
  csr_write(0x280, 0x0);

  // Calc register values and set them
  dsm_base_addr = (uint64_t)dsm->fake_paddr;
  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;
  nlb_src_addr  = (uint64_t)buf1->fake_paddr >> 6;
  nlb_dst_addr  = (uint64_t)buf2->fake_paddr >> 6;
  
  dsm_afuid_addr  = (uint32_t*)(dsm->vbase + DSM_AFU_ID);
  dsm_status_addr = (unsigned char*)(dsm->vbase + DSM_STATUS);
  
  printf("Setting address registers...\n");
  csr_write(NLB_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(NLB_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);
  printf("DSM_BASE  = %x\n", (uint32_t)dsm_base_addr);  
  printf("DSM_BASEH = %x\n", (uint32_t)dsm_base_addrh);  
  printf("DSM_BASEL = %x\n", (uint32_t)dsm_base_addrl);      


  while(*dsm_afuid_addr==0)
    { 
      usleep(5000);
    };
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
  
  csr_write(NLB_SRC_ADDR_OFF , (uint32_t)nlb_src_addr);
  csr_write(NLB_DST_ADDR_OFF , (uint32_t)nlb_dst_addr);
  csr_write(NLB_NUM_LINES_OFF, num_cl);

  printf("src  = %x\n", (uint32_t)nlb_src_addr);  
  printf("dst  = %x\n", (uint32_t)nlb_dst_addr);  

  // test config set here
  csr_cfg_val =   TEST_WRTHRU_EN
                |(TEST_CONT<<1)
                |(TEST_MODE<<2)
                |(TEST_DELAY_EN<<8)
                |(TEST_START_DELAY<<12)
                |(TEST_CFG<<20);
  csr_write(NLB_CFG_OFF, csr_cfg_val);
  printf("Test config = %08x\n", csr_cfg_val);

  csr_write(NLB_INACT_THRESH, TEST_INACT_THRESH);
  printf("Inactivity threshold = %d\n", TEST_INACT_THRESH);

  // start test
  printf("APP: NLB test starting... \n");
  csr_write(NLB_CTL_OFF, 0x00000003);
  //--------------------------------------------------------------
  switch(TEST_MODE) {
  case M_LPBK1:
  {
          poll_status(dsm_status_addr);
          ase_dump_to_file(buf1, "buf1.dump");
          ase_dump_to_file(buf2, "buf2.dump");
        
          // Memcpy check
          if( memcmp((unsigned char*)buf1->vbase, (unsigned char*) buf2->vbase, num_cl*64) == 0)
            printf("LPBK1: Memory Copy TEST Successful\n");
          else
            printf("LPBK1: Memory Copy TEST Failed\n");        
            
          break;
  }
  case M_READ:
  {
        poll_status(dsm_status_addr);
        break;
  }
  case M_WRITE:
  {
        poll_status(dsm_status_addr);
        break;
  }
  case M_TRPUT:
  {
    printf("RRS: polling loop, addr = %p ...\n", dsm_status_addr);
    ase_dump_to_file(buf1, "buf1.dump");
    ase_dump_to_file(buf2, "buf2.dump");
    poll_status(dsm_status_addr);
    break;
  }
  default:
  {
        printf("***Invalid Test Mode: %x***\n", TEST_MODE);
        return 0;
  }
  }
  

  printf("APP: Test done...\n");

  deallocate_buffer(buf1);
  deallocate_buffer(buf2);
  deallocate_buffer(dsm);
<<<<<<< HEAD

=======
>>>>>>> 23c89f94d68c0f6761c0bfb4db3b91c9ba738146
  session_deinit();

  return 0;
}


