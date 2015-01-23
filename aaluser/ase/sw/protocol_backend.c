// Copyright (c) 2014, Intel Corporation
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
 * Module Info: Generic protocol backend for keeping IPCs alive,
 * interfacing with DPI-C, messages and SW application
 *
 * Language   : C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 *
 */

#include "ase_common.h"

// ---------------------------------------------------------------
// Message queues descriptors
// ---------------------------------------------------------------
mqd_t app2sim_rx;           // app2sim mesaage queue in RX mode
mqd_t sim2app_tx;           // sim2app mesaage queue in TX mode
mqd_t app2sim_csr_wr_rx;    // CSR Write listener MQ in RX mode
mqd_t app2sim_umsg_rx;      // UMSG    message queue in RX mode
mqd_t app2sim_simkill_rx;   // app2sim message queue in RX mode
mqd_t sim2app_intr_tx;      // sim2app message queue in TX mode

// Global test complete counter
// Keeps tabs of how many session_deinits were received
int glbl_test_cmplt_cnt;

/*
 * Generate scope data
 */
svScope scope;
void scope_function()
{
  scope = svGetScope();
}


/*
 * DPI: UMSG Data exchange
 */
/* int glbl_umsg_meta; */
/* char glbl_umsg_data[CL_BYTE_WIDTH]; */
/* int glbl_umsg_serviced; */
/* void umsg_dex(cci_pkt *umsg) */
/* { */
/*   FUNC_CALL_ENTRY; */

/*   umsg->meta = glbl_umsg_meta; */
/*   memcpy(umsg->qword, glbl_umsg_data, CL_BYTE_WIDTH); */
/* #ifdef ASE_DEBUG */
/*   int i; */
/*   printf("UMSG_DEX =>\n"); */
/*   printf("%08x", (uint32_t)umsg->meta); */
/*   for (i = 1; i < 8; i++) */
/*     printf("%016llX ", umsg->qword[i]); */
/*   printf("\n"); */
/* #endif */
/*   umsg->cfgvalid = 0; */
/*   umsg->wrvalid  = 0; */
/*   umsg->rdvalid  = 0; */
/*   umsg->intrvalid = 0; */
/*   umsg->umsgvalid = 1; */

/*   // ase_umsg_cnt++; */

/*   FUNC_CALL_EXIT; */
/* } */


/*
 * DPI: WriteLine Data exchange
 */
void wr_memline_dex(cci_pkt *pkt, int *cl_addr, int *mdata, char *wr_data )
{
  FUNC_CALL_ENTRY;
  uint64_t* wr_target_vaddr = (uint64_t*)NULL;
  uint64_t fake_wr_addr = 0;
  int i;

  // Generate fake byte address
  fake_wr_addr = (uint64_t)(*cl_addr) << 6;
  // Decode virtual address
  wr_target_vaddr = ase_fakeaddr_to_vaddr((uint64_t)fake_wr_addr);

  // Mem-copy from TX1 packet to system memory
  memcpy(wr_target_vaddr, wr_data, CL_BYTE_WIDTH);

  //////////// Write this to RX-path //////////////
  // Zero out data buffer
  for(i = 0; i < 8; i++)
    pkt->qword[i] = 0x0;

  // Loop around metadata
  pkt->meta = (ASE_RX0_WR_RESP << 14) | (*mdata);

  // Valid signals
  pkt->cfgvalid = 0;
  pkt->wrvalid  = 1;
  pkt->rdvalid  = 0;
  pkt->intrvalid = 0;
  pkt->umsgvalid = 0;

  // ase_write_cnt++;

  FUNC_CALL_EXIT;
}


/*
 * DPI: ReadLine Data exchange
 */
void rd_memline_dex(cci_pkt *pkt, int *cl_addr, int *mdata )
{
  FUNC_CALL_ENTRY;

  uint64_t fake_rd_addr = 0;
  uint64_t* rd_target_vaddr = (uint64_t*) NULL;

  // Fake CL address to fake address conversion
  fake_rd_addr = (uint64_t)(*cl_addr) << 6;

  // Calculate Virtualized SHIM address (translation table)
  rd_target_vaddr = ase_fakeaddr_to_vaddr((uint64_t)fake_rd_addr);

  // Copy data to memory
  memcpy(pkt->qword, rd_target_vaddr, CL_BYTE_WIDTH);

  // Loop around metadata
  pkt->meta = (ASE_RX0_RD_RESP << 14) | (*mdata);

  // Valid signals
  pkt->cfgvalid = 0;
  pkt->wrvalid  = 0;
  pkt->rdvalid  = 1;
  pkt->intrvalid = 0;
  pkt->umsgvalid = 0;

  // ase_read_cnt++;

  FUNC_CALL_EXIT;
}


// -----------------------------------------------------------------------
// vbase/pbase exchange THREAD
// when an allocate request is received, the buffer is copied into a
// linked list. The reply consists of the pbase, fakeaddr and fd_ase.
// When a deallocate message is received, the buffer is invalidated.
// -----------------------------------------------------------------------
//int buffer_replicator()
int ase_listener()
{
   FUNC_CALL_ENTRY;

   /*
    * Buffer Replicator
    */
  // DPI buffer
  struct buffer_t ase_buffer;

  // Prepare an empty buffer
  ase_empty_buffer(&ase_buffer);
  // Receive a DPI message and get information from replicated buffer
  if (ase_recv_msg(&ase_buffer)==1)
    {
      // ALLOC request received
      if(ase_buffer.metadata == HDR_MEM_ALLOC_REQ)
	{
	  ase_alloc_action(&ase_buffer);
	  ase_buffer.is_privmem = 0;
	  if (ase_buffer.index == 0)
	    ase_buffer.is_csrmap = 1;
	  else
	    ase_buffer.is_csrmap = 0;
	}
      // if DEALLOC request is received
      else if(ase_buffer.metadata == HDR_MEM_DEALLOC_REQ)
	{
	  ase_dealloc_action(&ase_buffer);
	}

    #ifdef ASE_DEBUG
      ase_buffer_info(&ase_buffer);
    #else
      ase_buffer_oneline(&ase_buffer);
    #endif
    }


  /*
   * CSR Write listener
   */
  // Message string
  char csr_wr_str[ASE_MQ_MSGSIZE];
  char *pch;
  char ase_msg_data[CL_BYTE_WIDTH];
  uint32_t csr_offset;
  uint32_t csr_data;

  // Cleanse receptacle string
  memset(ase_msg_data, '\0', sizeof(ase_msg_data));

  // Receive csr_write packet
  if(mqueue_recv(app2sim_csr_wr_rx, (char*)csr_wr_str)==1)
    {
      // Tokenize message to get CSR offset and data
      pch = strtok(csr_wr_str, " ");
      csr_offset = atoi(pch);
      pch = strtok(NULL, " ");
      csr_data = atoi(pch);

      // CSRWrite Dispatch
      csr_write_dispatch ( 0, csr_offset, csr_data );

      // *FIXME*: Synchronizer must go here... TEST CODE
      ase_memory_barrier();
    }


  /*
   * UMSG listener
   */
#if 0
  // Message string
  char umsg_str[ASE_MQ_MSGSIZE];

  // Umsg parameters
  uint32_t umsg_id;
  uint32_t umsg_hint;
  char umsg_data[CL_BYTE_WIDTH];
  uint64_t *umas_target_addr;
  uint64_t umas_target_addrint;

  // Cleanse receptacle string
  memset (umsg_str, '\0', sizeof(umsg_str));

  // Keep checking message queue
  if (mqueue_recv(app2sim_umsg_rx, (char*)umsg_str ) == 1)
    {
      // Tokenize messgae to get msg_id & umsg_data
      sscanf (umsg_str, "%u %u %lu", &umsg_id, &umsg_hint, &umas_target_addrint );
      umas_target_addr = (uint64_t*)umas_target_addrint;
      memcpy(umsg_data, umas_target_addr, CL_BYTE_WIDTH);

      int ii;
      BEGIN_RED_FONTCOLOR;
      printf("Printing data...\n");
      printf("umsg_hint = %08x\n", umsg_hint);
      printf("umsg_id   = %08x\n", umsg_id);
      for (ii = 0; ii < CL_BYTE_WIDTH; ii++)
	printf("%02d ", umsg_data[ii]);
      printf("\nDONE\n");
      END_RED_FONTCOLOR;

      // UMSG Hint
      if (umsg_hint)
	{
	  glbl_umsg_serviced = 0;
	  umsg_init(1);
	  glbl_umsg_meta = (ASE_RX0_UMSG  << 14) | (umsg_hint << 12);
	  memset(glbl_umsg_data, '\0', CL_BYTE_WIDTH);
	  while(glbl_umsg_serviced != 1)
	    {
	      run_clocks(1);
	    }
	  umsg_listener_activecnt++;
	  umsg_init(0);
	}
      else
	{
	  // Send UMSG with data
	  glbl_umsg_serviced = 0;
	  umsg_init(1);
	  glbl_umsg_meta = (ASE_RX0_UMSG  << 14);
	  memcpy(glbl_umsg_data, umsg_data, CL_BYTE_WIDTH);
	  while(glbl_umsg_serviced != 1)
	    {
	      run_clocks(1);
	    }
	  umsg_listener_activecnt++;
	  umsg_init(0);
	}
    }
#endif

  /*
   * SIMKILL message handler
   */
  char ase_simkill_str[ASE_MQ_MSGSIZE];
  memset (ase_simkill_str, '\0', ASE_MQ_MSGSIZE);
  if(mqueue_recv(app2sim_simkill_rx, (char*)ase_simkill_str)==1)
    {
      // if (memcmp (ase_simkill_str, (char*)ASE_SIMKILL_MSG, ASE_MQ_MSGSIZE) == 0)
      // Update regression counter
      glbl_test_cmplt_cnt = glbl_test_cmplt_cnt + 1;

      // If in regression mode or SW-simkill mode
      if ( (cfg->ase_mode == 3) ||
	   ((cfg->ase_mode == 4) && (cfg->ase_num_tests == glbl_test_cmplt_cnt))
	   )
	{
	  printf("\n");
	  printf("SIM-C : ASE Session Deinitialization was detected... Simulator will EXIT\n");
	  run_clocks (100);
	  // sleep(5);
	  ase_perror_teardown();
	  start_simkill_countdown();
	}
    }


  FUNC_CALL_EXIT;
  return 0;
}


/*
 * Calculate Sysmem & CAPCM ranges to be used by ASE
 */
void calc_phys_memory_ranges()
{
  uint32_t cipuctl_22;
  uint32_t cipuctl_21_19;

  cipuctl_22    = (cfg->memmap_sad_setting & 0xF) >> 3;
  cipuctl_21_19 = (cfg->memmap_sad_setting & 0x7);

#ifdef ASE_DEBUG
  printf("        CIPUCTL[22] = %d | CIPUCTL[21:19] = %d\n", cipuctl_22, cipuctl_21_19 );
#endif

  // Memmory map calculation
  if (cfg->enable_capcm)
    {
      capcm_size = (uint64_t)( pow(2, cipuctl_21_19 + 1) * 1024 * 1024 * 1024);
      sysmem_size = (uint64_t)( (uint64_t)pow(2, FPGA_ADDR_WIDTH) - capcm_size);

      // Place CAPCM based on CIPUCTL[22]
      if (cipuctl_22 == 0)
	{
	  capcm_phys_lo = 0;
	  capcm_phys_hi = capcm_size - 1;
	  sysmem_phys_lo = capcm_size;
	  sysmem_phys_hi = (uint64_t)pow(2, FPGA_ADDR_WIDTH) - 1;
	}
      else
	{
	  capcm_phys_hi = (uint64_t)pow(2,FPGA_ADDR_WIDTH) - 1;
	  capcm_phys_lo = capcm_phys_hi + 1 - capcm_size;
	  sysmem_phys_lo = 0;
	  sysmem_phys_hi = sysmem_phys_lo + sysmem_size;
	}
    }
  else
    {
      sysmem_size = (uint64_t)pow(2, FPGA_ADDR_WIDTH);
      sysmem_phys_lo = 0;
      sysmem_phys_hi = sysmem_size-1;
      capcm_size = 0;
      capcm_phys_lo = 0;
      capcm_phys_hi = 0;
    }

  BEGIN_YELLOW_FONTCOLOR;
  printf("        System memory range  => 0x%016lx-0x%016lx | %ld~%ld GB \n",
	 sysmem_phys_lo, sysmem_phys_hi, sysmem_phys_lo/(uint64_t)pow(1024, 3), (uint64_t)(sysmem_phys_hi+1)/(uint64_t)pow(1024, 3) );
  if (cfg->enable_capcm)
    printf("        Private memory range => 0x%016lx-0x%016lx | %ld~%ld GB\n",
	   capcm_phys_lo, capcm_phys_hi, capcm_phys_lo/(uint64_t)pow(1024, 3), (uint64_t)(capcm_phys_hi+1)/(uint64_t)pow(1024, 3) );
  END_YELLOW_FONTCOLOR;

  // Internal check messages
  if (cfg->enable_capcm)
    {
      if (capcm_size > (uint64_t)8*1024*1024*1024 )
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        Caching agent private memory size > 8 GB, this can cause a virtual memory hog\n");
	  printf("        Consider using a smaller memory for simulation !! \n");
	  printf("        Simulation will continue with requested setting, change to a smaller CAPCM in ase.cfg !!\n");
	  END_RED_FONTCOLOR;
	}
      if (sysmem_size == 0)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        System SAD setting has set System Memory size to 0 bytes. Please check that this is intended !\n");
	  printf("        Any SW Workspace Allocate action will FAIL !!\n");
	  printf("        Simulation will continue with requested setting...\n");
	  END_RED_FONTCOLOR;
	}
      if (capcm_size == 0)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        CAPCM is enabled and has size set to 0 bytes. Please check that this is intended !!\n");
	  printf("        Simulation will continue, but NO CAPCM regions will be created");
	  END_RED_FONTCOLOR;
	}
    }
}


// -----------------------------------------------------------------------
// DPI Initialize routine
// - Setup message queues
// - Start buffer replicator, csr_write listener thread
// -----------------------------------------------------------------------
void ase_init()
{
  FUNC_CALL_ENTRY;

  // ASE configuration management
  ase_config_parse(ASE_CONFIG_FILE);

  // RRS: Wed Oct 16 17:35:23 PDT 2013
  // RRS: Environment variable instructions
  // Generate timstamp
  put_timestamp();

  // Print timestamp
  printf("SIM-C : Timestamp => %s\n", get_timestamp(0) );

  // Define a null string
  memset(null_str, 64, '\0');
  shim_called = 0;
  fake_off_low_bound = 0;

  // Create IPC cleanup setup
#ifdef SIM_SIDE
  create_ipc_listfile();
#endif

  // Set CSR write to '0', i.e. no CSR write has occured
  // csr_write_init (0);

  // Set up message queues
#ifdef SIM_SIDE
  printf("SIM-C : Set up ASE message queues...\n");
  ase_mqueue_setup();
#endif

  // Calculate memory map regions
  printf("SIM-C : Calculating memory map...\n");
  calc_phys_memory_ranges();

  // Random number for csr_pinned_addr
  if (cfg->enable_reuse_seed)
    {
      ase_addr_seed = ase_read_seed ();
     }
  else
    {
      ase_addr_seed = time(NULL);
      ase_write_seed ( ase_addr_seed );
    }
  srand ( ase_addr_seed );

  FUNC_CALL_EXIT;
}


// -----------------------------------------------------------------------
// ASE ready indicator:  Print a message that ASE is ready to go.
// Controls run-modes
// -----------------------------------------------------------------------
void ase_ready()
{
  char *tstamp_env;
  tstamp_env = malloc(80);

  // Set test_cnt to 0
  glbl_test_cmplt_cnt = 0;

  // Display "Ready for simulation"
  BEGIN_GREEN_FONTCOLOR;
  printf("SIM-C : ** ATTENTION : BEFORE running the software application **\n");
  tstamp_env = getenv("PWD");
  printf("        Run the following command into terminal where application will run (copy-and-paste) =>\n");
  printf("        If $SHELL is 'bash', run:\n");
  printf("                                export ASE_WORKDIR=%s\n", tstamp_env);
  printf("        If $SHELL is 'tcsh' or 'csh', run: \n");
  printf("                                setenv ASE_WORKDIR %s\n", tstamp_env);
  printf("        For any other $SHELL, consult your Linux administrator\n");
  printf("\n");
  printf("SIM-C : Ready for simulation...\n");
  END_GREEN_FONTCOLOR;

  // Register SIGINT and listen to it
  signal(SIGTERM, start_simkill_countdown);
  signal(SIGINT , start_simkill_countdown);
  signal(SIGQUIT, start_simkill_countdown);
  signal(SIGKILL, start_simkill_countdown); // *FIXME*: This possibly doesnt work //
  signal(SIGHUP,  start_simkill_countdown);

  // Get PID
  ase_pid = getpid();
  printf("SIM-C : PID of simulator is %d\n", ase_pid);

  // Indicate readiness with .ase_ready file
  ase_ready_fd = fopen(ASE_READY_FILENAME, "w");
  fprintf(ase_ready_fd, "%d", ase_pid);
  fclose(ase_ready_fd);

  printf("SIM-C : Press CTRL-C to close simulator...\n");
  
  // Run ase_regress.sh here
  if (cfg->ase_mode == 4) 
    {
      printf("Starting ase_regress.sh script...\n");
      system("./ase_regress.sh &");  
    }
}


/*
 * DPI simulation timeout counter
 * - When CTRL-C is pressed, start teardown sequence
 * - TEARDOWN SEQUENCE:
 *   - Close and unlink message queues
 *   - Close and unlink shared memories
 *   - Destroy linked list
 *   - Delete .ase_ready
 *   - Send $finish to VCS
 */
void start_simkill_countdown()
{
  FUNC_CALL_ENTRY;

  // Close and unlink message queue
  printf("SIM-C : Closing message queue and unlinking...\n");
  // ase_mqueue_teardown();

  // Destroy all open shared memory regions
  printf("SIM-C : Unlinking Shared memory regions.... \n");
  // ase_destroy();

  // *FIXME* Remove the ASE timestamp file
  if (unlink(TSTAMP_FILENAME) == -1)
    {
      printf("SIM-C : %s could not be deleted, please delete manually... \n", TSTAMP_FILENAME);
    }

  // Final clean of IPC
  final_ipc_cleanup();

  // Remove session files
  printf("SIM-C : Cleaning session files...\n");
  if ( unlink(ASE_READY_FILENAME) == -1 )
    {
      BEGIN_RED_FONTCOLOR;
      printf("Session file %s could not be removed, please remove manually !!\n", ASE_READY_FILENAME);
      END_RED_FONTCOLOR;
    }

  // Print location of transactions file
  BEGIN_GREEN_FONTCOLOR;
  printf("SIM-C : ASE Transactions file => $ASE_WORKDIR/transactions.tsv\n");
  END_GREEN_FONTCOLOR;

  // Send a simulation kill command
  printf("SIM-C : Sending kill command...\n");
  svSetScope(scope);
  simkill();

  FUNC_CALL_EXIT;
}


/*
 * ase_umsg_init : SIM_SIDE UMSG setup
 *                 Set up CSR addresses to indicate existance
 *                 and features of the UMSG system
 */
void ase_umsg_init(uint64_t dsm_base)
{
  FUNC_CALL_ENTRY;

#if 0
  uint32_t *cirbstat;

  printf ("SIM-C : Enabling UMSG subsystem in ASE...\n");

  // Calculate CIRBSTAT address
  cirbstat = (uint32_t*)((uint64_t)(dsm_base + ASE_CIRBSTAT_CSROFF));

  // CIRBSTAT setup (completed / ready)
  *cirbstat = cfg->num_umsg_log2 << 4 | 0x1 << 0;
  printf ("        DSM base      = %p\n", (uint32_t*)dsm_base);
  printf ("        CIRBSTAT addr = %p\n", cirbstat);
  printf ("        *cirbstat     = %08x\n", *cirbstat);
#endif

  FUNC_CALL_EXIT;
}


/*
 * Parse strings and remove unnecessary characters
 */
// Remove spaces
void remove_spaces(char* in_str)
{
  char* i;
  char* j;
  i = in_str;
  j = in_str;
  while(*j != 0)
    {
      *i = *j++;
      if(*i != ' ')
	i++;
    }
  *i = 0;
}


// Remove tabs
void remove_tabs(char* in_str)
{
  char *i = in_str;
  char *j = in_str;
  while(*j != 0)
    {
      *i = *j++;
      if(*i != '\t')
  	i++;
    }
  *i = 0;
}

// Remove newline
void remove_newline(char* in_str)
{
  char *i = in_str;
  char *j = in_str;
  while(*j != 0)
    {
      *i = *j++;
      if(*i != '\n')
  	i++;
    }
  *i = 0;
}


/*
 * ASE config parsing
 * - Set default values for ASE configuration
 * - See if a ase.cfg is available for overriding global values
 *   - If YES, parse and configure the cfg (ase_cfg_t) structure
 */
void ase_config_parse(char *filename)
{
  FUNC_CALL_ENTRY;

  FILE *fp;
  char *line;
  size_t len = 0;
  ssize_t read;
  char *parameter;
  int value;
  // int tmp_umsg_log2;

  char *ase_cfg_filepath;
  ase_cfg_filepath = malloc(256);
  ase_cfg_filepath = generate_tstamp_path(filename);

  // Allocate space to store ASE config
  cfg = (struct ase_cfg_t *)malloc( sizeof(struct ase_cfg_t) );
  if (cfg == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : ASE config structure could not be allocated... EXITING\n");
      END_RED_FONTCOLOR;
      ase_error_report("malloc", errno, ASE_OS_MALLOC_ERR);
    #ifdef SIM_SIDE
      start_simkill_countdown();
    #else
      exit(1);
    #endif
    }
  line = malloc(sizeof(char) * 80);

  // Default values
  cfg->ase_mode = 1;
  cfg->ase_timeout = 500;
  cfg->ase_num_tests = 1;
  cfg->enable_reuse_seed = 0;
  cfg->enable_capcm = 0;
  cfg->memmap_sad_setting = 0;
  cfg->num_umsg_log2 = 5;
  cfg->enable_cl_view = 1;

  // Find ase.cfg OR not
  // if ( access (ASE_CONFIG_FILE, F_OK) != -1 )
  if ( access (ase_cfg_filepath, F_OK) != -1 )
    {
      // FILE exists, overwrite
      printf("SIM-C : Reading %s configuration file\n", ASE_CONFIG_FILE);
      fp = fopen(ase_cfg_filepath, "r");

      // Parse file line by line
      while ((read = getline(&line, &len, fp)) != -1)
	{
	  // Remove all invalid characters
	  remove_spaces (line);
	  remove_tabs (line);
	  remove_newline (line);
	  // Ignore strings begining with '#' OR NULL (compound NOR)
	  if ( (line[0] != '#') && (line[0] != '\0') )
	    {
	      parameter = strtok(line, "=\n");
	      value = atoi(strtok(NULL, ""));
	      if (strcmp (parameter,"ASE_MODE") == 0)
	      	cfg->ase_mode = value;
	      else if (strcmp (parameter,"ASE_TIMEOUT") == 0)
	      	cfg->ase_timeout = value;
	      else if (strcmp (parameter,"ASE_NUM_TESTS") == 0)
	      	cfg->ase_num_tests = value;
	      else if (strcmp (parameter, "ENABLE_REUSE_SEED") == 0)
		cfg->enable_reuse_seed = value;
	      else if (strcmp (parameter,"ENABLE_CAPCM") == 0)
	      	cfg->enable_capcm = value;
	      else if (strcmp (parameter,"MEMMAP_SAD_SETTING") == 0)
	      	cfg->memmap_sad_setting = value;
	      else if (strcmp (parameter,"NUM_UMSG_LOG2") == 0)
		cfg->num_umsg_log2 = value;
	      else if (strcmp (parameter,"ENABLE_CL_VIEW") == 0)
		cfg->enable_cl_view = value;
	      else
	      	printf("SIM-C : In config file %s, Parameter type %s is unidentified \n", ASE_CONFIG_FILE, parameter);
	    }
	}

      // ASE mode control
      switch (cfg->ase_mode)
	{
	  // Classic Server client mode
	case 1:
	  printf("SIM-C : ASE was started in Mode 1 (Server-Client without SIMKILL)\n");
	  cfg->ase_timeout = 0;
	  cfg->enable_sw_simkill = 0;
	  cfg->ase_num_tests = 0;
	  break;

	  // Server Client mode with SIMKILL
	case 2:
	  printf("SIM-C : ASE was started in Mode 2 (Server-Client with SIMKILL)\n");
	  cfg->enable_sw_simkill = 0;
	  cfg->ase_num_tests = 0;
	  break;

	  // Long runtime mode (SW kills SIM)
	case 3:
	  printf("SIM-C : ASE was started in Mode 3 (Server-Client with Sw SIMKILL (long runs)\n");
	  cfg->ase_timeout = 0;
	  cfg->enable_sw_simkill = 1;
	  cfg->ase_num_tests = 0;
	  break;

	  // Regression mode (lets an SH file with
	case 4:
	  printf("SIM-C : ASE was started in Mode 4 (Regression mode)\n");
	  cfg->ase_timeout = 0;
	  cfg->enable_sw_simkill = 0;
	  break;

	  // Illegal modes
	default:
	  printf("SIM-C : ASE mode could not be identified, will revert to ASE_MODE = 1 (Server client w/o SIMKILL)\n");
	  cfg->ase_mode = 1;
	  cfg->ase_timeout = 0;
	  cfg->enable_sw_simkill = 0;
	  cfg->ase_num_tests = 0;
	}


      // CAPCM size implementation
      if (cfg->enable_capcm != 0)
	{
	  if ((cfg->memmap_sad_setting > 15) || (cfg->memmap_sad_setting < 0))
	    {
	      BEGIN_YELLOW_FONTCOLOR;
	      printf("SIM-C : In config file %s, there was an error in setting MEMMAP_SAD_SETTING\n", ASE_CONFIG_FILE);
	      printf("        MEMMAP_SAD_SETTING was %d\n", cfg->memmap_sad_setting);
	      printf("        Setting default MEMMAP_SAD_SETTING to default '2', see ase.cfg and ASE User Guide \n");
	      cfg->memmap_sad_setting = 2;
	      END_YELLOW_FONTCOLOR;
	    }
	}

      // UMSG implementation
      if (cfg->num_umsg_log2 == 0)
	{
	  BEGIN_YELLOW_FONTCOLOR;
	  printf("SIM-C : In config file %s, there was an error in setting NUM_UMSG_LOG2\n", ASE_CONFIG_FILE);
	  printf("        NUM_UMSG_LOG2 was %d\n", cfg->num_umsg_log2);
	  printf("        Setting default NUM_UMSG_LOG2 to default 5\n");
	  cfg->num_umsg_log2 = 5;
	  END_YELLOW_FONTCOLOR;
	}

      // Close file
      fclose(fp);
    }
  else
    {
      // FILE does not exist
      printf("SIM-C : %s not found, using default values\n", ASE_CONFIG_FILE);
    }

  // Print configurations
  BEGIN_YELLOW_FONTCOLOR;

  // ASE mode
  printf("        ASE mode                   ... ");
  switch (cfg->ase_mode)
    {
    case 1: printf("Server-Client mode without SIMKILL\n") ; break ;
    case 2: printf("Server-Client mode with SIMKILL\n") ; break ;
    case 3: printf("Server-Client mode with SW SIMKILL (long runs)\n") ; break ;
    case 4: printf("ASE Regression mode\n") ; break ;
    }  

  // Inactivity
  if (cfg->ase_timeout != 0)
    printf("        Inactivity kill-switch     ... ENABLED after %d clocks \n", cfg->ase_timeout);
  else
    printf("        Inactivity kill-switch     ... DISABLED \n");
  
  // Reuse seed
  if (cfg->enable_reuse_seed != 0)
    printf("        Reuse simulation seed      ... ENABLED \n");
  else
    printf("        Reuse simulation seed      ... DISABLED \n");

  // UMSG
  printf("        Number of UMSG buffers     ... %d (NUM_UMSG_LOG2 = %d) \n", (int)pow((float)2, (float)cfg->num_umsg_log2), cfg->num_umsg_log2);

  // CAPCM
  if (cfg->enable_capcm != 0)
    {
      printf("        CA Private memory          ... ENABLED\n");
    }
  else
    printf("        CA Private memory          ... DISABLED\n");

  // CL view
  if (cfg->enable_cl_view != 0)
    printf("        ASE Transaction view       ... ENABLED\n");
  else
    printf("        ASE Transaction view       ... DISABLED\n");

  END_YELLOW_FONTCOLOR;

  // Transfer data to hardware (for simulation only)
#ifdef SIM_SIDE
  ase_config_dex(cfg);
#endif

  FUNC_CALL_EXIT;
}
