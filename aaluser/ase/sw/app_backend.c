// Copyright(c) 2014-2016, Intel Corporation
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
#define _GNU_SOURCE

#include "ase_common.h"

// Lock
pthread_mutex_t app_lock;

// CSR Map
uint32_t mmio_write_cnt = 0;
uint32_t mmio_read_cnt = 0;

// MQ established
uint32_t mq_exist_status = MQ_NOT_ESTABLISHED;

// Umsg and MMIO statuses
uint32_t mmio_exist_status = NOT_ESTABLISHED;
uint32_t umas_exist_status = NOT_ESTABLISHED;

// CSR map storage
struct buffer_t *mmio_region;

// UMAS region
struct buffer_t *umas_region;

// Workspace metadata table
struct wsmeta_t *wsmeta_head = (struct wsmeta_t *) NULL; 
struct wsmeta_t *wsmeta_end = (struct wsmeta_t *) NULL;  

// Buffer index count
int asebuf_index_count = 0;    // global count/index
int userbuf_index_count = 0;   // User count/index

// Timestamp char array
char *tstamp_string;

#ifdef ASE_DEBUG
FILE *fp_pagetable_log = (FILE *)NULL;
#endif

/*
 * Send SIMKILL
 */
void send_simkill()
{
  // Simkill
  char ase_simkill_msg[ASE_MQ_MSGSIZE];
  memset(ase_simkill_msg, 0, ASE_MQ_MSGSIZE);
  snprintf(ase_simkill_msg, ASE_MQ_MSGSIZE, "%u", ASE_SIMKILL_MSG);
  mqueue_send(app2sim_simkill_tx, ase_simkill_msg, ASE_MQ_MSGSIZE);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  CTRL-C was seen... SW application will exit\n");
  END_YELLOW_FONTCOLOR;

  // MQ close
  mqueue_close(app2sim_mmioreq_tx);
  mqueue_close(sim2app_mmiorsp_rx);
  mqueue_close(app2sim_alloc_tx);
  mqueue_close(sim2app_alloc_rx);
  mqueue_close(app2sim_umsg_tx);
  mqueue_close(app2sim_simkill_tx);
  mqueue_close(app2sim_portctrl_req_tx); 
  mqueue_close(app2sim_dealloc_tx);
  mqueue_close(sim2app_dealloc_rx);
  mqueue_close(sim2app_portctrl_rsp_rx);

  exit(0);
}


/*
 * Session Initialize
 * Open the message queues to ASE simulator
 */
void session_init()
{
  FUNC_CALL_ENTRY;

  setvbuf(stdout, NULL, (int)_IONBF, (size_t)0);

  ipc_init();

  // Initialize lock
  if ( pthread_mutex_init(&app_lock, NULL) != 0)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Lock initialization failed, EXIT\n");
      END_YELLOW_FONTCOLOR;
      exit (EXIT_FAILURE);
    }
  
  // Initialize ase_workdir_path
  // ase_workdir_path = ase_malloc(ASE_FILEPATH_LEN);
  // ase_workdir_path = ase_eval_session_directory();  
  // ase_eval_session_directory ();  
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

  // Ignore SIGPIPE *FIXME*: Look for more elegant solution
  signal(SIGPIPE, SIG_IGN);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Initializing simulation session ... \n");
  END_YELLOW_FONTCOLOR;

  app2sim_alloc_tx    = mqueue_open( mq_array[0].name, mq_array[0].perm_flag );
  app2sim_mmioreq_tx  = mqueue_open( mq_array[1].name, mq_array[1].perm_flag );
  app2sim_umsg_tx     = mqueue_open( mq_array[2].name, mq_array[2].perm_flag );
  app2sim_simkill_tx  = mqueue_open( mq_array[3].name, mq_array[3].perm_flag );
  sim2app_alloc_rx    = mqueue_open( mq_array[4].name, mq_array[4].perm_flag );
  sim2app_mmiorsp_rx  = mqueue_open( mq_array[5].name, mq_array[5].perm_flag );
  app2sim_portctrl_req_tx = mqueue_open( mq_array[6].name, mq_array[6].perm_flag );
  app2sim_dealloc_tx  = mqueue_open( mq_array[7].name, mq_array[7].perm_flag );
  sim2app_dealloc_rx  = mqueue_open( mq_array[8].name, mq_array[8].perm_flag );
  sim2app_portctrl_rsp_rx = mqueue_open( mq_array[9].name, mq_array[9].perm_flag );

#ifdef ASE_DEBUG
  // Page table tracker
  fp_pagetable_log = fopen("app_pagetable.log", "w");
  if (fp_pagetable_log == NULL) 
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  APP pagetable logger initialization failed !\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  APP pagetable logger initialized\n");
      END_YELLOW_FONTCOLOR;
    }
#endif

  // Message queues have been established
  mq_exist_status = MQ_ESTABLISHED;

  BEGIN_YELLOW_FONTCOLOR;

  // Session start
  printf(" DONE\n");
  printf("  [APP]  Session started\n");

  // Send portctrl command to start a session
  char session_ctrlcmd[ASE_MQ_MSGSIZE];
  memset(session_ctrlcmd, 0, ASE_MQ_MSGSIZE);
  sprintf(session_ctrlcmd, "ASE_INIT %d", getpid());
  ase_portctrl(session_ctrlcmd);

  // Wait till session file is created
  poll_for_session_id();
  tstamp_string = (char*) ase_malloc(20);
  tstamp_string = get_timestamp(0);

  // Creating CSR map 
  printf("  [APP]  Creating MMIO ...\n");
  mmio_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  memset(mmio_region, 0, sizeof(struct buffer_t));
  mmio_region->memsize = MMIO_LENGTH;
  mmio_region->is_mmiomap = 1;  
  allocate_buffer(mmio_region, NULL);
  mmio_afu_vbase = (uint64_t*)((uint64_t)mmio_region->vbase + MMIO_AFU_OFFSET);
  mmio_exist_status = ESTABLISHED;
  printf("  [APP]  AFU MMIO Virtual Base Address = %p\n", (void*) mmio_afu_vbase); 

  // Create UMSG region
  printf("  [APP]  Creating UMAS ... \n");
  umas_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  memset(umas_region, 0, sizeof(struct buffer_t));
  umas_region->memsize = UMAS_LENGTH;
  umas_region->is_umas = 1;
  allocate_buffer(umas_region, NULL);
  umsg_umas_vbase = (uint64_t*)((uint64_t)umas_region->vbase);
  umas_exist_status = ESTABLISHED;
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
  if (umas_exist_status == ESTABLISHED) 
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Deallocating UMAS\n");
      deallocate_buffer(umas_region);
      END_YELLOW_FONTCOLOR;
    }
#ifdef ASE_DEBUG
  else
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  No UMAS established\n");
      END_RED_FONTCOLOR;
    }
#endif

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
  memset(ase_simkill_msg, 0, ASE_MQ_MSGSIZE);
  sprintf(ase_simkill_msg, "%u", ASE_SIMKILL_MSG);
  mqueue_send(app2sim_simkill_tx, ase_simkill_msg, ASE_MQ_MSGSIZE);
  
#ifdef ASE_DEBUG
  fclose(fp_pagetable_log);
#endif

  mqueue_close(app2sim_mmioreq_tx);
  mqueue_close(sim2app_mmiorsp_rx);
  mqueue_close(app2sim_alloc_tx);
  mqueue_close(sim2app_alloc_rx);
  mqueue_close(app2sim_umsg_tx);
  mqueue_close(app2sim_simkill_tx);
  mqueue_close(app2sim_portctrl_req_tx);
  mqueue_close(app2sim_dealloc_tx);
  mqueue_close(sim2app_dealloc_rx);


  BEGIN_YELLOW_FONTCOLOR;
  printf(" DONE\n");
  printf("  [APP]  Session ended\n");
  END_YELLOW_FONTCOLOR;

  free(umas_region);
  free(mmio_region);
  // free(ase_workdir_path);
  
  // Lock deinit
  pthread_mutex_destroy(&app_lock);

  FUNC_CALL_EXIT;
}


/*
 * MMIO Request call
 */
void mmio_request_put(struct mmio_t *pkt)
{
  FUNC_CALL_ENTRY;

  mqueue_send( app2sim_mmioreq_tx, (char*)pkt, sizeof(mmio_t) );

  FUNC_CALL_EXIT;
}


/*
 * MMIO Response get
 */
void mmio_response_get(struct mmio_t *pkt)
{
  FUNC_CALL_ENTRY;

  mqueue_recv( sim2app_mmiorsp_rx, (char*)pkt, sizeof(mmio_t) );

  FUNC_CALL_EXIT;
}

/*
 * MMIO Write 32-bit
 */
void mmio_write32 (uint32_t offset, uint32_t data)
{
  FUNC_CALL_ENTRY;
  
  // pthread_mutex_lock (&app_lock);

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Write\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      mmio_pkt->write_en = MMIO_WRITE_REQ;
      mmio_pkt->width = MMIO_WIDTH_32;
      mmio_pkt->addr = offset;
      memcpy(mmio_pkt->qword, &data, sizeof(uint32_t));
      mmio_pkt->resp_en = 0;

      uint32_t *mmio_vaddr;
      mmio_vaddr = (uint32_t*)((uint64_t)mmio_afu_vbase + offset);

      // Message
      mmio_request_put(mmio_pkt);
      memcpy(mmio_vaddr, (char*)&data, sizeof(uint32_t));

      // Display
      mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write #%d : offset = 0x%x, data = 0x%08x\n", mmio_write_cnt, offset, data);
      END_YELLOW_FONTCOLOR;
      free(mmio_pkt);
    }  


  FUNC_CALL_EXIT;
}


/*
 * MMIO Write 64-bit
 */
void mmio_write64 (uint32_t offset, uint64_t data)
{
  FUNC_CALL_ENTRY;

  // pthread_mutex_lock (&app_lock);

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Write\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      mmio_pkt->write_en = MMIO_WRITE_REQ;
      mmio_pkt->width = MMIO_WIDTH_64;
      mmio_pkt->addr = offset;
      memcpy(mmio_pkt->qword, &data, sizeof(uint64_t));
      mmio_pkt->resp_en = 0;

      uint64_t *mmio_vaddr;
      mmio_vaddr = (uint64_t*)((uint64_t)mmio_afu_vbase + offset);

      // Message
      mmio_request_put(mmio_pkt);
      *mmio_vaddr = data;

      mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write #%d : offset = 0x%x, data = 0x%llx\n", mmio_write_cnt, offset, (unsigned long long)data);
      END_YELLOW_FONTCOLOR;

      free(mmio_pkt);
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
void mmio_read32(uint32_t offset, uint32_t *data32)
{
  FUNC_CALL_ENTRY;

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Read\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      mmio_pkt->write_en = MMIO_READ_REQ;
      mmio_pkt->width = MMIO_WIDTH_32;
      mmio_pkt->addr = offset;
      mmio_pkt->resp_en = 0;

      // Messaging
      mmio_request_put(mmio_pkt);
      mmio_response_get(mmio_pkt);

      // Display
      mmio_read_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Read #%d  : offset = 0x%x\n", mmio_read_cnt, offset);
      
      // Write data
      // data = (uint32_t*)((uint64_t)mmio_afu_vbase + offset);
      *data32 = (uint32_t)mmio_pkt->qword[0];
      
      /* #ifdef ASE_DEBUG */
      /*       BEGIN_YELLOW_FONTCOLOR; */
      /*       printf("  [DEBUG]  mmio_response =>\n"); */
      /*       printf("  [DEBUG]  width=%d, addr=%x, resp_en=%d\n", mmio_pkt->width, mmio_pkt->addr, mmio_pkt->resp_en); */
      /*       printf("  [DEBUG]  data=%llx\n", mmio_pkt->qword[0]); */
      /*       printf("  [DEBUG]  *data=%08x\n", (uint32_t)*data32); */
      /*       END_YELLOW_FONTCOLOR; */
      /* #endif */

      printf("  [APP]  MMIO Read Resp : %08x\n", (uint32_t)*data32);
      END_YELLOW_FONTCOLOR;
      free(mmio_pkt);
    }

  FUNC_CALL_EXIT;
}


/*
 * MMIO Read 64-bit
 */
void mmio_read64(uint32_t offset, uint64_t *data64)
{
  FUNC_CALL_ENTRY;

  if (offset < 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  Requested offset is not in AFU MMIO region\n");
      printf("         Ignoring MMIO Read\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );

      mmio_pkt->write_en = MMIO_READ_REQ;
      mmio_pkt->width = MMIO_WIDTH_64;
      mmio_pkt->addr = offset;
      mmio_pkt->resp_en = 0;

      // Messaging
      mmio_request_put(mmio_pkt);
      mmio_response_get(mmio_pkt);

      // Display
      mmio_read_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Read #%d  : offset = 0x%x\n", mmio_read_cnt, offset);
      
      // Write data
      // data = (uint64_t*)((uint64_t)mmio_afu_vbase + offset);
      *data64 = mmio_pkt->qword[0];

/* #ifdef ASE_DEBUG */
/*       BEGIN_YELLOW_FONTCOLOR; */
/*       printf("  [DEBUG]  mmio_response =>\n"); */
/*       printf("  [DEBUG]  width=%d, addr=%x, resp_en=%d\n", mmio_pkt->width, mmio_pkt->addr, mmio_pkt->resp_en); */
/*       printf("  [DEBUG]  data=%llx\n", mmio_pkt->qword[0]); */
/*       END_YELLOW_FONTCOLOR; */
/* #endif */
      printf("  [APP]  MMIO Read Resp : %llx\n", (unsigned long long)*data64);
      END_YELLOW_FONTCOLOR;
      free(mmio_pkt);
    }

  FUNC_CALL_EXIT;
}


/*
 * allocate_buffer: Shared memory allocation and vbase exchange
 * Instantiate a buffer_t structure with given parameters
 * Must be called by ASE_APP
 */
void allocate_buffer(struct buffer_t *mem, uint64_t *suggested_vaddr)
{
  FUNC_CALL_ENTRY;

  // pthread_mutex_lock (&app_lock);

  char tmp_msg[ASE_MQ_MSGSIZE]  = { 0, };

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
  memset(mem->memname, 0, sizeof(mem->memname));  
  if(mem->is_mmiomap == 1)
    {
      sprintf(mem->memname, "/mmio.%s", tstamp_string);
    }
  else if (mem->is_umas == 1) 
    {
      sprintf(mem->memname, "/umas.%s", tstamp_string);
    }
  else
    {
      sprintf(mem->memname, "/buf%d.%s", userbuf_index_count, tstamp_string);
      userbuf_index_count++;
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
      perror("shm_open");
      exit(1);
    }
      

  // Mmap shared memory region
  if (suggested_vaddr == (uint64_t*) NULL)
    {
      mem->vbase = (uint64_t) mmap(NULL, mem->memsize, PROT_READ|PROT_WRITE, MAP_SHARED, mem->fd_app, 0);
    }
  else
    {
      mem->vbase = (uint64_t) mmap(suggested_vaddr, mem->memsize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, mem->fd_app, 0);
    }

  // Check
  if(mem->vbase == (uint64_t) MAP_FAILED)
    {
      perror("mmap");
      exit(1);
    }
  
  
  // Extend memory to required size
  ftruncate(mem->fd_app, (off_t)mem->memsize);

  // Autogenerate buffer index
  mem->index = asebuf_index_count;
  asebuf_index_count++;
  BEGIN_YELLOW_FONTCOLOR;
  printf("SUCCESS\n");
  END_YELLOW_FONTCOLOR;

  // Set buffer as valid
  mem->valid = ASE_BUFFER_VALID;

  // Send an allocate command to DPI, metadata = ASE_MEM_ALLOC
  mem->metadata = HDR_MEM_ALLOC_REQ;
  mem->next = NULL;

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
  mqueue_send(app2sim_alloc_tx, tmp_msg, ASE_MQ_MSGSIZE);

  // Receive message from DPI with pbase populated
  while(mqueue_recv(sim2app_alloc_rx, tmp_msg, ASE_MQ_MSGSIZE)==0) { /* wait */ }
  ase_str_to_buffer_t(tmp_msg, mem);

  // Print out the buffer
#ifdef ASE_BUFFER_VIEW
  ase_buffer_info(mem);
#endif

  // book-keeping WSmeta // used by ASEALIAFU
  struct wsmeta_t *ws;
  ws = (struct wsmeta_t *) ase_malloc(sizeof(struct wsmeta_t));
  ws->index = mem->index;
  ws->buf_structaddr = (uint64_t*)mem;  
  append_wsmeta(ws);

  // pthread_mutex_unlock(&app_lock);

#ifdef ASE_DEBUG  
  if (fp_pagetable_log != NULL) 
    {
      if (mem->index % 20 == 0) 
	{
	  fprintf(fp_pagetable_log, 
		  "Index\tfd_app\tfd_ase\tAppVBase\tASEVBase\tBufsize\tBufname\t\tPhysBase\n");
	}
      
      fprintf(fp_pagetable_log, 
	      "%d\t%d\t%d\t%p\t%p\t%x\t%s\t\t%p\n",
	      mem->index,
	      mem->fd_app,
	      mem->fd_ase,
	      (void*)mem->vbase, 
	      (void*)mem->pbase,
	      mem->memsize,
	      mem->memname,
	      (void*)mem->fake_paddr
	      );
    }
#endif

  close(mem->fd_app);

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

  // pthread_mutex_lock (&app_lock);

  int ret;
  char tmp_msg[ASE_MQ_MSGSIZE] = { 0, };
  /* char *mq_name; */
  /* mq_name = ase_malloc (ASE_MQ_NAME_LEN); */
  /* memset(mq_name, 0, ASE_MQ_NAME_LEN); */

#if 0
  ase_buffer_info(mem);
#endif 

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Deallocating memory %s ...", mem->memname);
  END_YELLOW_FONTCOLOR;

  // Send buffer with metadata = HDR_MEM_DEALLOC_REQ
  mem->metadata = HDR_MEM_DEALLOC_REQ;

  // Send a one way message to request a deallocate
  ase_buffer_t_to_str(mem, tmp_msg);
  mqueue_send(app2sim_dealloc_tx, tmp_msg, ASE_MQ_MSGSIZE);

  // Wait for response to deallocate
  mqueue_recv(sim2app_dealloc_rx, tmp_msg, ASE_MQ_MSGSIZE);
  ase_str_to_buffer_t(tmp_msg, mem);
  
  // Unmap the memory accordingly
  ret = munmap((void*)mem->vbase, (size_t)mem->memsize);
  if(0 != ret)
    {
      perror("munmap");
      exit(1);
    }

  //  close(mem->fd_app);
  free(mem);

  // Print if successful
  BEGIN_YELLOW_FONTCOLOR;
  printf("SUCCESS\n");
  END_YELLOW_FONTCOLOR;

  // pthread_mutex_unlock (&app_lock);

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

  // pthread_mutex_lock (&app_lock);

  //int wsid;
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
	  //wsid = wsptr->index;
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

  // pthread_mutex_unlock (&app_lock);

  FUNC_CALL_EXIT;
}


/*
 * UMSG Get Address
 * umsg_get_address: Takes in umsg_id, and returns App virtual address
 */
uint64_t* umsg_get_address(int umsg_id) 
{
  uint64_t* ret_vaddr;
  if ((umsg_id >= 0) && (umsg_id < NUM_UMSG_PER_AFU))
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
  memset((char*)umsg_pkt, 0, sizeof(struct umsgcmd_t) );

  umsg_pkt->id = umsg_id;
  memcpy((char*)umsg_pkt->qword, (char*)umsg_data, sizeof(uint64_t));

  // Send Umsg packet to simulator
  mqueue_send(app2sim_umsg_tx, (char*)umsg_pkt, sizeof(struct umsgcmd_t));

  FUNC_CALL_EXIT;
}


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
void __attribute__((optimize("O0"))) ase_portctrl(const char *ctrl_msg)
{
  char dummy_rxstr[ASE_MQ_MSGSIZE];
  memset(dummy_rxstr, 0, ASE_MQ_MSGSIZE);

  // Send message
  mqueue_send(app2sim_portctrl_req_tx, ctrl_msg, ASE_MQ_MSGSIZE);

  // Receive message
  mqueue_recv(sim2app_portctrl_rsp_rx, dummy_rxstr, ASE_MQ_MSGSIZE);

  // Allow simulator to parse message and sort itself out
  usleep(1000);
}


