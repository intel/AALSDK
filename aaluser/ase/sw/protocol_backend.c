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


// Global test complete counter
// Keeps tabs of how many session_deinits were received
int glbl_test_cmplt_cnt;

// Global umsg mode, lookup before issuing UMSG
int glbl_umsgmode;


/*
 * Generate scope data
 */
svScope scope;
void scope_function()
{
  scope = svGetScope();
}

/*
 * ASE instance already running 
 * - If instance is found, return its process ID, else return 0 
 */
int ase_instance_running()
{
  FUNC_CALL_ENTRY;

  FILE *fp_ready_check = (FILE *)NULL;
  int ase_simv_pid;

  // If Ready file does not exist
  if ( access(ASE_READY_FILENAME, F_OK) == -1)
    {
      ase_simv_pid = 0;
    }
  // If ready file exists
  else
    {
      // Read contents of file and send to 
      fp_ready_check = fopen(ASE_READY_FILENAME, "r");
      // If success
      if (fp_ready_check != NULL)
	{
	  fscanf(fp_ready_check, "%d", &ase_simv_pid);
	  fclose(fp_ready_check);
	}
      // If failed
      else
	{
	  ase_simv_pid = -1;
	}
    }

  /* if (fp_ready_check != NULL) */
  /*   { */
  /*     fclose(fp_ready_check); */
  /*   } */

  return ase_simv_pid;
  FUNC_CALL_EXIT;
}


/*
 * DPI: CONFIG path data exchange
 */
void sv2c_config_dex(const char *str)
{
  sv2c_config_filepath = ase_malloc(ASE_FILEPATH_LEN);
  // strcpy(sv2c_config_filepath, str);
  strncpy(sv2c_config_filepath, str, ASE_FILEPATH_LEN);
#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [DEBUG]  sv2c_config_filepath = %s\n", sv2c_config_filepath);
  END_YELLOW_FONTCOLOR;
#endif

  if ( (sv2c_config_filepath != NULL) && (access(sv2c_config_filepath, F_OK)==0) )
    {
      printf("SIM-C : +CONFIG %s file was found\n", sv2c_config_filepath);
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : ** WARNING ** +CONFIG file was not found, will revert to DEFAULTS\n");
      memset(sv2c_config_filepath, 0, ASE_FILEPATH_LEN);
      END_YELLOW_FONTCOLOR;
    }
}


/*
 * DPI: SCRIPT path data exchange
 */
void sv2c_script_dex(const char *str)
{
  sv2c_script_filepath = ase_malloc(ASE_FILEPATH_LEN);
  // strcpy(sv2c_script_filepath, str);
  strncpy(sv2c_script_filepath, str, ASE_FILEPATH_LEN);
#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [DEBUG]  sv2c_script_filepath = %s\n", sv2c_script_filepath);
  END_YELLOW_FONTCOLOR;
#endif

  // Check for existance of file
  if ((sv2c_config_filepath != NULL) && (access(sv2c_script_filepath, F_OK)==0))
    {
      printf("SIM-C : +SCRIPT %s file was found\n", sv2c_script_filepath);
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : ** WARNING ** +SCRIPT file was not found, will revert to DEFAULTS\n");
      memset(sv2c_script_filepath, 0, ASE_FILEPATH_LEN);
      END_YELLOW_FONTCOLOR;
    }
}


/*
 * DPI: WriteLine Data exchange
 */
void wr_memline_dex(cci_pkt *pkt)
{
  FUNC_CALL_ENTRY;

  uint64_t phys_addr;
  uint64_t *wr_target_vaddr = (uint64_t*)NULL;
  //int ret_fd;
#ifndef DEFEATURE_ATOMICS
  uint64_t *rd_target_vaddr = (uint64_t*)NULL;
  long long cmp_qword;  // Data to be compared
  long long new_qword;  // Data to be writen if compare passes
#endif

  if (pkt->mode == CCIPKT_WRITE_MODE) 
    {
      /* 
       * Normal write operation
       * Takes Write request and performs verbatim
       */
      // Get cl_addr, deduce wr_target_vaddr
      phys_addr = (uint64_t)pkt->cl_addr << 6;
      wr_target_vaddr = ase_fakeaddr_to_vaddr((uint64_t)phys_addr); 
      
      // Write to memory
      memcpy(wr_target_vaddr, (char*)pkt->qword, CL_BYTE_WIDTH);
      
      // Success
      pkt->success = 1;
    }
#ifndef DEFEATURE_ATOMICS
  else if (pkt->mode == CCIPKT_ATOMIC_MODE)
    {
      /*
       * This is a special mode in which read response goes back 
       * WRITE request is responded with a READ response
       */
      // Specifics of the requested compare operation
      cmp_qword = pkt->qword[0];
      new_qword = pkt->qword[4];
      
      // Get cl_addr, deduce rd_target_vaddr
      phys_addr = (uint64_t)pkt->cl_addr << 6;
      rd_target_vaddr = ase_fakeaddr_to_vaddr((uint64_t)phys_addr); 
      
      // Perform read first and set response packet accordingly
      memcpy((char*)pkt->qword, rd_target_vaddr, CL_BYTE_WIDTH);      

      // Get cl_addr, deduct wr_target, use qw_start to determine exact qword
      wr_target_vaddr = (uint64_t*)( (uint64_t)rd_target_vaddr + pkt->qw_start*8 );
      
      // CmpXchg output
      pkt->success = (int)__sync_bool_compare_and_swap (wr_target_vaddr, cmp_qword, new_qword);
      
      // Debug output
#ifdef ASE_DEBUG
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [DEBUG]  CmpXchg_op=%d\n", pkt->success);
      END_YELLOW_FONTCOLOR;
#endif
    }
#endif  

  FUNC_CALL_EXIT;
}
      

/*
 * DPI: ReadLine Data exchange
 */
void rd_memline_dex(cci_pkt *pkt )
{
  FUNC_CALL_ENTRY;

  uint64_t phys_addr;
  uint64_t *rd_target_vaddr = (uint64_t*)NULL;
  // int ret_fd;

  // Get cl_addr, deduce rd_target_vaddr
  phys_addr = (uint64_t)pkt->cl_addr << 6;
  rd_target_vaddr = ase_fakeaddr_to_vaddr((uint64_t)phys_addr);

  // Read from memory
  memcpy((char*)pkt->qword, rd_target_vaddr, CL_BYTE_WIDTH);

  FUNC_CALL_EXIT;
}


/*
 * DPI: MMIO response
 */
void mmio_response (struct mmio_t *mmio_pkt)
{
  FUNC_CALL_ENTRY;

  mqueue_send(sim2app_mmiorsp_tx, (char*)mmio_pkt, sizeof(mmio_t)); // ASE_MQ_MSGSIZE);

  FUNC_CALL_EXIT;
}

/*
 * Count error flag ping/pong
 */
int count_error_flag = 0;
void count_error_flag_pong(int flag)
{
  count_error_flag = flag;
}


/*
 * Update global disable/enable 
 */
int glbl_dealloc_allowed;
void update_glbl_dealloc(int flag)
{
  glbl_dealloc_allowed = flag;
}


/* ********************************************************************
 * ASE Listener thread
 * --------------------------------------------------------------------
 * vbase/pbase exchange THREAD
 * when an allocate request is received, the buffer is copied into a
 * linked list. The reply consists of the pbase, fakeaddr and fd_ase.
 * When a deallocate message is received, the buffer is invalidated.
 *
 * MMIO Request
 * Calls MMIO Dispatch task in ccip_emulator
 *
 * *******************************************************************/
int ase_listener()
{
  //   FUNC_CALL_ENTRY;

  // ---------------------------------------------------------------------- //
  /*
   * Port Control message
   * Format: <cmd> <value>
   * -----------------------------------------------------------------
   * Supported commands       |
   * ASE_INIT   <APP_PID>     | Session control - sends PID to 
   *                          |
   * AFU_RESET  <0,1>         | AFU reset handle
   * UMSG_MODE  <8-bit mask>  | UMSG mode control
   *
   * ASE responds with "COMPLETED" as a string, there is no
   * expectation of a string check
   * 
   */
  char portctrl_msgstr[ASE_MQ_MSGSIZE];
  char portctrl_cmd[ASE_MQ_MSGSIZE];
  int portctrl_value;
  memset(portctrl_msgstr, 0, ASE_MQ_MSGSIZE);
  memset(portctrl_cmd, 0, ASE_MQ_MSGSIZE);

  // Simulator is not in lockdown mode (simkill not in progress)
  if (self_destruct_in_progress == 0)
    {
      if (mqueue_recv(app2sim_portctrl_req_rx, (char*)portctrl_msgstr, ASE_MQ_MSGSIZE) == ASE_MSG_PRESENT)
	{
	  sscanf(portctrl_msgstr, "%s %d", portctrl_cmd, &portctrl_value);
	  // while(1);
	  if ( memcmp(portctrl_cmd, "AFU_RESET", 9) == 0)
	    {
	      // AFU Reset control
	      portctrl_value = (portctrl_value != 0) ? 1 : 0 ;
	      afu_softreset_trig ( portctrl_value );
	      printf("SIM-C : Soft Reset set to %d\n", portctrl_value);
	     
	      // Send portctrl_rsp message
	      mqueue_send(sim2app_portctrl_rsp_tx, "COMPLETED", ASE_MQ_MSGSIZE);
	    }
	  else if ( memcmp(portctrl_cmd, "UMSG_MODE", 9) == 0)
	    {
	      // Umsg mode setting here
	      glbl_umsgmode = portctrl_value & 0xFF;
	      printf("SIM-C : UMSG Mode mask set to 0x%x\n", glbl_umsgmode);

	      // Send portctrl_rsp message
	      mqueue_send(sim2app_portctrl_rsp_tx, "COMPLETED", ASE_MQ_MSGSIZE);
	    }
	  else if ( memcmp(portctrl_cmd, "ASE_INIT", 8) == 0)
	    {
	      printf("Session requested by PID = %d\n", portctrl_value);
	      // Generate new timestamp
	      put_timestamp();
	      tstamp_filepath = ase_malloc(ASE_FILEPATH_LEN);
	      sprintf(tstamp_filepath, "%s/%s", ase_workdir_path, TSTAMP_FILENAME);
	      // Print timestamp
	      printf("SIM-C : Session ID => %s\n", get_timestamp(0) );	  

	      // Send portctrl_rsp message
	      mqueue_send(sim2app_portctrl_rsp_tx, "COMPLETED", ASE_MQ_MSGSIZE);
	    }
	  else
	    {
	      BEGIN_RED_FONTCOLOR;
	      printf("SIM-C : Undefined Port Control function ... IGNORING\n");
	      END_RED_FONTCOLOR;

	      // Send portctrl_rsp message
	      mqueue_send(sim2app_portctrl_rsp_tx, "COMPLETED", ASE_MQ_MSGSIZE);
	    }
	}

      // ------------------------------------------------------------------------------- //

      /*
       * Buffer Allocation Replicator
       */
      struct buffer_t ase_buffer;
      char logger_str[ASE_LOGGER_LEN];

      // Receive a DPI message and get information from replicated buffer
      // if (ase_recv_msg(&ase_buffer)==ASE_MSG_PRESENT)
      ase_empty_buffer(&ase_buffer);
      if (mqueue_recv(app2sim_alloc_rx, (char*)&ase_buffer, ASE_MQ_MSGSIZE)==ASE_MSG_PRESENT)
	{
	  // ALLOC request received
	  if(ase_buffer.metadata == HDR_MEM_ALLOC_REQ)
	    {
	      // Allocate action
	      ase_alloc_action(&ase_buffer);
	      ase_buffer.is_privmem = 0;
	      if (ase_buffer.index == 0)
		ase_buffer.is_mmiomap = 1;
	      else
		ase_buffer.is_mmiomap = 0;

	      // Format workspace info string
	      memset (logger_str, 0, ASE_LOGGER_LEN);
	      if (ase_buffer.is_mmiomap) 
		{
		  sprintf(logger_str + strlen(logger_str), "MMIO map Allocated ");
		}
	      else if (ase_buffer.is_umas)
		{
		  sprintf(logger_str + strlen(logger_str), "UMAS Allocated ");
		}
	      else
		{
		  sprintf(logger_str + strlen(logger_str), "Buffer %d Allocated ", ase_buffer.index);
		}
	      sprintf(logger_str + strlen(logger_str), " (located /dev/shm/%s) =>\n", ase_buffer.memname);
	      sprintf(logger_str + strlen(logger_str), "\t\tHost App Virtual Addr  = %p\n", (void*)ase_buffer.vbase);
	      sprintf(logger_str + strlen(logger_str), "\t\tHW Physical Addr       = %p\n", (void*)ase_buffer.fake_paddr);
	      sprintf(logger_str + strlen(logger_str), "\t\tHW CacheAligned Addr   = %p\n", (void*)(ase_buffer.fake_paddr >> 6));
	      sprintf(logger_str + strlen(logger_str), "\t\tWorkspace Size (bytes) = %d\n", ase_buffer.memsize);
	      sprintf(logger_str + strlen(logger_str), "\n");

	      // Inject buffer message
	      buffer_msg_inject ( logger_str );
	    }

	  // Standard oneline message ---> Hides internal info
	  ase_buffer_oneline(&ase_buffer);

	  // Write buffer information to file
	  if ( (ase_buffer.is_mmiomap == 0) || (ase_buffer.is_privmem == 0) )
	    {
	      // Flush info to file
	      if (fp_workspace_log != NULL)
		{
		  fprintf(fp_workspace_log, "%s", logger_str);
		  fflush(fp_workspace_log);
		}
	    }

	  // Debug only
        #ifdef ASE_DEBUG
	  BEGIN_YELLOW_FONTCOLOR;
	  ase_buffer_info(&ase_buffer);
	  END_YELLOW_FONTCOLOR;
        #endif
	}
      // ------------------------------------------------------------------------------- //

      ase_empty_buffer(&ase_buffer);
      if (glbl_dealloc_allowed) 
	{
	  if (mqueue_recv(app2sim_dealloc_rx, (char*)&ase_buffer, ASE_MQ_MSGSIZE)==ASE_MSG_PRESENT)
	    {
	      // Format workspace info string
	      memset (logger_str, 0, ASE_LOGGER_LEN);
	      sprintf(logger_str + strlen(logger_str), "\nBuffer %d Deallocated =>\n", ase_buffer.index);
	      sprintf(logger_str + strlen(logger_str), "\n");
	  
	      // Deallocate action
	      ase_dealloc_action(&ase_buffer);
	  
	      // Inject buffer message
	      buffer_msg_inject ( logger_str );
	  
	      // Standard oneline message ---> Hides internal info
	      ase_buffer.valid = ASE_BUFFER_INVALID;
	      ase_buffer_oneline(&ase_buffer);

	      // Debug only
            #ifdef ASE_DEBUG
	      BEGIN_YELLOW_FONTCOLOR;
	      ase_buffer_info(&ase_buffer);
	      END_YELLOW_FONTCOLOR;
            #endif
	    }
	}
  
      // ------------------------------------------------------------------------------- //
      /*
       * MMIO request listener
       */
      // Message string
      char mmio_str[ASE_MQ_MSGSIZE];
      struct mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(mmio_t) );

      // Cleanse receptacle string
      memset(mmio_str, 0, ASE_MQ_MSGSIZE);
  
      // Receive csr_write packet
      if(mqueue_recv(app2sim_mmioreq_rx, (char*)mmio_pkt, sizeof(mmio_t) )==ASE_MSG_PRESENT)
	{
	  mmio_dispatch (0, mmio_pkt);

	  // *FIXME*: Synchronizer must go here... TEST CODE
	  ase_memory_barrier();
	}
      // ------------------------------------------------------------------------------- //


      /*
       * UMSG engine
       * *FIXME*: Profiling and costliness analysis needed here
       * *FIXME*: Notification service needs to be built
       */
      char umsg_mapstr[ASE_MQ_MSGSIZE];
      struct umsgcmd_t *umsg_pkt;
      umsg_pkt = (struct umsgcmd_t *)ase_malloc(sizeof(struct umsgcmd_t) );

      // cleanse string before reading
      memset(umsg_mapstr, 0, ASE_MQ_MSGSIZE);
      if ( mqueue_recv(app2sim_umsg_rx, (char*)umsg_mapstr, sizeof(struct umsgcmd_t) ) == ASE_MSG_PRESENT)
	{
	  memcpy(umsg_pkt, (umsgcmd_t *)umsg_mapstr, sizeof(struct umsgcmd_t));

	  // Hint trigger
	  umsg_pkt->hint = (glbl_umsgmode >> umsg_pkt->id) & 0x1;

        /* #ifdef ASE_DEBUG */
	/*   BEGIN_YELLOW_FONTCOLOR; */
	/*   printf("  [DEBUG] umsg_pkt %d %d %llx\n", umsg_pkt->id, umsg_pkt->hint, umsg_pkt->qword[0]); */
	/*   END_YELLOW_FONTCOLOR; */
        /* #endif */

	  // dispatch to event processing
	  umsg_dispatch(0, umsg_pkt);
	}
      // ------------------------------------------------------------------------------- //


      /*
       * SIMKILL message handler
       */
      char ase_simkill_str[ASE_MQ_MSGSIZE];
      memset (ase_simkill_str, 0, ASE_MQ_MSGSIZE);
      if(mqueue_recv(app2sim_simkill_rx, (char*)ase_simkill_str, ASE_MQ_MSGSIZE)==ASE_MSG_PRESENT)
	{
	  // Update regression counter
	  glbl_test_cmplt_cnt = glbl_test_cmplt_cnt + 1;

	  // Mode specific exit behaviour
	  if (cfg->ase_mode == ASE_MODE_DAEMON_NO_SIMKILL)
	    {
	      printf("SIM-C : ASE running in daemon mode (see ase.cfg)\n");
	      printf("        Reseting buffers ... Simulator RUNNING\n");
	      ase_destroy();
	      BEGIN_GREEN_FONTCOLOR;
	      printf("SIM-C : Ready to run next test\n");	  
	      END_GREEN_FONTCOLOR;
	    }
	  else if (cfg->ase_mode == ASE_MODE_DAEMON_SIMKILL)
	    {
	      printf("SIM-C : ASE Timeout SIMKILL will happen soon\n");
	    }
	  else if (cfg->ase_mode == ASE_MODE_DAEMON_SW_SIMKILL)
	    {
	      printf("SIM-C : ASE recognized a SW simkill (see ase.cfg)... Simulator will EXIT\n");
	      run_clocks (500);
	      ase_perror_teardown();
	      start_simkill_countdown();
	    }
	  else if ((cfg->ase_mode == ASE_MODE_REGRESSION) && (cfg->ase_num_tests == glbl_test_cmplt_cnt))
	    {
	      printf("SIM-C : ASE completed %d tests (see ase.cfg)... Simulator will EXIT\n", cfg->ase_num_tests);
	      run_clocks (500);
	      ase_perror_teardown();
	      start_simkill_countdown();
	    }

	  // Check for simulator sanity -- if transaction counts dont match
	  // Kill the simulation ASAP -- DEBUG feature only
       #ifdef ASE_DEBUG
	  if (count_error_flag_ping() == 1)
	    {
	      BEGIN_RED_FONTCOLOR;
	      printf("SIM-C : ** ERROR ** Transaction counts do not match, something got lost\n");
	      END_RED_FONTCOLOR;
	      run_clocks (500);
	      ase_perror_teardown();
	      start_simkill_countdown();	  
	    }				
        #endif
	}
      // ------------------------------------------------------------------------------- //
    }
  else 
    {
    #ifdef ASE_DEBUG
      BEGIN_RED_FONTCOLOR;
      printf("  [DEBUG]  Simulator is in Lockdown mode, Simkill in progress\n");
      sleep(1);
      END_RED_FONTCOLOR;
    #endif
    }

  //  FUNC_CALL_EXIT;
  return 0;
}


/*
 * Calculate Sysmem & CAPCM ranges to be used by ASE
 */
void calc_phys_memory_ranges()
{
  sysmem_size = cfg->phys_memory_available_gb * pow(1024, 3);
  sysmem_phys_lo = 0;
  sysmem_phys_hi = sysmem_size-1;

  // Calculate address mask
  PHYS_ADDR_PREFIX_MASK = ((sysmem_phys_hi >> MEMBUF_2MB_ALIGN) << MEMBUF_2MB_ALIGN);
#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [DEBUG]  PHYS_ADDR_PREFIX_MASK = %llx\n", (long long)PHYS_ADDR_PREFIX_MASK);
  END_YELLOW_FONTCOLOR;
#endif

  BEGIN_YELLOW_FONTCOLOR;
  printf("        System memory range  => 0x%016lx-0x%016lx | %ld~%ld GB \n",
	 sysmem_phys_lo, sysmem_phys_hi, 
	 sysmem_phys_lo/(uint64_t)pow(1024, 3), 
	 (uint64_t)(sysmem_phys_hi+1)/(uint64_t)pow(1024, 3) );
  END_YELLOW_FONTCOLOR;
}


// -----------------------------------------------------------------------
// DPI Initialize routine
// - Setup message queues
// - Start buffer replicator, csr_write listener thread
// -----------------------------------------------------------------------
int ase_init()
{
  FUNC_CALL_ENTRY;

  // Set stdout bufsize to empty immediately
  setvbuf(stdout, NULL, _IONBF, 0);

  // Set self_destruct flag = 0, SIMulator is not in lockdown
  self_destruct_in_progress = 0;

  // Register SIGINT and listen to it
  signal(SIGTERM, start_simkill_countdown);
  signal(SIGINT , start_simkill_countdown);
  signal(SIGQUIT, start_simkill_countdown);
  signal(SIGKILL, start_simkill_countdown); // *FIXME*: This possibly doesnt work //
  signal(SIGHUP,  start_simkill_countdown);

  // Ignore SIGPIPE *FIXME*: Look for more elegant solution
  signal(SIGPIPE, SIG_IGN);

  // Get PID
  ase_pid = getpid();
  printf("SIM-C : PID of simulator is %d\n", ase_pid);

  // Evaluate PWD
  ase_run_path = ase_malloc(ASE_FILEPATH_LEN);
  ase_run_path = getenv("PWD");
#ifdef ASE_DEBUG
  if (ase_run_path == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : getenv(PWD) evaluated NULL -- this is unexpected !\n");
      printf("        Needs Debug here\n");
      start_simkill_countdown();
      END_RED_FONTCOLOR;
    }
#endif

  // ASE configuration management
  ase_config_parse(ASE_CONFIG_FILE);

  // Evaluate IPCs
  ipc_init();

  // Evaluate Session directory
  // ase_workdir_path = ase_malloc(ASE_FILEPATH_LEN);
  /* ase_workdir_path = ase_eval_session_directory();   */
  // ase_eval_session_directory();
  // sprintf(ase_workdir_path, "%s/", ase_run_path);
  printf("SIM-C : ASE Session Directory located at =>\n");
  printf("        %s\n", ase_workdir_path);
  printf("SIM-C : Current Working Directory =>\n");
  printf("        %s\n", ase_run_path);

  // Create IPC cleanup setup
  create_ipc_listfile();

#ifdef ASE_DEBUG
  // Create a memory access log
  fp_memaccess_log = fopen("aseafu_access.log", "w");
  if (fp_memaccess_log == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : Memory access debug logger initialization failed !\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : Memory access debug logger initialized\n");
      END_YELLOW_FONTCOLOR;
    }

  // Page table tracker
  fp_pagetable_log = fopen("ase_pagetable.log", "w");
  if (fp_pagetable_log == NULL) 
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : ASE pagetable logger initialization failed !\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : ASE pagetable logger initialized\n");
      END_YELLOW_FONTCOLOR;
    }
  
#endif

  // Set up message queues
  printf("SIM-C : Creating Messaging IPCs...\n");
  int ipc_iter;
  for( ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_create( mq_array[ipc_iter].name );

  // Open message queues
  app2sim_alloc_rx    = mqueue_open(mq_array[0].name,  mq_array[0].perm_flag);
  app2sim_mmioreq_rx  = mqueue_open(mq_array[1].name,  mq_array[1].perm_flag);
  app2sim_umsg_rx     = mqueue_open(mq_array[2].name,  mq_array[2].perm_flag);
  app2sim_simkill_rx  = mqueue_open(mq_array[3].name,  mq_array[3].perm_flag);
  sim2app_alloc_tx    = mqueue_open(mq_array[4].name,  mq_array[4].perm_flag);
  sim2app_mmiorsp_tx  = mqueue_open(mq_array[5].name,  mq_array[5].perm_flag);
  app2sim_portctrl_req_rx = mqueue_open(mq_array[6].name,  mq_array[6].perm_flag);
  app2sim_dealloc_rx  = mqueue_open(mq_array[7].name,  mq_array[7].perm_flag);
  sim2app_dealloc_tx  = mqueue_open(mq_array[8].name,  mq_array[8].perm_flag);
  sim2app_portctrl_rsp_tx = mqueue_open(mq_array[9].name,  mq_array[9].perm_flag);

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

  // Open Buffer info log
  fp_workspace_log = fopen("workspace_info.log", "wb");
  if (fp_workspace_log == (FILE*)NULL)
    {
      ase_error_report("fopen", errno, ASE_OS_FOPEN_ERR);
    }
  else
    {
      printf("SIM-C : Information about opened workspaces => workspace_info.log \n");
    }

  fflush(stdout);

  FUNC_CALL_EXIT;
  return 0;
}


// -----------------------------------------------------------------------
// ASE ready indicator:  Print a message that ASE is ready to go.
// Controls run-modes
// -----------------------------------------------------------------------
int ase_ready()
{
  FUNC_CALL_ENTRY;

  // App run command
  app_run_cmd = ase_malloc (ASE_FILEPATH_LEN);
  memset (app_run_cmd, 0, ASE_FILEPATH_LEN);

  // Set test_cnt to 0
  glbl_test_cmplt_cnt = 0;

  // Indicate readiness with .ase_ready file
  ase_ready_filepath = ase_malloc (ASE_FILEPATH_LEN);
  sprintf(ase_ready_filepath, "%s/%s", ase_workdir_path, ASE_READY_FILENAME);

  // Write .ase_ready file
  fp_ase_ready = fopen( ase_ready_filepath, "w");
  if (fp_ase_ready != NULL) 
    {
      fprintf(fp_ase_ready, "%d", ase_pid);
      fclose(fp_ase_ready);
    }
  else
    {
      printf("SIM-C : Error creating ready file\n");
      ase_error_report("fopen", errno, ASE_OS_FOPEN_ERR);
    }

  // Display "Ready for simulation"
  BEGIN_GREEN_FONTCOLOR;
  printf("SIM-C : ** ATTENTION : BEFORE running the software application **\n");
  printf("        Run the following command into terminal where application will run (copy-and-paste) =>\n");
  printf("        $SHELL   | Run:\n");
  printf("        ---------+---------------------------------------------------\n");
  printf("        bash     | export ASE_WORKDIR=%s\n", ase_run_path);
  printf("        tcsh/csh | setenv ASE_WORKDIR %s\n", ase_run_path);
  printf("        For any other $SHELL, consult your Linux administrator\n");
  printf("\n");
  END_GREEN_FONTCOLOR;

  // Run ase_regress.sh here
  if (cfg->ase_mode == ASE_MODE_REGRESSION)
    {
      printf("Starting ase_regress.sh script...\n");
      if ( (sv2c_script_filepath != NULL) && (strlen(sv2c_script_filepath)!= 0) )
	{
	  sprintf(app_run_cmd, "%s &", sv2c_script_filepath);
	}
      else
	{
	  strncpy(app_run_cmd, "./ase_regress.sh &", ASE_FILEPATH_LEN);
	}
      system(app_run_cmd);
    }
  else
    {
      BEGIN_GREEN_FONTCOLOR;
      printf("SIM-C : Ready for simulation...\n");
      printf("SIM-C : Press CTRL-C to close simulator...\n");
      END_GREEN_FONTCOLOR;
    }

  fflush(stdout);

  FUNC_CALL_EXIT;
  return 0;
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
  //ase_mqueue_teardown();
  // Close message queues
  mqueue_close(app2sim_alloc_rx);       
  mqueue_close(sim2app_alloc_tx);       
  mqueue_close(app2sim_mmioreq_rx);
  mqueue_close(sim2app_mmiorsp_tx);
  mqueue_close(app2sim_umsg_rx);
  mqueue_close(app2sim_simkill_rx);
  mqueue_close(app2sim_portctrl_req_rx);
  mqueue_close(app2sim_dealloc_rx);       
  mqueue_close(sim2app_dealloc_tx);       

  int ipc_iter;
  for(ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_destroy(mq_array[ipc_iter].name);

  free(mq_array);

  // Destroy all open shared memory regions
  printf("SIM-C : Unlinking Shared memory regions.... \n");
  // ase_destroy();

  // *FIXME* Remove the ASE timestamp file
  if (unlink(tstamp_filepath) == -1)
    {
      printf("SIM-C : %s could not be deleted, please delete manually... \n", TSTAMP_FILENAME);
    }

  // Final clean of IPC
  final_ipc_cleanup();

  // Close workspace log
  if (fp_workspace_log != NULL)
    {
      fclose(fp_workspace_log);
    }

#ifdef ASE_DEBUG
  if (fp_memaccess_log != NULL) 
    {
      fclose(fp_memaccess_log);
    }
  if (fp_pagetable_log != NULL) 
    {
      fclose(fp_pagetable_log);
    }
#endif

  // Remove session files
  printf("SIM-C : Cleaning session files...\n");
  if ( unlink(ase_ready_filepath) == -1 )
    {
      BEGIN_RED_FONTCOLOR;
      printf("Session file %s could not be removed, please remove manually !!\n", ASE_READY_FILENAME);
      END_RED_FONTCOLOR;
    }

  // Print location of log files
  BEGIN_GREEN_FONTCOLOR;
  printf("SIM-C : Simulation generated log files\n");
  printf("        Transactions file   | $ASE_WORKDIR/ccip_transactions.tsv\n");
  printf("        Workspaces info     | $ASE_WORKDIR/workspace_info.log\n");
  printf("        Protocol Warnings   | $ASE_WORKDIR/ccip_errors_and_warnings.txt\n");
  printf("        ASE seed            | $ASE_WORKDIR/ase_seed.txt\n");
  END_GREEN_FONTCOLOR;

  // Send a simulation kill command
  printf("SIM-C : Sending kill command...\n");
  usleep(1000);
  
  // Set scope
  svSetScope(scope);

  simkill();

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

  FILE *fp = (FILE *)NULL;
  char *line;
  size_t len = 0;
  // ssize_t read;
  char *parameter;
  int value;

  char *ase_cfg_filepath;
  ase_cfg_filepath = ase_malloc(256);
  memset (ase_cfg_filepath, 0, 256);
  // if ( strlen(sv2c_config_filepath) != 0 )
  if ( (strlen(sv2c_config_filepath) != 0) && (sv2c_config_filepath!=(char*)NULL))
    {
      sprintf(ase_cfg_filepath, "%s", sv2c_config_filepath);
    }
  else
    {
      sprintf(ase_cfg_filepath, "%s/%s", ase_run_path, ASE_CONFIG_FILE);
    }

  // Allocate space to store ASE config
  cfg = (struct ase_cfg_t *)ase_malloc( sizeof(struct ase_cfg_t) );
  if (cfg == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : ASE config structure could not be allocated... EXITING\n");
      END_RED_FONTCOLOR;
      ase_error_report("malloc", errno, ASE_OS_MALLOC_ERR);
      start_simkill_countdown();
    }
  line = ase_malloc(sizeof(char) * 80);

  // Default values
  cfg->ase_mode = ASE_MODE_DAEMON_NO_SIMKILL;
  cfg->ase_timeout = 500;
  cfg->ase_num_tests = 1;
  cfg->enable_reuse_seed = 0;
  cfg->enable_cl_view = 1;
  cfg->phys_memory_available_gb = 256;

  // Find ase.cfg OR not
  // if ( access (ASE_CONFIG_FILE, F_OK) != -1 )
  if ( access (ase_cfg_filepath, F_OK) != -1 )
    {
      // FILE exists, overwrite
      printf("SIM-C : Reading %s configuration file\n", ASE_CONFIG_FILE);
      fp = fopen(ase_cfg_filepath, "r");

      // Parse file line by line
      // while ((read = getline(&line, &len, fp)) != -1)
      while (getline(&line, &len, fp) != -1)
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
	      else if (strcmp (parameter,"ENABLE_CL_VIEW") == 0)
		cfg->enable_cl_view = value;
	      else if (strcmp(parameter,"PHYS_MEMORY_AVAILABLE_GB") == 0)
		{
		  if (value < 0)
		    {
		      BEGIN_RED_FONTCOLOR;
		      printf("SIM-C : Physical memory size is negative in %s\n", ASE_CONFIG_FILE);
		      printf("        Reverting to default 256 GB\n");
		      END_RED_FONTCOLOR;
		    }
		  else
		    {
		      cfg->phys_memory_available_gb = value;
		    }
		}
	      else
	      	printf("SIM-C : In config file %s, Parameter type %s is unidentified \n", ASE_CONFIG_FILE, parameter);
	    }
	}

      // ASE mode control
      switch (cfg->ase_mode)
	{
	  // Classic Server client mode
	case ASE_MODE_DAEMON_NO_SIMKILL:
	  printf("SIM-C : ASE was started in Mode 1 (Server-Client without SIMKILL)\n");
	  cfg->ase_timeout = 0;
	  cfg->ase_num_tests = 0;
	  break;

	  // Server Client mode with SIMKILL
	case ASE_MODE_DAEMON_SIMKILL:
	  printf("SIM-C : ASE was started in Mode 2 (Server-Client with SIMKILL)\n");
	  cfg->ase_num_tests = 0;
	  break;

	  // Long runtime mode (SW kills SIM)
	case ASE_MODE_DAEMON_SW_SIMKILL:
	  printf("SIM-C : ASE was started in Mode 3 (Server-Client with Sw SIMKILL (long runs)\n");
	  cfg->ase_timeout = 0;
	  cfg->ase_num_tests = 0;
	  break;

	  // Regression mode (lets an SH file with
	case ASE_MODE_REGRESSION:
	  printf("SIM-C : ASE was started in Mode 4 (Regression mode)\n");
	  cfg->ase_timeout = 0;
	  break;

	  // Illegal modes
	default:
	  printf("SIM-C : ASE mode could not be identified, will revert to ASE_MODE = 1 (Server client w/o SIMKILL)\n");
	  cfg->ase_mode = ASE_MODE_DAEMON_NO_SIMKILL;
	  cfg->ase_timeout = 0;
	  cfg->ase_num_tests = 0;
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
    case ASE_MODE_DAEMON_NO_SIMKILL : printf("Server-Client mode without SIMKILL\n") ; break ;
    case ASE_MODE_DAEMON_SIMKILL    : printf("Server-Client mode with SIMKILL\n") ; break ;
    case ASE_MODE_DAEMON_SW_SIMKILL : printf("Server-Client mode with SW SIMKILL (long runs)\n") ; break ;
    case ASE_MODE_REGRESSION        : printf("ASE Regression mode\n") ; break ;
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

  // CL view
  if (cfg->enable_cl_view != 0)
    printf("        ASE Transaction view       ... ENABLED\n");
  else
    printf("        ASE Transaction view       ... DISABLED\n");

  // GBs of physical memory available
  printf("        Amount of physical memory  ... %d GB\n", cfg->phys_memory_available_gb);

  END_YELLOW_FONTCOLOR;

  // Transfer data to hardware (for simulation only)
#ifdef SIM_SIDE
  ase_config_dex(cfg);
#endif

  FUNC_CALL_EXIT;
}

