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
 * Module Info:
 * - Protocol backend for keeping IPCs alive
 * - Interfacing with DPI-C, messaging
 * - Interface to page table
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
char umsg_mode_msg[ASE_LOGGER_LEN];

// Session status
int session_empty;

// MMIO Respons lock
// pthread_mutex_t mmio_resp_lock;

// User clock frequency
float f_usrclk;

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

  //FILE *fp_ready_check = (FILE *)NULL;
  int ase_simv_pid;

  // If Ready file does not exist
  if ( access(ASE_READY_FILENAME, F_OK) == -1)
    {
      ase_simv_pid = 0;
    }
  // If ready file exists
  else
    {
      ase_simv_pid = ase_read_lock_file( getenv("PWD") );
    }

  FUNC_CALL_EXIT;
  return ase_simv_pid;
}


/*
 * DPI: CONFIG path data exchange
 */
void sv2c_config_dex(const char *str)
{
  // Allocate memory
  sv2c_config_filepath = ase_malloc(ASE_FILEPATH_LEN);

  // Check that input string is not NULL
  if (str == NULL)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : sv2c_config_dex => Input string is unusable\n");
      END_YELLOW_FONTCOLOR;
      ase_free_buffer(sv2c_config_filepath);
    }
  else
    {
      // If Malloc fails
      if (sv2c_config_filepath != NULL)
        {
          // Attempt string copy and keep safe
          ase_string_copy(sv2c_config_filepath, str, ASE_FILEPATH_LEN);
#ifdef ASE_DEBUG
          BEGIN_YELLOW_FONTCOLOR;
          printf("  [DEBUG]  sv2c_config_filepath = %s\n", sv2c_config_filepath);
          END_YELLOW_FONTCOLOR;
#endif

          // Check if file exists
          if (access(sv2c_config_filepath, F_OK)==0)
            {
              BEGIN_YELLOW_FONTCOLOR;
              printf("SIM-C : +CONFIG %s file found !\n", sv2c_config_filepath);
              END_YELLOW_FONTCOLOR;
            }
          else
            {
              BEGIN_RED_FONTCOLOR;
              printf("SIM-C : ** WARNING ** +CONFIG file was not found, will revert to DEFAULTS\n");
              END_RED_FONTCOLOR;
              memset(sv2c_config_filepath, 0, ASE_FILEPATH_LEN);
            }
        }
    }
}


/*
 * DPI: SCRIPT path data exchange
 */
void sv2c_script_dex(const char *str)
{
  if (str == NULL)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : sv2c_script_dex => Input string is unusable\n");
      END_YELLOW_FONTCOLOR;
    }
  else
    {
      sv2c_script_filepath = ase_malloc(ASE_FILEPATH_LEN);
      if (sv2c_script_filepath != NULL)
        {
          ase_string_copy(sv2c_script_filepath, str, ASE_FILEPATH_LEN);
#ifdef ASE_DEBUG
          BEGIN_YELLOW_FONTCOLOR;
          printf("  [DEBUG]  sv2c_script_filepath = %s\n", sv2c_script_filepath);
          END_YELLOW_FONTCOLOR;
#endif

          // Check for existance of file
          if (access(sv2c_script_filepath, F_OK)==0)
            {
              BEGIN_YELLOW_FONTCOLOR;
              printf("SIM-C : +SCRIPT %s file found !\n", sv2c_script_filepath);
              END_YELLOW_FONTCOLOR;
            }
          else
            {
              BEGIN_YELLOW_FONTCOLOR;
              printf("SIM-C : ** WARNING ** +SCRIPT file was not found, will revert to DEFAULTS\n");
              memset(sv2c_script_filepath, 0, ASE_FILEPATH_LEN);
              END_YELLOW_FONTCOLOR;
            }
        }
    }
}


/*
 * DPI: Return ASE seed
 */
uint32_t get_ase_seed ()
{
  // return ase_seed;
  return 0xFF;
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

  // Lock channel
  // pthread_mutex_lock (&mmio_resp_lock);

#ifdef ASE_DEBUG
  print_mmiopkt(fp_memaccess_log, "MMIO Got ", mmio_pkt);
#endif

  // Send MMIO Response
  mqueue_send(sim2app_mmiorsp_tx, (char*)mmio_pkt, sizeof(mmio_t));

  // Unlock channel
  // pthread_mutex_unlock (&mmio_resp_lock);

  FUNC_CALL_EXIT;
}


/*
 * DPI: Reset response
 */
void sw_reset_response ()
{
  FUNC_CALL_ENTRY;

  // Send portctrl_rsp message
  mqueue_send(sim2app_portctrl_rsp_tx, completed_str_msg, ASE_MQ_MSGSIZE);

  FUNC_CALL_EXIT;
}


/*
 * Count error flag ping/pong
 */
volatile int count_error_flag;
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


/*
 * Populating required DFH in BBS
 */
// Specific constants
uint64_t *port_vbase;

// Capability CSRs
uint64_t *csr_port_capability;
uint64_t *csr_port_umsg;

// UMSG CSRs
uint64_t *csr_umsg_capability;
uint64_t *csr_umsg_base_address;
uint64_t *csr_umsg_mode;

/*
 * Initialize: Populate FME DFH block
 * When initialized, this is called
 * update*function is called when UMSG is to be set up
 */
void initialize_fme_dfh (struct buffer_t *buf)
{
  FUNC_CALL_ENTRY;

  port_vbase = (uint64_t*) buf->pbase;

  /*
   * PORT CSRs
   */
  // PORT_CAPABILITY
  csr_port_capability = (uint64_t*)((uint64_t)port_vbase + 0x0030);
  *csr_port_capability = (0x100 << 23) + (0x0 << 0);

  // PORT_UMSG DFH
  csr_port_umsg = (uint64_t*)((uint64_t)port_vbase + 0x2000);
  *csr_port_umsg = (uint64_t)((0x3UL << 60) || (0x1000 << 39) || (0x11 << 0));

  /*
   * UMSG settings
   */
  // UMSG_CAPABILITY
  csr_umsg_capability = (uint64_t*)((uint64_t)port_vbase + 0x2008);
  *csr_umsg_capability = (0x0 << 9) + (0x0 << 8) + (0x8 << 0);

  // UMSG_BASE_ADDRESS (only initalize address, update function will update CSR)
  csr_umsg_base_address =(uint64_t*)((uint64_t)port_vbase + 0x2010);

  // UMSG_MODE
  csr_umsg_mode =(uint64_t*)((uint64_t)port_vbase + 0x2018);
  *csr_umsg_mode = 0x0;

  FUNC_CALL_EXIT;
}


// Update FME DFH after UMAS becomes known
void update_fme_dfh(struct buffer_t *umas)
{
  // Write UMAS address
  *csr_umsg_base_address = (uint64_t)umas->pbase;
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

  // Allocate a completed string
  completed_str_msg = (char*)ase_malloc(ASE_MQ_MSGSIZE);
  snprintf(completed_str_msg, 10, "COMPLETED");

  // Simulator is not in lockdown mode (simkill not in progress)
  if (self_destruct_in_progress == 0)
    {
      if (mqueue_recv(app2sim_portctrl_req_rx, (char*)portctrl_msgstr, ASE_MQ_MSGSIZE) == ASE_MSG_PRESENT)
        {
          sscanf(portctrl_msgstr, "%s %d", portctrl_cmd, &portctrl_value);
          if ( memcmp(portctrl_cmd, "AFU_RESET", 9) == 0)
            {
              // AFU Reset control
              portctrl_value = (portctrl_value != 0) ? 1 : 0 ;

              // Wait until transactions clear
              // AFU Reset trigger function will wait until channels clear up
              afu_softreset_trig (0, portctrl_value );

              // Reset response is returned from simulator once queues are cleared
              // Simulator cannot be held up here.
            }
          else if ( memcmp(portctrl_cmd, "UMSG_MODE", 9) == 0)
            {
              // Umsg mode setting here
              glbl_umsgmode = portctrl_value & 0xFFFFFFFF;
              snprintf(umsg_mode_msg, ASE_LOGGER_LEN, "UMSG Mode mask set to 0x%x", glbl_umsgmode);
              printf("SIM-C : %s\n", umsg_mode_msg);
              buffer_msg_inject(1, umsg_mode_msg);

              // Send portctrl_rsp message
              mqueue_send(sim2app_portctrl_rsp_tx, completed_str_msg, ASE_MQ_MSGSIZE);
            }
          else if ( memcmp(portctrl_cmd, "ASE_INIT", 8) == 0)
            {
              printf("Session requested by PID = %d\n", portctrl_value);
              // Generate new timestamp
              put_timestamp();
              tstamp_filepath = ase_malloc(ASE_FILEPATH_LEN);
              snprintf(tstamp_filepath, ASE_FILEPATH_LEN, "%s/%s", ase_workdir_path, TSTAMP_FILENAME);
              // Print timestamp
              glbl_session_id = ase_malloc(20);
              glbl_session_id = get_timestamp(0);
              if (glbl_session_id == NULL)
                {
                  BEGIN_RED_FONTCOLOR;
                  printf("SIM-C : Session ID could not be allocated, ERROR.. exiting\n");
                  END_RED_FONTCOLOR;
                  start_simkill_countdown();
                }
              else
                {
                  printf("SIM-C : Session ID => %s\n", glbl_session_id );
                }
              session_empty = 0;

              // Send portctrl_rsp message
              mqueue_send(sim2app_portctrl_rsp_tx, completed_str_msg, ASE_MQ_MSGSIZE);
            }
          else if ( memcmp(portctrl_cmd, "ASE_SIMKILL", 11) == 0)
            {
#ifdef ASE_DEBUG
              BEGIN_YELLOW_FONTCOLOR;
              printf("SIM-C : ASE_SIMKILL requested, processing options... \n");
              END_YELLOW_FONTCOLOR;
#endif
              // ------------------------------------------------------------- //
              // Update regression counter
              glbl_test_cmplt_cnt = glbl_test_cmplt_cnt + 1;
              // Mode specific exit behaviour
              if ((cfg->ase_mode == ASE_MODE_DAEMON_NO_SIMKILL) && (session_empty == 0))
                {
                  printf("SIM-C : ASE running in daemon mode (see ase.cfg)\n");
                  printf("        Reseting buffers ... Simulator RUNNING\n");
                  /* while (glbl_dealloc_allowed != 1) */
                  /*   { */
                  /*     printf("Waiting for Session to complete\n"); */
                  /*     sleep(1); */
                  /*   }                   */
                  ase_reset_trig();
                  ase_destroy();
                  BEGIN_GREEN_FONTCOLOR;
                  printf("SIM-C : Ready to run next test\n");
                  END_GREEN_FONTCOLOR;
                  session_empty = 1;
                  buffer_msg_inject(0, TEST_SEPARATOR);
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
              else if (cfg->ase_mode == ASE_MODE_REGRESSION)
                {
                  if (cfg->ase_num_tests == glbl_test_cmplt_cnt)
                    {
                      printf("SIM-C : ASE completed %d tests (see supplied ASE config file)... Simulator will EXIT\n", cfg->ase_num_tests);
                      run_clocks (500);
                      ase_perror_teardown();
                      start_simkill_countdown();
                    }
                  else
                    {
                      ase_reset_trig();
                    }
                }

              // Check for simulator sanity -- if transaction counts dont match
              // Kill the simulation ASAP -- DEBUG feature only
#ifdef ASE_DEBUG
              count_error_flag_ping();
              if (count_error_flag != 0)
                {
                  BEGIN_RED_FONTCOLOR;
                  printf("SIM-C : ** ERROR ** Transaction counts do not match, something got lost\n");
                  END_RED_FONTCOLOR;
                  run_clocks (500);
                  ase_perror_teardown();
                  start_simkill_countdown();
                }
#endif

              // Send portctrl_rsp message
              mqueue_send(sim2app_portctrl_rsp_tx, completed_str_msg, ASE_MQ_MSGSIZE);
            }
          else
            {
              BEGIN_RED_FONTCOLOR;
              printf("SIM-C : Undefined Port Control function ... IGNORING\n");
              END_RED_FONTCOLOR;

              // Send portctrl_rsp message
              mqueue_send(sim2app_portctrl_rsp_tx, completed_str_msg, ASE_MQ_MSGSIZE);
            }
        }

      // ------------------------------------------------------------------------------- //

      /*
       * Buffer Allocation Replicator
       */
      struct buffer_t ase_buffer;
      char logger_str[ASE_LOGGER_LEN];
      char incoming_alloc_msgstr[ASE_MQ_MSGSIZE];
      memset(incoming_alloc_msgstr, 0, ASE_MQ_MSGSIZE);

      // Receive a DPI message and get information from replicated buffer
      ase_empty_buffer(&ase_buffer);
      if (mqueue_recv(app2sim_alloc_rx, (char*)incoming_alloc_msgstr, ASE_MQ_MSGSIZE)==ASE_MSG_PRESENT)
        {
          // Typecast string to buffer_t
          memcpy((char*)&ase_buffer, incoming_alloc_msgstr, sizeof(struct buffer_t));

          // Allocate action
          ase_alloc_action(&ase_buffer);
          ase_buffer.is_privmem = 0;
          if (ase_buffer.index == 0)
            {
              ase_buffer.is_mmiomap = 1;
            }
          else
            {
              ase_buffer.is_mmiomap = 0;
            }

          // Format workspace info string
          memset (logger_str, 0, ASE_LOGGER_LEN);
          if (ase_buffer.is_mmiomap)
            {
              snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "MMIO map Allocated ");
              initialize_fme_dfh(&ase_buffer);
            }
          else if (ase_buffer.is_umas)
            {
              snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "UMAS Allocated ");
              update_fme_dfh(&ase_buffer);
            }
          else
            {
              snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "Buffer %d Allocated ", ase_buffer.index);
            }
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, " (located /dev/shm/%s) =>\n", ase_buffer.memname);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\t\tHost App Virtual Addr  = %p\n", (void*)ase_buffer.vbase);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\t\tHW Physical Addr       = %p\n", (void*)ase_buffer.fake_paddr);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\t\tHW CacheAligned Addr   = %p\n", (void*)(ase_buffer.fake_paddr >> 6));
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\t\tWorkspace Size (bytes) = %d\n", ase_buffer.memsize);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\n");

          // Inject buffer message
          buffer_msg_inject (1, logger_str );

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
      char incoming_dealloc_msgstr[ASE_MQ_MSGSIZE];
      memset(incoming_dealloc_msgstr, 0, ASE_MQ_MSGSIZE);

      ase_empty_buffer(&ase_buffer);
      if (mqueue_recv(app2sim_dealloc_rx, (char*)incoming_dealloc_msgstr, ASE_MQ_MSGSIZE)==ASE_MSG_PRESENT)
        {
          // Typecast string to buffer_t
          memcpy((char*)&ase_buffer, incoming_dealloc_msgstr, sizeof(struct buffer_t));

          // Format workspace info string
          memset (logger_str, 0, ASE_LOGGER_LEN);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\nBuffer %d Deallocated =>\n", ase_buffer.index);
          snprintf(logger_str + strlen(logger_str), ASE_LOGGER_LEN, "\n");

          // Deallocate action
          ase_dealloc_action(&ase_buffer, 1);

          // Inject buffer message
          buffer_msg_inject (1, logger_str );

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

      // ------------------------------------------------------------------------------- //
      /*
       * MMIO request listener
       */
      // Message string
      struct mmio_t *mmio_pkt;
      mmio_pkt = (struct mmio_t *)ase_malloc( sizeof(mmio_t) );

      // Receive csr_write packet
      if(mqueue_recv(app2sim_mmioreq_rx, (char*)mmio_pkt, sizeof(mmio_t) )==ASE_MSG_PRESENT)
        {
#ifdef ASE_DEBUG
          print_mmiopkt(fp_memaccess_log, "MMIO Sent", mmio_pkt);
#endif
          mmio_dispatch (0, mmio_pkt);
        }

      // ------------------------------------------------------------------------------- //
      /*
       * UMSG engine
       * *FIXME*: Profiling and costliness analysis needed here
       * *FIXME*: Notification service needs to be built
       */
      char umsg_mapstr[ASE_MQ_MSGSIZE];
      /* struct umsgcmd_t *umsg_pkt; */
      /* umsg_pkt = (struct umsgcmd_t *)ase_malloc(sizeof(struct umsgcmd_t) ); */

      // cleanse string before reading
      if ( mqueue_recv(app2sim_umsg_rx, (char*)umsg_mapstr, sizeof(struct umsgcmd_t) ) == ASE_MSG_PRESENT)
        {
          memcpy(incoming_umsg_pkt, (umsgcmd_t *)umsg_mapstr, sizeof(struct umsgcmd_t));

          // Hint trigger
          incoming_umsg_pkt->hint = (glbl_umsgmode >> (4*incoming_umsg_pkt->id)) & 0xF;

          // dispatch to event processing
          umsg_dispatch(0, incoming_umsg_pkt);
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
  // setvbuf(stdout, NULL, _IONBF, 0);
  setbuf(stdout, NULL);

  // Set self_destruct flag = 0, SIMulator is not in lockdown
  self_destruct_in_progress = 0;

  // Graceful kill handlers
  register_signal(SIGTERM, start_simkill_countdown);
  register_signal(SIGINT , start_simkill_countdown);
  register_signal(SIGQUIT, start_simkill_countdown);
  register_signal(SIGHUP,  start_simkill_countdown);

  // Runtime error handler (print backtrace)
  register_signal(SIGSEGV, backtrace_handler);
  register_signal(SIGBUS, backtrace_handler);
  register_signal(SIGABRT, backtrace_handler);

  // Ignore SIGPIPE *FIXME*: Look for more elegant solution
  signal(SIGPIPE, SIG_IGN);

  // Get PID
  ase_pid = getpid();
  printf("SIM-C : PID of simulator is %d\n", ase_pid);

  // Allocate incoming_umsg_pkt
  incoming_umsg_pkt = (struct umsgcmd_t *)ase_malloc(sizeof(struct umsgcmd_t) );

  // ASE configuration management
  // ase_config_parse(ASE_CONFIG_FILE);
  ase_config_parse ( sv2c_config_filepath );

  // Evaluate IPCs
  ipc_init();

  printf("SIM-C : Current Directory located at =>\n");
  printf("        %s\n", ase_workdir_path);

  // Create IPC cleanup setup
  create_ipc_listfile();

  // Sniffer file stat path
  ccip_sniffer_file_statpath = ase_malloc(ASE_FILEPATH_LEN);
  snprintf(ccip_sniffer_file_statpath, ASE_FILEPATH_LEN, "%s/ccip_warning_and_errors.txt", ase_workdir_path);

  // Remove existing error log files from previous run
  BEGIN_YELLOW_FONTCOLOR;
  if ( access(ccip_sniffer_file_statpath, F_OK) == 0)
    {
      if (unlink(ccip_sniffer_file_statpath) == 0)
        {
          printf("SIM-C : Removed sniffer log file from previous run\n");
        }
    }
  END_YELLOW_FONTCOLOR;

  /*
   * Debug logs
   */
#ifdef ASE_DEBUG
  // Create a memory access log
  fp_memaccess_log = fopen("aseafu_access.log", "w");
  if (fp_memaccess_log == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [DEBUG]  Memory access debug logger initialization failed !\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [DEBUG]  Memory access debug logger initialized\n");
      END_YELLOW_FONTCOLOR;
    }

  // Page table tracker
  fp_pagetable_log = fopen("ase_pagetable.log", "w");
  if (fp_pagetable_log == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [DEBUG]  ASE pagetable logger initialization failed !\n");
      END_RED_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("  [DEBUG]  ASE pagetable logger initialized\n");
      END_YELLOW_FONTCOLOR;
    }
#endif

  // Set up message queues
  printf("SIM-C : Creating Messaging IPCs...\n");
  int ipc_iter;
  for( ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_create( mq_array[ipc_iter].name );

  // Open message queues
  app2sim_alloc_rx        = mqueue_open(mq_array[0].name,  mq_array[0].perm_flag);
  app2sim_mmioreq_rx      = mqueue_open(mq_array[1].name,  mq_array[1].perm_flag);
  app2sim_umsg_rx         = mqueue_open(mq_array[2].name,  mq_array[2].perm_flag);
  sim2app_alloc_tx        = mqueue_open(mq_array[3].name,  mq_array[3].perm_flag);
  sim2app_mmiorsp_tx      = mqueue_open(mq_array[4].name,  mq_array[4].perm_flag);
  app2sim_portctrl_req_rx = mqueue_open(mq_array[5].name,  mq_array[5].perm_flag);
  app2sim_dealloc_rx      = mqueue_open(mq_array[6].name,  mq_array[6].perm_flag);
  sim2app_dealloc_tx      = mqueue_open(mq_array[7].name,  mq_array[7].perm_flag);
  sim2app_portctrl_rsp_tx = mqueue_open(mq_array[8].name,  mq_array[8].perm_flag);
  sim2app_intr_request_tx = mqueue_open(mq_array[9].name,  mq_array[9].perm_flag);

  // Calculate memory map regions
  printf("SIM-C : Calculating memory map...\n");
  calc_phys_memory_ranges();

  // Random number for csr_pinned_addr
  /* if (cfg->enable_reuse_seed) */
  /*   { */
  /*     ase_seed = ase_read_seed (); */
  /*   } */
  /* else */
  /*   { */
  /*     ase_seed = generate_ase_seed(); */
  /*     ase_write_seed ( ase_seed ); */
  /*   } */
  ase_write_seed (cfg->ase_seed);
  srand(cfg->ase_seed);

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

  // Set test_cnt to 0
  glbl_test_cmplt_cnt = 0;

  // Write lock file
  ase_write_lock_file();

  // Display "Ready for simulation"
  BEGIN_GREEN_FONTCOLOR;
  printf("SIM-C : ** ATTENTION : BEFORE running the software application **\n");
  printf("        Set env(ASE_WORKDIR) in terminal where application will run (copy-and-paste) =>\n");
  printf("        $SHELL   | Run:\n");
  printf("        ---------+---------------------------------------------------\n");
  printf("        bash/zsh | export ASE_WORKDIR=%s\n", ase_workdir_path);
  printf("        tcsh/csh | setenv ASE_WORKDIR %s\n", ase_workdir_path);
  printf("        For any other $SHELL, consult your Linux administrator\n");
  printf("\n");
  END_GREEN_FONTCOLOR;

  // Run ase_regress.sh here
  if (cfg->ase_mode == ASE_MODE_REGRESSION)
    {
      printf("Starting ase_regress.sh script...\n");
      if ( (sv2c_script_filepath != NULL) && (strlen(sv2c_script_filepath)!= 0) )
        {
          snprintf(app_run_cmd, ASE_FILEPATH_LEN, "%s &", sv2c_script_filepath);
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

#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [DEBUG]  Caught a SIG\n");
  END_YELLOW_FONTCOLOR;
#endif

  // Close and unlink message queue
  printf("SIM-C : Closing message queue and unlinking...\n");

  // Close message queues
  mqueue_close(app2sim_alloc_rx);
  mqueue_close(sim2app_alloc_tx);
  mqueue_close(app2sim_mmioreq_rx);
  mqueue_close(sim2app_mmiorsp_tx);
  mqueue_close(app2sim_umsg_rx);
  mqueue_close(app2sim_portctrl_req_rx);
  mqueue_close(app2sim_dealloc_rx);
  mqueue_close(sim2app_dealloc_tx);
  mqueue_close(sim2app_portctrl_rsp_tx);
  mqueue_close(sim2app_intr_request_tx);

  int ipc_iter;
  for(ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_destroy(mq_array[ipc_iter].name);

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
  printf("        Transactions file       | $ASE_WORKDIR/ccip_transactions.tsv\n");
  printf("        Workspaces info         | $ASE_WORKDIR/workspace_info.log\n");
  END_GREEN_FONTCOLOR;
  if ( access(ccip_sniffer_file_statpath, F_OK) != -1 )
    {
      BEGIN_RED_FONTCOLOR;
      printf("        Protocol warning/errors | $ASE_WORKDIR/ccip_warning_and_errors.txt\n");
      END_RED_FONTCOLOR;
    }
  BEGIN_GREEN_FONTCOLOR;
  printf("        ASE seed                | $ASE_WORKDIR/ase_seed.txt\n");
  END_GREEN_FONTCOLOR;

  // Send a simulation kill command
  printf("SIM-C : Sending kill command...\n");
  usleep(1000);

  // Set scope
  svSetScope(scope);

  // Free memories
  // ase_free_buffer (ase_workdir_path);
  free(cfg);
  free(ase_ready_filepath);
  free(incoming_umsg_pkt);

  // Issue Simulation kill
  simkill();

  FUNC_CALL_EXIT;
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
  char *parameter;
  int value;
  char *pch;

  /* char *ase_cfg_filepath; */
  /* ase_cfg_filepath = ase_malloc(256); */

  /* if ( access(sv2c_config_filepath, F_OK) != -1 ) */
  /*   { */
  /*     /\* if ( (strlen(sv2c_config_filepath) != 0) && (sv2c_config_filepath!=(char*)NULL)) *\/ */
  /*     /\*        { *\/ */
  /*     // snprintf(ase_cfg_filepath, 256, "%s", sv2c_config_filepath); */
  /*     /\* } *\/ */
  /* ase_string_copy(ase_cfg_filepath, sv2c_config_filepath, 256); */
  /*   } */

  // ase_string_copy(ase_cfg_filepath, sv2c_config_filepath, 256);

  // Allocate space to store ASE config
  cfg = (struct ase_cfg_t *)ase_malloc( sizeof(struct ase_cfg_t) );
  if (cfg == (struct ase_cfg_t *)NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : ASE config structure could not be allocated... EXITING\n");
      END_RED_FONTCOLOR;
      ase_error_report("malloc", errno, ASE_OS_MALLOC_ERR);
      start_simkill_countdown();
    }
  else
    {
      // Allocate memory to store a line
      line = ase_malloc(sizeof(char) * 80);

      // Default values
      cfg->ase_mode                 = ASE_MODE_DAEMON_NO_SIMKILL;
      cfg->ase_timeout              = 50000;
      cfg->ase_num_tests            = 1;
      cfg->enable_reuse_seed        = 0;
      cfg->ase_seed                 = 9876;
      cfg->enable_cl_view           = 1;
      cfg->usr_tps                  = DEFAULT_USR_CLK_TPS;
      cfg->phys_memory_available_gb = 256;

      // Fclk Mhz
      f_usrclk = DEFAULT_USR_CLK_MHZ;

      // Find ase.cfg OR not
      if ( access (filename, F_OK) != -1 )
        {
          // FILE exists, overwrite
          fp = fopen(filename, "r");
          if (fp == NULL)
            {
              BEGIN_RED_FONTCOLOR;
              printf("SIM-C : %s supplied by +CONFIG could not be opened, IGNORED\n", filename);
              END_RED_FONTCOLOR;
            }
          else
            {
              BEGIN_GREEN_FONTCOLOR;
              printf("SIM-C : Reading %s configuration file \n", filename );
              END_GREEN_FONTCOLOR;
              // Parse file line by line
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
                      if (parameter != NULL)
                        {
                          if (strncmp (parameter,"ASE_MODE", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->ase_mode = atoi(pch);
                                }
                            }
                          else if (strncmp (parameter,"ASE_TIMEOUT", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->ase_timeout = atoi(pch);
                                }
                            }
                          else if (strncmp (parameter,"ASE_NUM_TESTS", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->ase_num_tests = atoi(pch);
                                }
                            }
                          else if (strncmp (parameter, "ENABLE_REUSE_SEED", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->enable_reuse_seed =  atoi(pch);
                                }
                            }
                          else if (strncmp (parameter, "ASE_SEED", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->ase_seed =  atoi(pch);
                                }
                            }
                          else if (strncmp (parameter,"ENABLE_CL_VIEW", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  cfg->enable_cl_view =  atoi(pch);
                                }
                            }
                          else if (strncmp (parameter, "USR_CLK_MHZ", 20) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  f_usrclk = atof(pch);
                                  if (f_usrclk == 0.000000)
                                    {
                                      BEGIN_RED_FONTCOLOR;
                                      printf("SIM-C : User Clock Frequency cannot be 0.000 MHz\n");
                                      printf("        Reverting to %f MHz\n", DEFAULT_USR_CLK_MHZ);
                                      f_usrclk = DEFAULT_USR_CLK_MHZ;
                                      cfg->usr_tps = DEFAULT_USR_CLK_TPS;
                                      END_RED_FONTCOLOR;
                                    }
                                  else if (f_usrclk == DEFAULT_USR_CLK_MHZ)
                                    {
                                      cfg->usr_tps = DEFAULT_USR_CLK_TPS;
                                    }
                                  else
                                    {
                                      cfg->usr_tps = (int)( 1E+12/(f_usrclk*pow(1000,2)) );
#ifdef ASE_DEBUG
                                      BEGIN_YELLOW_FONTCOLOR;
                                      printf("  [DEBUG]  usr_tps = %d\n", cfg->usr_tps);
                                      END_YELLOW_FONTCOLOR;
#endif
                                      if (f_usrclk != DEFAULT_USR_CLK_MHZ)
                                        {
                                          BEGIN_RED_FONTCOLOR;
                                          printf("SIM-C : User clock Frequency was modified from %f to %f MHz\n", DEFAULT_USR_CLK_MHZ, f_usrclk);
                                          printf("        **WARNING** Modifying User Clock is not supported in-system !\n");
                                          END_RED_FONTCOLOR;
                                        }
                                    }
                                }
                            }
                          else if (strncmp(parameter,"PHYS_MEMORY_AVAILABLE_GB", 32) == 0)
                            {
                              pch = strtok(NULL, "");
                              if (pch != NULL)
                                {
                                  value = atoi(pch);
                                  if (value < 0)
                                    {
                                      BEGIN_RED_FONTCOLOR;
                                      printf("SIM-C : Physical memory size is negative in %s\n", filename);
                                      printf("        Reverting to default 256 GB\n");
                                      END_RED_FONTCOLOR;
                                    }
                                  else
                                    {
                                      cfg->phys_memory_available_gb = value;
                                    }
                                }
                            }
                          else
                            {
                              printf("SIM-C : In config file %s, Parameter type %s is unidentified \n", filename, parameter);
                            }
                        }
                    }
                }
            }

          /*
           * ASE mode control
           */
          BEGIN_YELLOW_FONTCOLOR;
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
          if (fp != NULL)
            {
              fclose(fp);
            }

        }
      else
        {
          // FILE does not exist
          printf("SIM-C : %s not found, using default values\n", filename);
        }

      // Mode configuration
      printf("        ASE mode                   ... ");
      switch (cfg->ase_mode)
        {
        case ASE_MODE_DAEMON_NO_SIMKILL : printf("Server-Client mode without SIMKILL\n") ; break ;
        case ASE_MODE_DAEMON_SIMKILL    : printf("Server-Client mode with SIMKILL\n") ; break ;
        case ASE_MODE_DAEMON_SW_SIMKILL : printf("Server-Client mode with SW SIMKILL (long runs)\n") ; break ;
        case ASE_MODE_REGRESSION        : printf("ASE Regression mode\n") ; break ;
        }

      // Inactivity
      if (cfg->ase_mode == ASE_MODE_DAEMON_SIMKILL)
        printf("        Inactivity kill-switch     ... ENABLED after %d clocks \n", cfg->ase_timeout);
      else
        printf("        Inactivity kill-switch     ... DISABLED \n");

      // Reuse seed
      if (cfg->enable_reuse_seed != 0)
        printf("        Reuse simulation seed      ... ENABLED \n");
      else
        {
          printf("        Reuse simulation seed      ... DISABLED (will create one at $ASE_WORKDIR/ase_seed.txt) \n");
          cfg->ase_seed = generate_ase_seed();
        }

      // ASE will be run with this seed
      printf("        ASE Seed                   ... %d \n", cfg->ase_seed);

      // CL view
      if (cfg->enable_cl_view != 0)
        printf("        ASE Transaction view       ... ENABLED\n");
      else
        printf("        ASE Transaction view       ... DISABLED\n");

      // User clock frequency
      printf("        User Clock Frequency       ... %.6f MHz, T_uclk = %d ps \n", f_usrclk, cfg->usr_tps);
      if (f_usrclk != DEFAULT_USR_CLK_MHZ)
        {
          printf("        ** NOTE **: User Clock Frequency was changed from default %f MHz !\n", DEFAULT_USR_CLK_MHZ);
        }

      // GBs of physical memory available
      printf("        Amount of physical memory  ... %d GB\n", cfg->phys_memory_available_gb);
      END_YELLOW_FONTCOLOR;

      // Transfer data to hardware (for simulation only)
#ifdef SIM_SIDE
      ase_config_dex(cfg);
#endif

      // free memory
      free(line);
    }

  // Free cfg filepath
  // free(ase_cfg_filepath);

  FUNC_CALL_EXIT;
}
