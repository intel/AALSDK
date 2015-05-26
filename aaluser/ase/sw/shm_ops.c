// Copyright (c) 2014-2015, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// **************************************************************************
/*
 * Module Info: ASE native SW application interface (bare-bones ASE access)
 * Language   : C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 */

#include "ase_common.h"

// Message queues opened by APP
int app2sim_tx;           // app2sim mesaage queue in TX mode
int sim2app_rx;           // sim2app mesaage queue in RX mode
int app2sim_csr_wr_tx;    // CSR Write MQ in TX mode
int app2sim_umsg_tx;      // UMSG MQ in TX mode
#if 0
int sim2app_intr_rx;      // INTR MQ in RX mode
#endif
int app2sim_simkill_tx;   // Simkill MQ in TX mode

// Lock
pthread_mutex_t lock;

// CSR Map
/* uint32_t csr_map[CSR_MAP_SIZE/4]; */
uint32_t csr_write_cnt = 0;
uint32_t *ase_csr_base;

// MQ established
uint32_t mq_exist_status = MQ_NOT_ESTABLISHED;

// UMSG specific status & global indicators
uint32_t *dsm_cirbstat;
uint32_t num_umsg_log2;
uint32_t num_umsg;
uint32_t umas_exist_status = UMAS_NOT_ESTABLISHED;
struct buffer_t *umas;
uint32_t glbl_umsgmode_csr;

// Instances for SPL page table and context
struct buffer_t *spl_pt;
struct buffer_t *spl_cxt;

// CSR map storage
struct buffer_t *csr_region;

/*
 * Send SIMKILL
 */
void send_simkill()
{
  //#ifdef UNIFIED_FLOW
  char ase_simkill_msg[ASE_MQ_MSGSIZE];
  sprintf(ase_simkill_msg, "%u", ASE_SIMKILL_MSG);
  mqueue_send(app2sim_simkill_tx, ase_simkill_msg);
  // #endif

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  CTRL-C was seen... SW application will exit\n");
  END_YELLOW_FONTCOLOR;
  exit(1);
  /* kill (ase_pid, SIGKILL); */
}


/*
 * Session Initialize
 * Open the message queues to ASE simulator
 */
void session_init()
{
  FUNC_CALL_ENTRY;

  ipc_init();

  // Initialize lock
  if ( pthread_mutex_init(&lock, NULL) != 0)
    {
      printf("  [APP]  Lock initialization failed, EXIT\n");
      exit (1);
    }
  
  // Initialize ase_workdir_path
  ase_workdir_path = ase_eval_session_directory();
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  ASE Session Directory located at =>\n");
  printf("         %s\n", ase_workdir_path);
  END_YELLOW_FONTCOLOR;

  // Register kill signals
  signal(SIGTERM, send_simkill);
  signal(SIGINT , send_simkill);
  signal(SIGQUIT, send_simkill);
  signal(SIGKILL, send_simkill); // *FIXME*: This possibly doesnt work // 
  signal(SIGHUP,  send_simkill);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Initializing simulation session ... ");
  END_YELLOW_FONTCOLOR;

#if 0
  app2sim_csr_wr_tx  = mqueue_create(APP2SIM_CSR_WR_SMQ_PREFIX, O_WRONLY);
  app2sim_tx         = mqueue_create(APP2SIM_SMQ_PREFIX, O_WRONLY);
  sim2app_rx         = mqueue_create(SIM2APP_SMQ_PREFIX, O_RDONLY);
  app2sim_umsg_tx    = mqueue_create(APP2SIM_UMSG_SMQ_PREFIX, O_WRONLY);
  app2sim_simkill_tx = mqueue_create(APP2SIM_SIMKILL_SMQ_PREFIX, O_WRONLY);
  //#if 0
  sim2app_intr_rx    = mqueue_create(SIM2APP_INTR_SMQ_PREFIX, O_RDONLY);
#endif

  app2sim_tx         = mqueue_open(mq_array[0].name, mq_array[0].perm_flag);
  app2sim_csr_wr_tx  = mqueue_open(mq_array[1].name, mq_array[1].perm_flag);
  app2sim_umsg_tx    = mqueue_open(mq_array[2].name, mq_array[2].perm_flag);
  app2sim_simkill_tx = mqueue_open(mq_array[3].name, mq_array[3].perm_flag);
  sim2app_rx         = mqueue_open(mq_array[4].name, mq_array[4].perm_flag);

  // Message queues have been established
  mq_exist_status = MQ_ESTABLISHED;
  BEGIN_YELLOW_FONTCOLOR;

  // Session start
  printf(" DONE\n");
  printf("  [APP]  Session started\n");

  // Creating CSR map 
  printf("  [APP]  Creating CSR map...\n");
  csr_region = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  csr_region->memsize = CSR_MAP_SIZE;
  csr_region->is_csrmap = 1;
  allocate_buffer(csr_region);

  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/*
 * Start ASE RTL simulator
 */
/* void ase_remote_start_simulator() */
/* { */
/*   int ret; */
/*   ret = system("cd $ASE_WORKDIR ; make sim &"); */
/*   if (ret == -1) */
/*     { */
/*       BEGIN_RED_FONTCOLOR; */
/*       printf("APP-C : Problem starting simulator, check if executable exists\n"); */
/*       END_RED_FONTCOLOR; */
/*       exit(1); */
/*     } */
/* } */


/*
 * Session deninitialize
 * Close down message queues to ASE simulator
 */
void session_deinit()
{
  FUNC_CALL_ENTRY;

  // Um-mapping CSR region
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocating CSR map\n");
  END_YELLOW_FONTCOLOR;
  deallocate_buffer(csr_region);

  if (umas_exist_status == UMAS_ESTABLISHED) 
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Deallocating UMAS\n");
      deallocate_buffer(umas);
      END_YELLOW_FONTCOLOR;
    }

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deinitializing simulation session ... ");
  END_YELLOW_FONTCOLOR;

  // Send SIMKILL
  // #ifdef UNIFIED_FLOW
  char ase_simkill_msg[ASE_MQ_MSGSIZE];
  sprintf(ase_simkill_msg, "%u", ASE_SIMKILL_MSG);
  mqueue_send(app2sim_simkill_tx, ase_simkill_msg);
  // #endif

  mqueue_close(app2sim_csr_wr_tx);
  mqueue_close(app2sim_tx);
  mqueue_close(sim2app_rx);
  mqueue_close(app2sim_umsg_tx);
#if 0
  mqueue_close(sim2app_intr_rx);
#endif
  mqueue_close(app2sim_simkill_tx);

  BEGIN_YELLOW_FONTCOLOR;
  printf(" DONE\n");
  printf("  [APP]  Session ended\n");
  END_YELLOW_FONTCOLOR;

  // Lock deinit
  pthread_mutex_destroy(&lock);

  FUNC_CALL_EXIT;
}


/*
 * csr_write : Write data to a location in CSR region (index = 0)
 */
void csr_write(uint32_t csr_offset, uint32_t data)
{
  FUNC_CALL_ENTRY;

  char csr_wr_str[ASE_MQ_MSGSIZE];
  uint32_t *csr_vaddr;

  // Update CSR Region
  csr_vaddr = (uint32_t*)((uint64_t)csr_region->vbase + csr_offset);
  *csr_vaddr = data;

  // ---------------------------------------------------
  // Form a csr_write message
  //                     -----------------
  // CSR_write message:  | offset | data |
  //                     -----------------
  // ---------------------------------------------------
  if (mq_exist_status == MQ_NOT_ESTABLISHED)
    session_init();

  // Send message
  sprintf(csr_wr_str, "%u %u", csr_offset, data);
  mqueue_send(app2sim_csr_wr_tx, csr_wr_str);

  // Display
  csr_write_cnt++;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  CSR Write #%d : offset = 0x%x, data = 0x%08x\n", csr_write_cnt, csr_offset, data);
  END_YELLOW_FONTCOLOR;
  
  usleep(100);
  
  FUNC_CALL_EXIT;
}


/*
 * csr_read : CSR read operation
 *            Read back from DSM space
 */
uint32_t csr_read(uint32_t csr_offset)
{
  FUNC_CALL_ENTRY;

  uint32_t csr_data;
  uint32_t *csr_vaddr; 

  csr_vaddr = (uint32_t*)((uint64_t)ase_csr_base + csr_offset); 
  csr_data = *csr_vaddr;
  
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  CSR Read\t: offset = 0x%x, data = 0x%08x\n", csr_offset, csr_data);
  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;

  return csr_data;
}


/*
 * allocate_buffer: Shared memory allocation and vbase exchange
 * Instantiate a buffer_t structure with given parameters
 * Must be called by ASE_APP
 */
void allocate_buffer(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  pthread_mutex_lock (&lock);

  char tmp_msg[ASE_MQ_MSGSIZE]  = { 0, };
  int static buffer_index_count = 0;

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Attempting to open a shared memory... ");
  END_YELLOW_FONTCOLOR;

  // Buffer is invalid until successfully allocated
  mem->valid = ASE_BUFFER_INVALID;

  // If memory size is not set, then exit !!
  if (mem->memsize <= 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("        Memory requested must be larger than 0 bytes... exiting...\n");
      END_YELLOW_FONTCOLOR;
      exit(1);
    }

  // Autogenerate a memname, by defualt the first region id=0 will be
  // called "/csr", subsequent regions will be called strcat("/buf", id)
  // Initially set all characters to NULL
  memset(mem->memname, '\0', sizeof(mem->memname));
  if(buffer_index_count == 0)
    /* if (mem->is_csrmap == 1)  */
    {
      strcpy(mem->memname, "/csr.");
      strcat(mem->memname, get_timestamp(0) );
      /* mem->is_csrmap = 1; */
    }
  else
    {
      sprintf(mem->memname, "/buf%d.", buffer_index_count);
      strcat(mem->memname, get_timestamp(0) );
      /* mem->is_csrmap = 0; */
    }

  // Disable private memory flag
  mem->is_privmem = 0;

  // Obtain a file descriptor for the shared memory region
  // Tue May  5 19:24:21 PDT 2015
  // https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
  // S_IREAD | S_IWRITE are obselete
  // mem->fd_app = shm_open(mem->memname, O_CREAT|O_RDWR, S_IREAD|S_IWRITE);
  mem->fd_app = shm_open(mem->memname, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
  if(mem->fd_app < 0)
    {
      /* ase_error_report("shm_open", errno, ASE_OS_SHM_ERR); */
      perror("shm_open");
      exit(1);
    }

  // Mmap shared memory region
  mem->vbase = (uint64_t) mmap(NULL, mem->memsize, PROT_READ|PROT_WRITE, MAP_SHARED, mem->fd_app, 0);
  if(mem->vbase == (uint64_t) MAP_FAILED)
    {
      perror("mmap");
      /* ase_error_report("mmap", errno, ASE_OS_MEMMAP_ERR); */
      exit(1);
    }

// Pin ASE CSR base, so CSR Writes can be managed
if (buffer_index_count == 0)
  {
      ase_csr_base = (uint32_t*)mem->vbase;
      //      printf("  [APP]  ASE CSR virtual base = %p\n", ase_csr_base);
  }

  // Extend memory to required size
  ftruncate(mem->fd_app, (off_t)mem->memsize);

  // Set ase_csr_base
  //  if (buffer_index_count == 0)
  // ase_csr_base = (uint32_t*)mem->vbase;

  // Autogenerate buffer index
  mem->index = buffer_index_count++;
  BEGIN_YELLOW_FONTCOLOR;
  printf("SUCCESS\n");
  END_YELLOW_FONTCOLOR;

  // Set buffer as valid
  mem->valid = ASE_BUFFER_VALID;

  // Send an allocate command to DPI, metadata = ASE_MEM_ALLOC
  mem->metadata = HDR_MEM_ALLOC_REQ;
  mem->next = NULL;

  // If memtest is enabled
#ifdef ASE_MEMTEST_ENABLE
  shm_dbg_memtest(mem);
#endif

  // Message queue must be enabled when using DPI (else debug purposes only)
  if (mq_exist_status == MQ_NOT_ESTABLISHED)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Session not started --- STARTING now\n");
      END_YELLOW_FONTCOLOR;
      session_init();
    }

  // Form message and transmit to DPI
  ase_buffer_t_to_str(mem, tmp_msg);
  mqueue_send(app2sim_tx, tmp_msg);

  // Receive message from DPI with pbase populated
  while(mqueue_recv(sim2app_rx, tmp_msg)==0) { /* wait */ }
  ase_str_to_buffer_t(tmp_msg, mem);

  // Print out the buffer
#ifdef ASE_BUFFER_VIEW
  ase_buffer_info(mem);
#endif

  pthread_mutex_unlock(&lock);

  FUNC_CALL_EXIT;
}


/*
 * deallocate_buffer : Deallocate a memory region
 * Destroy shared memory regions
 * Called by ASE APP only
 */
void deallocate_buffer(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  int ret;
  char tmp_msg[ASE_MQ_MSGSIZE] = { 0, };
  char *mq_name;
  mq_name = malloc (ASE_MQ_NAME_LEN);
  memset(mq_name, '\0', ASE_MQ_NAME_LEN);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocating memory region %s ...", mem->memname);
  END_YELLOW_FONTCOLOR;
  usleep(50000);                                   // Short duration wait for sanity

  // Send buffer with metadata = HDR_MEM_DEALLOC_REQ
  mem->metadata = HDR_MEM_DEALLOC_REQ;

  // Send a one way message to request a deallocate
  ase_buffer_t_to_str(mem, tmp_msg);
  mqueue_send(app2sim_tx, tmp_msg);

  // Unmap the memory accordingly
  ret = munmap((void*)mem->vbase, (size_t)mem->memsize);
  if(0 != ret)
    {
      /* ase_error_report("munmap", errno, ASE_OS_MEMMAP_ERR); */
      perror("munmap");
      exit(1);
    }

  // Print if successful
  BEGIN_YELLOW_FONTCOLOR;
  printf("SUCCESS\n");
  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/*
 * shm_dbg_memtest : A memory read write test (DEBUG feature)
 * To run the test ASE_MEMTEST_ENABLE must be enabled.
 * - This test runs alongside a process ase_dbg_memtest.
 * - shm_dbg_memtest() is started before MEM_ALLOC_REQ message is sent to DPI
 *   The simply starts writing 0xCAFEBABE to memory region
 * - ase_dbg_memtest() is started after the MEM_ALLOC_REPLY message is sent back
 *   This reads all the data, verifies it is 0xCAFEBABE and writes 0x00000000 there
 * PURPOSE: To make sure all the shared memory regions are initialised correctly
 */
void shm_dbg_memtest(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  uint32_t *memptr;
  uint32_t *low_addr, *high_addr;

  // Calculate APP low and high address
  low_addr = (uint32_t*)mem->vbase;
  high_addr = (uint32_t*)((uint64_t)mem->vbase + mem->memsize);

  // Start writer
  for(memptr = low_addr; memptr < high_addr; memptr++) {
      *memptr = 0xCAFEBABE;
  }

  FUNC_CALL_ENTRY;
}


/*
 * umas_init : Set up UMAS region
 *             Create a ASE_PAGESIZE * 4KB Umsg region
 * Requires buffer_t handles to UMAS and CSR regions
 */
void umas_init(uint32_t umsg_mode) 
{
  uint32_t csr_umsgbase;

  if (umas_exist_status == UMAS_ESTABLISHED)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  UMAS already established.\n");
      END_YELLOW_FONTCOLOR;
    }
  else
    {
      // Initialize 
      umas = (struct buffer_t *)malloc(sizeof(struct buffer_t));
      umas->memsize = 32 * 1024;
      umas->is_umas = 1;
      allocate_buffer (umas);

      // UMSGmode
      csr_write (ASE_UMSGMODE_CSROFF, umsg_mode);
      glbl_umsgmode_csr = umsg_mode;

      // Setting UMSGBASE
      // UMAS setting in UMSG spec page 12
      csr_umsgbase = ((umas->fake_paddr >> 8) << 2) || 0x1;
      csr_write ( ASE_UMSGBASE_CSROFF, csr_umsgbase);
  
      // Message
      umas_exist_status = UMAS_ESTABLISHED;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  UMAS initialized. \n");
      END_YELLOW_FONTCOLOR;
    }
}


/*
 * Send Unordered Msg (usmg)
 * Fast simplex link to CCI for sending unordered messages to CAFU
 * A listener loop in SIM_SIDE listens to the message and implements
 * requested action
 *
 * Parameters : "4 bit umsg id     " Message ID
 *              "64 byte char array" message
 * Action     : Form a message and send it down a message queue
 *
 */
void umsg_send(int umas_id, char *umsg_data)
{

  uint64_t *umas_target_addr;
  char umsg_str[ASE_MQ_MSGSIZE] = { 0, };
  uint32_t umsg_hint;

  // If requested umas_id is illegal 
  if (umas_id >= 32)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested message ID has not been allocated !!\n");
      END_RED_FONTCOLOR;
      exit(1);
    }

  // Write data to UMAS + umas_id*64
  umas_target_addr = (uint64_t*)((uint64_t)umas->vbase + (umas_id*CL_BYTE_WIDTH));
  memcpy(umas_target_addr, umsg_data, CL_BYTE_WIDTH);

  // Calculate hint
  umsg_hint = glbl_umsgmode_csr & (0x0 || (1 << umas_id));  

  printf("UMSG hint = %d, id = %d\n", umsg_hint, umas_id);

  // MQ Send to SIM
  // memset (umsg_str, '\0', sizeof(umsg_str));
  sprintf(umsg_str, "%u %u %s", umas_id, umsg_hint, umsg_data);

  mqueue_send(app2sim_tx, umsg_str);
}


/*
 * umas_deinit : Deinitialize UMAS region
 *               Deallocate region and unlink
 */
void umas_deinit()
{
  // Disable UMSGBASE
  csr_write (ASE_UMSGBASE_CSROFF, 0x0);
  deallocate_buffer(umas);
  umas_exist_status = UMAS_NOT_ESTABLISHED;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  UMAS deinitialized. \n");  
  END_RED_FONTCOLOR;
}


/*
 * setup_spl_cxt_pte : Create SPL Page table and Contexts
 * Setup SPL context and page tables using ASE memory allocation
 * mechanism. SPL will read this from simulated memory regions.
 */
void setup_spl_cxt_pte(struct buffer_t *dsm, struct buffer_t *afu_cxt)
{
  FUNC_CALL_ENTRY;

  // Allocate spaces for shared buffers
  spl_pt  = (struct buffer_t *)malloc(sizeof(struct buffer_t));
  spl_cxt = (struct buffer_t *)malloc(sizeof(struct buffer_t));

  uint64_t num_2mb_chunks;
  uint64_t *spl_pt_addr;
  uint64_t afu_cxt_2mb_align;
  int ii;
  uint64_t *spl_cxt_vaddr;

  // Calculate number of 2MB chunks
  num_2mb_chunks = afu_cxt->memsize /(2*1024*1024);
  if (afu_cxt->memsize - num_2mb_chunks*(2*1024*1024) > 0)
    num_2mb_chunks++;

  // Print info
  BEGIN_YELLOW_FONTCOLOR;
  printf("   AFU CXT size = 0x%x bytes | Number of 2 MB chunks = %lu\n", afu_cxt->memsize, num_2mb_chunks);
  END_YELLOW_FONTCOLOR;

  // Allocate SPL page table
  spl_pt->memsize = CL_BYTE_WIDTH * num_2mb_chunks;
  allocate_buffer(spl_pt);

  // Calculate SPL_PT address and AFU_CXT 2MB bounds
  for(ii = 0; ii < num_2mb_chunks; ii = ii + 1)
    {
      afu_cxt_2mb_align = (uint64_t)afu_cxt->fake_paddr + (uint64_t)(ii*CCI_CHUNK_SIZE);
      spl_pt_addr       = (uint64_t*)((uint64_t)spl_pt->vbase + CL_BYTE_WIDTH*ii);
      *spl_pt_addr      = afu_cxt_2mb_align;
    }

  // SPL context
  spl_cxt->memsize = 64; // 2 cache lines // but allocating 4 KB
  allocate_buffer(spl_cxt);
  spl_cxt_vaddr = (uint64_t*)spl_cxt->vbase;
  spl_cxt_vaddr[0] = spl_pt->fake_paddr;
  spl_cxt_vaddr[1] = afu_cxt->vbase;
  spl_cxt_vaddr[2] = (uint64_t)(num_2mb_chunks << 32) | 0x1;
  spl_cxt_vaddr[3] = 1;

  FUNC_CALL_EXIT;

}


/*
 * SPL Setup DSM: Sets up SPL DSM
 */
void spl_driver_dsm_setup(struct buffer_t *dsm)
{
  FUNC_CALL_ENTRY;

  uint32_t dsm_base_addrl, dsm_base_addrh;
  uint32_t *dsm_afuid_addr;

  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;
  dsm_afuid_addr= (uint32_t*)(dsm->vbase + DSM_AFU_ID);
  *(dsm_afuid_addr+0x0) = 0x0;
  *(dsm_afuid_addr+0x1) = 0x0;
  *(dsm_afuid_addr+0x2) = 0x0;
  *(dsm_afuid_addr+0x3) = 0x0;

  // Write SPL context ASE physical address & DSM address
  csr_write(SPL_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(SPL_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);
  /* csr_write(SPL_CXT_BASEH_OFF, (spl_cxt->fake_paddr >> 32)); */
  /* csr_write(SPL_CXT_BASEL_OFF, spl_cxt->fake_paddr); */

  // Poll for SPL_ID and print
  while(*dsm_afuid_addr==0)
    {
      usleep(5000);
    }
  // sleep(1);
  printf("\n*** SPL ID %x %x %x %x***\n",
	 *(dsm_afuid_addr+0x3), *(dsm_afuid_addr+0x2), *(dsm_afuid_addr+0x1), *dsm_afuid_addr);

  FUNC_CALL_EXIT;

}


/*
 * SPL Reset
 */
void spl_driver_reset(struct buffer_t *dsm)
{
  FUNC_CALL_ENTRY;

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Issuing SPL Reset\n");
  END_YELLOW_FONTCOLOR;
  csr_write( SPL_CH_CTRL_OFF, 0x1 );
  usleep(20);
  csr_write( SPL_CH_CTRL_OFF, 0x0 );

  FUNC_CALL_EXIT;

}


/*
 * AFU DSM poll - signature check
 */
void spl_driver_afu_setup(struct buffer_t *dsm)
{
  FUNC_CALL_ENTRY;

  uint32_t dsm_base_addrl, dsm_base_addrh;
  uint32_t *dsm_afuid_addr;

  printf("Polling AFU_ID...\n");
  dsm_base_addrh= (uint64_t)dsm->fake_paddr >>32;
  dsm_base_addrl= (uint32_t)dsm->fake_paddr;
  dsm_afuid_addr  = (uint32_t*)(dsm->vbase + DSM_AFU_ID);
  *(dsm_afuid_addr+0x0) = 0x0;
  *(dsm_afuid_addr+0x1) = 0x0;
  *(dsm_afuid_addr+0x2) = 0x0;
  *(dsm_afuid_addr+0x3) = 0x0;
  csr_write(AFU_DSM_BASEH_OFF, (uint32_t)dsm_base_addrh);
  csr_write(AFU_DSM_BASEL_OFF, (uint32_t)dsm_base_addrl);

  while(*dsm_afuid_addr==0)
    {
      usleep(5000);
    }

  printf("\n*** AFU_ID %x %x %x %x***\n", *(dsm_afuid_addr+0x3), *(dsm_afuid_addr+0x2), *(dsm_afuid_addr+0x1), *dsm_afuid_addr);
}

/*
 * SPL Driver Transaction start
 */
// void spl_driver_start(struct buffer_t *dsm, struct buffer_t *afu_cxt)
void spl_driver_start(uint64_t *afu_cxt_vbase)
{
  BEGIN_YELLOW_FONTCOLOR;
  printf("APP-C : Starting SPL ... ");
  csr_write(SPL_CXT_BASEH_OFF, (spl_cxt->fake_paddr >> 32));
  csr_write(SPL_CXT_BASEL_OFF, spl_cxt->fake_paddr);
  /* csr_write(AFU_CXT_BASEH_OFF, (afu_cxt->vbase >> 32)); */
  /* csr_write(AFU_CXT_BASEL_OFF, afu_cxt->vbase); */
  csr_write(AFU_CXT_BASEH_OFF, (uint32_t)((uint64_t)afu_cxt_vbase >> 32) );
  csr_write(AFU_CXT_BASEL_OFF, (uint32_t)((uint64_t)afu_cxt_vbase) );
  csr_write(SPL_CH_CTRL_OFF, 0x2);
  printf("DONE\n");
  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/*
 * SPL Driver Transaction stop
 */
void spl_driver_stop()
{
  FUNC_CALL_ENTRY;

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  APP-C : Stopping SPL ");
  END_YELLOW_FONTCOLOR;
  csr_write(SPL_CH_CTRL_OFF, 0x0);

  FUNC_CALL_EXIT;
}

