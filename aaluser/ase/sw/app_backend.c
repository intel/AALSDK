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
// pthread_mutex_t app_lock;
pthread_mutex_t mmio_lock;

// CSR Map
/* uint32_t mmio_write_cnt = 0; */
/* uint32_t mmio_read_cnt = 0; */

// MQ established
uint32_t mq_exist_status = MQ_NOT_ESTABLISHED;

// Umsg and MMIO statuses
uint32_t mmio_exist_status = NOT_ESTABLISHED;
uint32_t umas_exist_status = NOT_ESTABLISHED;

// Session status
uint32_t session_exist_status = NOT_ESTABLISHED;

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
 * MMIO Read response watcher
 */
// MMIO Tid
int glbl_mmio_tid;
pthread_mutex_t mmio_tid_lock;

// Tracker thread Id
pthread_t mmio_watch_tid;

// MMIO Response packet handoff control
mmio_t *mmio_rsp_pkt;
volatile int mmio_rsp_pkt_available;
volatile int mmio_rsp_pkt_accepted;

/*
 * MSI-X watcher 
 */
// MSI-X watcher 
pthread_t msix_watch_tid;


/*
 * MMIO Generate TID
 * - Creation of TID must be atomic
 */
uint32_t generate_mmio_tid()
{
  // *FIXME*: TID credit must not overrun, no more than 512 outstanding MMIO Requests

  // Return value
  uint32_t ret_mmio_tid;

  // Lock access to resource
  pthread_mutex_lock(&mmio_tid_lock);

  // Increment and mask
  // __asm__ __volatile__("lock add $1, %0" : "+r"(glbl_mmio_tid));
  ret_mmio_tid = glbl_mmio_tid & MMIO_TID_BITMASK;
  glbl_mmio_tid++;

  // Unlock access to resource
  pthread_mutex_unlock(&mmio_tid_lock);

  // Return ID
  return ret_mmio_tid;
}


/*
 * THREAD: MMIO Read thread watcher
 */
void *mmio_response_watcher()
{
  mmio_rsp_pkt = (struct mmio_t *)ase_malloc( sizeof(struct mmio_t) );
  int ret;

  // start watching for messages
  while(1)
    {
      // Set available/accepted flag to 0
      mmio_rsp_pkt_accepted = 0;
      mmio_rsp_pkt_available = 0;

      // If received, update global message
      ret = mqueue_recv( sim2app_mmiorsp_rx, (char*)mmio_rsp_pkt, sizeof(mmio_t) );
      if (ret == ASE_MSG_PRESENT) 
	{
	  // Mark as available
	  mmio_rsp_pkt_available = 1;
	  
	  // Wait until ACK from requesting thread
	  while (mmio_rsp_pkt_accepted != 1)
	    {
	      usleep(500);
	      /* printf("Waiting for mmio_rsp_pkt_accepted == 1\n"); */
	      /* // usleep(500000); */
	      /* sleep(1); */
	    }
	}
    }
}


/*
 * Interrupt request (FPGA->CPU) watcher
 */

/*
 * Send SIMKILL
 */
void send_simkill()
{
  // Simkill
  char ase_simkill_msg[ASE_MQ_MSGSIZE];
  memset(ase_simkill_msg, 0, ASE_MQ_MSGSIZE);
  sprintf(ase_simkill_msg, "ASE_SIMKILL 0");
  ase_portctrl(ase_simkill_msg);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  CTRL-C was seen... SW application will exit\n");
  END_YELLOW_FONTCOLOR;

  // MQ close
  mqueue_close(app2sim_mmioreq_tx);
  mqueue_close(sim2app_mmiorsp_rx);
  mqueue_close(app2sim_alloc_tx);
  mqueue_close(sim2app_alloc_rx);
  mqueue_close(app2sim_umsg_tx);
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
  /* if ( pthread_mutex_init(&app_lock, NULL) != 0) */
  /*   { */
  /*     BEGIN_YELLOW_FONTCOLOR; */
  /*     printf("  [APP]  Lock initialization failed, EXIT\n"); */
  /*     END_YELLOW_FONTCOLOR; */
  /*     exit (EXIT_FAILURE); */
  /*   } */

  if ( pthread_mutex_init(&mmio_lock, NULL) != 0)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Lock initialization failed, EXIT\n");
      END_YELLOW_FONTCOLOR;
      exit (EXIT_FAILURE);
    }

  if ( pthread_mutex_init(&mmio_tid_lock, NULL) != 0)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO TID Lock initialization failed, EXIT\n");
      END_YELLOW_FONTCOLOR;
      exit (EXIT_FAILURE);
    }

  // Initialize ase_workdir_path
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  ASE Session Directory located at =>\n");
  printf("         %s\n", ase_workdir_path);
  END_YELLOW_FONTCOLOR;

  // Register kill signals
  signal(SIGTERM, send_simkill);
  signal(SIGINT , send_simkill);
  signal(SIGQUIT, send_simkill);
  signal(SIGHUP,  send_simkill);

  // Ignore SIGPIPE *FIXME*: Look for more elegant solution
  signal(SIGPIPE, SIG_IGN);

  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Initializing simulation session ... \n");
  END_YELLOW_FONTCOLOR;

  app2sim_alloc_tx        = mqueue_open( mq_array[0].name, mq_array[0].perm_flag );
  app2sim_mmioreq_tx      = mqueue_open( mq_array[1].name, mq_array[1].perm_flag );
  app2sim_umsg_tx         = mqueue_open( mq_array[2].name, mq_array[2].perm_flag );
  sim2app_alloc_rx        = mqueue_open( mq_array[3].name, mq_array[3].perm_flag );
  sim2app_mmiorsp_rx      = mqueue_open( mq_array[4].name, mq_array[4].perm_flag );
  app2sim_portctrl_req_tx = mqueue_open( mq_array[5].name, mq_array[5].perm_flag );
  app2sim_dealloc_tx      = mqueue_open( mq_array[6].name, mq_array[6].perm_flag );
  sim2app_dealloc_rx      = mqueue_open( mq_array[7].name, mq_array[7].perm_flag );
  sim2app_portctrl_rsp_rx = mqueue_open( mq_array[8].name, mq_array[8].perm_flag );

  // Message queues have been established
  mq_exist_status = MQ_ESTABLISHED;

  // Page table tracker (optional logger)
#ifdef ASE_DEBUG
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
  BEGIN_YELLOW_FONTCOLOR;

  // Set MMIO Tid to 0
  glbl_mmio_tid = 0;

  // Thread error integer
  int thr_err;

  // Start MMIO watcher thread
  printf("  [APP]  Starting MMIO Read watcher ... ");
  thr_err = pthread_create (&mmio_watch_tid, NULL, &mmio_response_watcher, NULL);
  if (thr_err != 0)
    {
      BEGIN_RED_FONTCOLOR;
      printf("FAILED\n");
      perror("pthread_create");
      exit(1);
      END_RED_FONTCOLOR;
    }
  else
    {
      printf("SUCCESS\n");
    }

  // Start MSI-X watcher thread
  // printf("  [APP]  Starting Interrupt watcher ... \n");

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
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Creating MMIO ...\n");
  END_YELLOW_FONTCOLOR;
  mmio_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  memset(mmio_region, 0, sizeof(struct buffer_t));
  mmio_region->memsize = MMIO_LENGTH;
  mmio_region->is_mmiomap = 1;
  allocate_buffer(mmio_region, NULL);
  mmio_afu_vbase = (uint64_t*)((uint64_t)mmio_region->vbase + MMIO_AFU_OFFSET);
  mmio_exist_status = ESTABLISHED;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  AFU MMIO Virtual Base Address = %p\n", (void*) mmio_afu_vbase);
  END_YELLOW_FONTCOLOR;

  // Create UMSG region
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  Creating UMAS ... \n");
  END_YELLOW_FONTCOLOR;
  umas_region = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));
  memset(umas_region, 0, sizeof(struct buffer_t));
  umas_region->memsize = UMAS_LENGTH;
  umas_region->is_umas = 1;
  allocate_buffer(umas_region, NULL);
  umsg_umas_vbase = (uint64_t*)((uint64_t)umas_region->vbase);
  umas_exist_status = ESTABLISHED;
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [APP]  UMAS Virtual Base address = %p\n", (void*)umsg_umas_vbase);
  END_YELLOW_FONTCOLOR;

  // Session status
  session_exist_status = ESTABLISHED;

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

  int thr_err;

  if (session_exist_status == ESTABLISHED)
    {
      // Unmap UMAS region
      if (umas_exist_status == ESTABLISHED)
	{
	  BEGIN_YELLOW_FONTCOLOR;
	  printf("  [APP]  Deallocating UMAS\n");
	  deallocate_buffer(umas_region);
	  END_YELLOW_FONTCOLOR;
	  umas_exist_status = NOT_ESTABLISHED;
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
      mmio_exist_status = NOT_ESTABLISHED;

      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Deinitializing simulation session ... ");
      END_YELLOW_FONTCOLOR;

      // Send SIMKILL
      char session_ctrlcmd[ASE_MQ_MSGSIZE];
      memset(session_ctrlcmd, 0, ASE_MQ_MSGSIZE);
      sprintf(session_ctrlcmd, "ASE_SIMKILL 0");
      ase_portctrl(session_ctrlcmd);

#ifdef ASE_DEBUG
      fclose(fp_pagetable_log);
#endif

      // Close MMIO Response tracker thread
      pthread_kill(mmio_watch_tid, SIGINT);

      // close message queue
      mqueue_close(app2sim_mmioreq_tx);
      mqueue_close(sim2app_mmiorsp_rx);
      mqueue_close(app2sim_alloc_tx);
      mqueue_close(sim2app_alloc_rx);
      mqueue_close(app2sim_umsg_tx);
      mqueue_close(app2sim_portctrl_req_tx);
      mqueue_close(app2sim_dealloc_tx);
      mqueue_close(sim2app_dealloc_rx);
      mqueue_close(sim2app_portctrl_rsp_rx);

      BEGIN_YELLOW_FONTCOLOR;
      printf(" DONE\n");
      printf("  [APP]  Session ended\n");
      END_YELLOW_FONTCOLOR;

      /* free(umas_region); */
      /* free(mmio_region); */
      // free(ase_workdir_path);

      // Lock deinit
      // pthread_mutex_destroy(&app_lock);
      pthread_mutex_destroy(&mmio_lock);
      pthread_mutex_destroy(&mmio_tid_lock);
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  Session already deinitialized, call ignored !\n");
      END_YELLOW_FONTCOLOR;
    }

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

      mmio_pkt->tid = generate_mmio_tid();
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
      // mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write     : tid = 0x%03x, offset = 0x%x, data = 0x%08x\n", mmio_pkt->tid, mmio_pkt->addr, data);
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

      mmio_pkt->tid= generate_mmio_tid();
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

      // mmio_write_cnt++;
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Write     : tid = 0x%03x, offset = 0x%x, data = 0x%llx\n", mmio_pkt->tid, mmio_pkt->addr, (unsigned long long)data);
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

      mmio_pkt->tid      = generate_mmio_tid();
      mmio_pkt->write_en = MMIO_READ_REQ;
      mmio_pkt->width = MMIO_WIDTH_32;
      mmio_pkt->addr = offset;
      mmio_pkt->resp_en = 0;
      
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Read      : tid = 0x%03x, offset = 0x%x\n", mmio_pkt->tid, mmio_pkt->addr);
      END_YELLOW_FONTCOLOR;

      mmio_request_put(mmio_pkt);

      // Wait until correct response found
      while (mmio_pkt->tid != mmio_rsp_pkt->tid)
	{
	  usleep(500);
	}

      memcpy(mmio_pkt, mmio_rsp_pkt, sizeof(mmio_t));
      mmio_rsp_pkt_accepted = 1;

      // Display
      BEGIN_YELLOW_FONTCOLOR;

      // Write data
      *data32 = (uint32_t)mmio_pkt->qword[0];

      printf("  [APP]  MMIO Read Resp : tid = 0x%03x, %08x\n", mmio_pkt->tid, (uint32_t)*data32);
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

      mmio_pkt->tid      = generate_mmio_tid();
      mmio_pkt->write_en = MMIO_READ_REQ;
      mmio_pkt->width    = MMIO_WIDTH_64;
      mmio_pkt->addr     = offset;
      mmio_pkt->resp_en  = 0;
      
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [APP]  MMIO Read      : tid = 0x%03x, offset = 0x%x\n", mmio_pkt->tid, mmio_pkt->addr);
      END_RED_FONTCOLOR;

      // Send request
      mmio_request_put(mmio_pkt);

      // Wait for correct response to be back
      while (mmio_pkt->tid != mmio_rsp_pkt->tid)
	{
	  usleep(5000);
	};

      memcpy(mmio_pkt, mmio_rsp_pkt, sizeof(mmio_t));
      mmio_rsp_pkt_accepted = 1;

      // Display
      BEGIN_YELLOW_FONTCOLOR;

      // Write data
      *data64 = mmio_pkt->qword[0];

      printf("  [APP]  MMIO Read Resp : tid = 0x%03x, data = %llx\n", mmio_pkt->tid, (unsigned long long)*data64);
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
  int ret;
  ret = ftruncate(mem->fd_app, (off_t)mem->memsize);
#ifdef ASE_DEBUG
  if (ret != 0)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [DEBUG]  ftruncate failed");
      perror("ftruncate");
      END_YELLOW_FONTCOLOR;
    }
#endif

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
  // free(mem);

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
  wsmeta_end->valid = 1;

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
  if ((bufptr != NULL) && (wsptr->valid == 1))
    {
      deallocate_buffer((struct buffer_t *)bufptr);
      wsptr->valid = 0;
    }
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
// void __attribute__((optimize("O0"))) ase_portctrl(const char *ctrl_msg)
void ase_portctrl(const char *ctrl_msg)
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

