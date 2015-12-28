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
/* int app2sim_tx;           // app2sim mesaage queue in TX mode */
/* int sim2app_rx;           // sim2app mesaage queue in RX mode */
/* int app2sim_mmio_tx;      // MMIO read write request */
/* int app2sim_umsg_tx;      // UMSG MQ in TX mode */
/* #if 0 */
/* int sim2app_intr_rx;      // INTR MQ in RX mode */
/* #endif */
/* int app2sim_simkill_tx;   // Simkill MQ in TX mode */

// Lock
pthread_mutex_t lock;

// CSR Map
/* uint32_t csr_map[CSR_MAP_SIZE/4]; */
uint32_t mmio_write_cnt = 0;
uint32_t mmio_read_cnt = 0;
uint64_t *mmio_afu_vbase;

uint64_t *umsg_umas_vbase;

// MQ established
uint32_t mq_exist_status = MQ_NOT_ESTABLISHED;

// UMSG specific status & global indicators
/* uint32_t *dsm_cirbstat; */
/* uint32_t num_umsg_log2; */
/* uint32_t num_umsg; */
uint32_t umas_exist_status = UMAS_NOT_ESTABLISHED;
/* uint32_t glbl_umsgmode_csr; */

// Instances for SPL page table and context
/* struct buffer_t *spl_pt; */
/* struct buffer_t *spl_cxt; */

// CSR map storage
struct buffer_t *mmio_region;

// UMAS region
struct buffer_t *umas_region;

// Workspace metadata table
struct wsmeta_t *wsmeta_head = (struct wsmeta_t *) NULL;
struct wsmeta_t *wsmeta_end = (struct wsmeta_t *) NULL;


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

  setvbuf(stdout, NULL, _IONBF, 0);

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

  app2sim_tx          = mqueue_open( mq_array[0].name, mq_array[0].perm_flag );
  app2sim_mmioreq_tx  = mqueue_open( mq_array[1].name, mq_array[1].perm_flag );
  app2sim_umsg_tx     = mqueue_open( mq_array[2].name, mq_array[2].perm_flag );
  app2sim_simkill_tx  = mqueue_open( mq_array[3].name, mq_array[3].perm_flag );
  sim2app_rx          = mqueue_open( mq_array[4].name, mq_array[4].perm_flag );
  sim2app_mmiorsp_rx  = mqueue_open( mq_array[5].name, mq_array[5].perm_flag );
  app2sim_portctrl_tx = mqueue_open( mq_array[6].name, mq_array[6].perm_flag );

  // Message queues have been established
  mq_exist_status = MQ_ESTABLISHED;

  BEGIN_YELLOW_FONTCOLOR;

  // Session start
  printf(" DONE\n");
  printf("  [APP]  Session started\n");

  // Creating CSR map 
  printf("  [APP]  Creating MMIO region...\n");
  mmio_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  mmio_region->memsize = MMIO_LENGTH;
  mmio_region->is_mmiomap = 1;  
  allocate_buffer(mmio_region);
  mmio_afu_vbase = (uint64_t*)((uint64_t)mmio_region->vbase + MMIO_AFU_OFFSET);
  printf("  [APP]  AFU MMIO Virtual Base Address = %p\n", (void*) mmio_afu_vbase); 

  // Create UMSG region
  printf("  [APP]  Creating UMAS region... \n");
  umas_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  umas_region->memsize = UMAS_LENGTH;
  umas_region->is_umas = 1;
  allocate_buffer(umas_region);
  umsg_umas_vbase = (uint64_t*)((uint64_t)umas_region->vbase);
  printf("  [APP]  UMAS Virtual Base address = %p\n", (void*)umsg_umas_vbase);

  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/*
 * Session deninitialize
 * Close down message queues to ASE simulator
 */
void session_deinit()
{
  FUNC_CALL_ENTRY;

  // Unmap UMAS region
  if (umas_exist_status == UMAS_ESTABLISHED) 
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Deallocating UMAS\n");
      deallocate_buffer(umas_region);
      END_YELLOW_FONTCOLOR;
    }

  // Um-mapping CSR region
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocating MMIO map\n");
  END_YELLOW_FONTCOLOR;
  deallocate_buffer(mmio_region);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deinitializing simulation session ... ");
  END_YELLOW_FONTCOLOR;

  // Send SIMKILL
  char ase_simkill_msg[ASE_MQ_MSGSIZE];
  sprintf(ase_simkill_msg, "%u", ASE_SIMKILL_MSG);
  mqueue_send(app2sim_simkill_tx, ase_simkill_msg);
  
  mqueue_close(app2sim_mmioreq_tx);
  mqueue_close(sim2app_mmiorsp_rx);
  mqueue_close(app2sim_tx);
  mqueue_close(sim2app_rx);
  mqueue_close(app2sim_umsg_tx);
  mqueue_close(app2sim_simkill_tx);
  mqueue_close(app2sim_portctrl_tx);

  BEGIN_YELLOW_FONTCOLOR;
  printf(" DONE\n");
  printf("  [APP]  Session ended\n");
  END_YELLOW_FONTCOLOR;

  // Lock deinit
  pthread_mutex_destroy(&lock);

  FUNC_CALL_EXIT;
}


/*
 * MMIO Write 32-bit
 */
void mmio_write32 (uint32_t offset, uint32_t data)
{
  FUNC_CALL_ENTRY;

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Write\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      if (mq_exist_status == MQ_NOT_ESTABLISHED)
	session_init();

      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      char mmio_str[ASE_MQ_MSGSIZE];  
      memset(mmio_str, '\0', ASE_MQ_MSGSIZE);

      char csr_data_str[CL_BYTE_WIDTH];
      memset(csr_data_str, '\0', CL_BYTE_WIDTH);
      // mmio_pkt->qword[0] = (long long)data;
      memcpy(csr_data_str, &data, sizeof(uint32_t));

      uint32_t *mmio_vaddr;
      mmio_vaddr = (uint32_t*)((uint64_t)mmio_afu_vbase + offset);

      // Prepare MMIO pkt
      mmio_pkt->type = MMIO_WRITE_REQ;
      mmio_pkt->width = MMIO_WIDTH_32;
      mmio_pkt->addr = offset;
      memcpy(mmio_pkt->qword, csr_data_str, sizeof(uint32_t));
      mmio_pkt->resp_en = 0;
  
#ifdef ASE_DEBUG
      printf("mmio_pkt => %x %d %d %llx %d\n", 
	     mmio_pkt->type,
	     mmio_pkt->width,
	     mmio_pkt->addr,
	     mmio_pkt->qword[0],
	     mmio_pkt->resp_en);
#endif

      // Update CSR Region
      memcpy(mmio_vaddr, (char*)csr_data_str, sizeof(uint32_t));
  
      // Send message
      memcpy(mmio_str, mmio_pkt, sizeof(mmio_t));
      mqueue_send(app2sim_mmioreq_tx, mmio_str);

      // Display
      mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write #%d : offset = 0x%x, data = 0x%08x\n", mmio_write_cnt, offset, data);

      // Wait until MMIO response comes back and discard
      while(mqueue_recv(sim2app_mmiorsp_rx, mmio_str)==0) { /* wait */ }

#ifdef ASE_DEBUG  
      printf("  [APP]  MMIO Write #%d completed\n", mmio_write_cnt);
#endif

      END_YELLOW_FONTCOLOR;
    }
  FUNC_CALL_EXIT;
}


/*
 * MMIO Write 64-bit
 */
void mmio_write64 (uint32_t offset, uint64_t data)
{
  FUNC_CALL_ENTRY;

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Write\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      if (mq_exist_status == MQ_NOT_ESTABLISHED)
	session_init();

      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      char mmio_str[ASE_MQ_MSGSIZE];
      memset(mmio_str, '\0', ASE_MQ_MSGSIZE);

      char csr_data_str[CL_BYTE_WIDTH];
      memset(csr_data_str, '\0', CL_BYTE_WIDTH);
      memcpy(csr_data_str, &data, sizeof(uint64_t));

      uint64_t *mmio_vaddr;
      mmio_vaddr = (uint64_t*)((uint64_t)mmio_afu_vbase + offset);

      // ---------------------------------------------------
      // Form a csr_write message
      //                     -------------------------
      // CSR_write message:  | width | offset | data |
      //                     -------------------------
      // ---------------------------------------------------
      // Update CSR Region
      memcpy(mmio_vaddr, csr_data_str, sizeof(uint64_t));
      // *mmio_vaddr = data;
  
      mmio_pkt->type = MMIO_WRITE_REQ;
      mmio_pkt->width = MMIO_WIDTH_64;
      mmio_pkt->addr = offset;
      memcpy(mmio_pkt->qword, csr_data_str, sizeof(uint64_t));
      mmio_pkt->resp_en = 0;

      // Send message
      memcpy(mmio_str, (char*)mmio_pkt, sizeof(mmio_t));
      mqueue_send(app2sim_mmioreq_tx, mmio_str);

#ifdef ASE_DEBUG
      printf("mmio_pkt => %x %d %d %llx %d\n", 
	     mmio_pkt->type,
	     mmio_pkt->width,
	     mmio_pkt->addr,
	     mmio_pkt->qword[0],
	     mmio_pkt->resp_en);
#endif

      // Display
      mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write #%d : offset = 0x%x, data = 0x%llx\n", mmio_write_cnt, offset, (unsigned long long)data);

      // Wait until MMIO response comes back and discard
      while(mqueue_recv(sim2app_mmiorsp_rx, mmio_str)==0) { /* wait */ }
      memcpy(mmio_pkt, mmio_str, sizeof(mmio_t));
#ifdef ASE_DEBUG  
      printf("  [APP]  MMIO Write #%d completed\n", mmio_write_cnt);
#endif

      END_YELLOW_FONTCOLOR;
    }
  FUNC_CALL_EXIT;
}



/* *********************************************************************
 * MMIO Read
 * *********************************************************************
 *
 * Request packet
 * ---------------------------------------
 * | MMIO_READ_REQ | MMIO_WIDTH | Offset |
 * ---------------------------------------
 *
 * Response packet
 * -------------------------------------
 * | MMIO_READ_RSP | MMIO_WIDTH | Data |
 * -------------------------------------
 *
 */
/*
 * MMIO Read 32-bit
 */
void mmio_read32(uint32_t offset, uint32_t *data)
{
  FUNC_CALL_ENTRY;

  char mmio_str[ASE_MQ_MSGSIZE];
  mmio_t *mmio_pkt;

  char csr_data_str[CL_BYTE_WIDTH];
  memset(csr_data_str, '\0', CL_BYTE_WIDTH);

  if (mq_exist_status == MQ_NOT_ESTABLISHED)
    session_init();

  // Send MMIO Read Request
  /* sprintf(mmio_str, "%u %u %u", MMIO_READ_REQ, MMIO_WIDTH_32, offset); */
  mmio_pkt->type = MMIO_READ_REQ;
  mmio_pkt->width = MMIO_WIDTH_32;
  mmio_pkt->addr = offset;
  memcpy(mmio_pkt->qword, csr_data_str, CL_BYTE_WIDTH);
  mmio_pkt->resp_en = 0;
  
  // Send MMIO Request
  memset(mmio_str, '\0', ASE_MQ_MSGSIZE);
  memcpy(mmio_str, (char*)mmio_pkt, sizeof(mmio_t));
  mqueue_send(app2sim_mmioreq_tx, mmio_str);

  // Display
  mmio_read_cnt++;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  MMIO Read #%d  : offset = 0x%x\n", mmio_read_cnt, offset);

  // Receive MMIO Read Response
  memset(mmio_str, '\0', ASE_MQ_MSGSIZE);
  while(mqueue_recv(sim2app_mmiorsp_rx, mmio_str)==0) { /* wait */ }
  memcpy(mmio_pkt, mmio_str, sizeof(mmio_t));

#ifdef ASE_DEBUG  
  printf("  [APP]  MMIO Read #%d completed\n", mmio_read_cnt);
#endif
  
  // Write data
  data = (uint32_t*)((uint64_t)mmio_afu_vbase + offset);
  *data = (uint32_t)mmio_pkt->qword[0];
  
  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/*
 * MMIO Read 64-bit
 */
void mmio_read64(uint32_t offset, uint64_t *data)
{
  FUNC_CALL_ENTRY;

  char mmio_str[ASE_MQ_MSGSIZE];
  mmio_t *mmio_pkt;

  char csr_data_str[CL_BYTE_WIDTH];
  memset(csr_data_str, '\0', CL_BYTE_WIDTH);

  if (mq_exist_status == MQ_NOT_ESTABLISHED)
    session_init();

  // Send MMIO Read Request
  mmio_pkt->type = MMIO_READ_REQ;
  mmio_pkt->width = MMIO_WIDTH_64;
  mmio_pkt->addr = offset;
  memcpy(mmio_pkt->qword, csr_data_str, CL_BYTE_WIDTH);
  mmio_pkt->resp_en = 0;
  
  // Send MMIO Request
  memset(mmio_str, '\0', ASE_MQ_MSGSIZE);
  memcpy(mmio_str, (char*)mmio_pkt, sizeof(mmio_t));
  mqueue_send(app2sim_mmioreq_tx, mmio_str);

  // Display
  mmio_read_cnt++;

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  MMIO Read #%d  : offset = 0x%x\n", mmio_read_cnt, offset);

  // Receive MMIO Read Response
  memset(mmio_str, '\0', ASE_MQ_MSGSIZE);
  while(mqueue_recv(sim2app_mmiorsp_rx, mmio_str)==0) { /* wait */ }
  memcpy(mmio_pkt, mmio_str, sizeof(mmio_t));

#ifdef ASE_DEBUG  
  printf("  [APP]  MMIO Read #%d completed\n", mmio_write_cnt);
#endif
  
  // Typecast back to mmio_pkt, and update data
  data = (uint64_t*)((uint64_t)mmio_afu_vbase + offset);
  *data = (uint64_t)mmio_pkt->qword[0];

  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
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
  // called "/mmio", subsequent regions will be called strcat("/buf", id)
  // Initially set all characters to NULL
  memset(mem->memname, '\0', sizeof(mem->memname));
  // if(buffer_index_count == 0)
  if(mem->is_mmiomap == 1)
    /* if (mem->is_mmiomap == 1)  */
    {
#if 0
      strcpy(mem->memname, "/mmio.");
      strcat(mem->memname, get_timestamp(0) );
#endif
      sprintf(mem->memname, "/mmio.%s", get_timestamp(0));
    /* #ifdef ASE_DEBUG */
    /*   printf("  [DEBUG] memname => %s\n", mem->memname); */
    /* #endif       */
      /* mem->is_mmiomap = 1; */
    }
  else if (mem->is_umas == 1) 
    {
#if 0
      strcpy(mem->memname, "/umas.");
      strcat(mem->memname, get_timestamp(0) );
#endif
      sprintf(mem->memname, "/umas.%s", get_timestamp(0));
    /* #ifdef ASE_DEBUG */
    /*   printf("  [DEBUG] memname => %s\n", mem->memname); */
    /* #endif             */
    }
  else
    {
#if 0
      sprintf(mem->memname, "/buf%d.", buffer_index_count);
      strcat(mem->memname, get_timestamp(0) );
#endif
      sprintf(mem->memname, "/buf%d.%s", buffer_index_count, get_timestamp(0));
      /* mem->is_mmiomap = 0; */
    }

  // Disable private memory flag
  mem->is_privmem = 0;

  // Obtain a file descriptor for the shared memory region
  // Tue May  5 19:24:21 PDT 2015
  // https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
  // S_IREAD | S_IWRITE are obselete
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
      mmio_afu_vbase = (uint64_t*)mem->vbase;
    #ifdef ASE_DEBUG
      printf("  [APP]  ASE MMIO virtual base = %p\n", mmio_afu_vbase);
    #endif
    }

  // Extend memory to required size
  ftruncate(mem->fd_app, (off_t)mem->memsize);

  // Set mmio_afu_vbase
  //  if (buffer_index_count == 0)
  // mmio_afu_vbase = (uint32_t*)mem->vbase;

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
/* #ifdef ASE_MEMTEST_ENABLE */
/*   shm_dbg_memtest(mem); */
/* #endif */

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
  // memcpy(tmp_msg, (char*)mem, sizeof(struct buffer_t));
  mqueue_send(app2sim_tx, tmp_msg);

  // Receive message from DPI with pbase populated
  while(mqueue_recv(sim2app_rx, tmp_msg)==0) { /* wait */ }
  ase_str_to_buffer_t(tmp_msg, mem);

  // Print out the buffer
#ifdef ASE_BUFFER_VIEW
  ase_buffer_info(mem);
#endif

  // book-keeping WSmeta
  struct wsmeta_t *ws;
  ws = (struct wsmeta_t *) malloc(sizeof(struct wsmeta_t));
  ws->index = mem->index;
  ws->buf_structaddr = (uint64_t*)mem;  
  append_wsmeta(ws);

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
  mq_name = ase_malloc (ASE_MQ_NAME_LEN);
  memset(mq_name, '\0', ASE_MQ_NAME_LEN);

#if 0
  ase_buffer_info(mem);
#endif 

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocating memory region %s ...", mem->memname);
  END_YELLOW_FONTCOLOR;
  // usleep(50000);                                   // Short duration wait for sanity

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
 * Appends and maintains a Workspace Meta Linked List (wsmeta_t)
 * <index, buffer_t<vaddr>> linkedlist
 */
void append_wsmeta(struct wsmeta_t *new)
{
  FUNC_CALL_ENTRY;

  if (wsmeta_head == NULL) 
    {
      wsmeta_head = new;
      wsmeta_end = new;
    }
  
  wsmeta_end->next = new;
  new->next = NULL;
  wsmeta_end = new;

#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  struct wsmeta_t *wsptr;
  printf("WSMeta traversal START =>\n");
  wsptr = wsmeta_head;
  while(wsptr != NULL)
    {
      printf("\t%d %p\n", wsptr->index, wsptr->buf_structaddr );
      wsptr = wsptr->next;
    }
  printf("WSMeta traversal END\n");
  END_YELLOW_FONTCOLOR;
#endif

  FUNC_CALL_EXIT;
}


/*
 * deallocate_buffer_by_index:
 * Find a workspace by ID and then call deallocate_buffer
 */
void deallocate_buffer_by_index(int search_index)
{
  FUNC_CALL_ENTRY;

  int wsid;
  uint64_t *bufptr = (uint64_t*) NULL;
  struct wsmeta_t *wsptr;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocate request index = %d ... ", search_index);

  // Traverse wsmeta_t
  wsptr = wsmeta_head;
  while (wsptr != NULL)
    {
      if (wsptr->index == search_index)
	{
	  wsid = wsptr->index;
	  bufptr = wsptr->buf_structaddr;
	  printf("FOUND\n");
	  break;
	}
      else
	{
	  wsptr = wsptr->next;
	}
    }
  END_YELLOW_FONTCOLOR;
  
  // Call deallocate
  if (bufptr != NULL)    
    deallocate_buffer((struct buffer_t *)bufptr);
  else
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Buffer pointer was returned as NULL\n");
      END_RED_FONTCOLOR;
    }

  FUNC_CALL_EXIT;
}


/*
 * UMSG Get Address
 * umsg_get_address: Takes in umsg_id, and returns App virtual address
 */
uint64_t* umsg_get_address(int umsg_id) 
{
  uint64_t* ret_vaddr;
  if ((umsg_id >= 0) || (umsg_id < NUM_UMSG_PER_AFU))
    {
      ret_vaddr = (uint64_t*)( (uint64_t)umsg_umas_vbase + (uint64_t)(umsg_id*(ASE_PAGESIZE + 64)) );
    }
  else
    {
      ret_vaddr = NULL;
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested umsg_id out of range... returning NULL\n");
      END_RED_FONTCOLOR;
    }
  return ret_vaddr;
}


/*
 * umsg_send: Send Umsg
 */
void umsg_send (int umsg_id, uint64_t *umsg_data)
{
  FUNC_CALL_ENTRY;

  umsgcmd_t *umsg_pkt;
    
  umsg_pkt = (struct umsgcmd_t *)ase_malloc( sizeof(struct umsgcmd_t) );
  memset((char*)umsg_pkt, '\0', sizeof(struct umsgcmd_t) );

  umsg_pkt->id = umsg_id;
  memcpy((char*)umsg_pkt->qword, (char*)umsg_data, sizeof(uint64_t));

  /* char *umsg_str; */
  /* umsg_str = ase_malloc(ASE_MQ_MSGSIZE); */

  // Send Umsg packet to simulator
  mqueue_send(app2sim_umsg_tx, (char*)umsg_pkt );

  FUNC_CALL_EXIT;
}


// void umsg_set_attribute

/*
 * ase_portctrl: Send port control message to simulator
 * 
 * AFU_RESET
 * UMSG_MODE <mode_bits>[7:0]
 * 
 * ## WARNING ##: Do not remove __attribute__ optimization control
 * The extra delay is required for portctrl command to be parsed 
 * by simulator. Removing this CAN have unintended program control 
 * or race conditions
 *
 */
void __attribute__((optimize("O0"))) ase_portctrl(char *ctrl_msg)
{
  mqueue_send(app2sim_portctrl_tx, ctrl_msg);

  // Allow simulator to parse message and sort itself out
  usleep(1000);
}


//////////////////////////////////////////////////////////////////////////////////////

// struct buffer_t *mem 

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
#if 0
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
#endif

/*
 * umas_init : Set up UMAS region
 *             Create a ASE_PAGESIZE * 4KB Umsg region
 * Requires buffer_t handles to UMAS and CSR regions
 */
#if 0
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
      umas = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
      umas->memsize = NUM_UMSG_PER_AFU * ASE_PAGESIZE;
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
#endif

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
#if 0
void umsg_send(int umas_id, char *umsg_data)
{

  uint64_t *umas_target_addr;
  char umsg_str[SIZEOF_UMSG_PACK_T];
  /* uint32_t umsg_hint; */
  umsg_pack_t inst;
  /* int ii; */

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
  // umsg_hint = glbl_umsgmode_csr & (0x0 || (1 << umas_id));  
  inst.id = umas_id;
  inst.hint = glbl_umsgmode_csr & (0x0 || (1 << umas_id));  
  memcpy(inst.data, umsg_data, CL_BYTE_WIDTH);

  // MQ Send to SIM
  // memset(umsg_str, '\0',ASE_MQ_MSGSIZE );
  memcpy(umsg_str, &inst, SIZEOF_UMSG_PACK_T);
  /* printf("umsg_str =>\n"); */
  /* for(ii = 0 ; ii < SIZEOF_UMSG_PACK_T; ii++) */
  /*   printf("%02X", (int)umsg_str[ii]); */
  /* printf("\n"); */

  mqueue_send(app2sim_umsg_tx, umsg_str);
}
#endif

/*
 * umas_deinit : Deinitialize UMAS region
 *               Deallocate region and unlink
 */
#if 0
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
#endif 

