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
 * Module Info: Memory Model operations (C module)
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 * 
 * Purpose: Keeping cci_to_mem_translator.c clutter free and modular
 * test and debug. Includes message queue management by DPI.
 * NOTE: These functions must be called by DPI side ONLY.
 */

#include "ase_common.h"

#if 0
// Message queues opened by DPI
int app2sim_rx;           // app2sim mesaage queue in RX mode
int sim2app_tx;           // sim2app mesaage queue in TX mode
int app2sim_mmioreq_rx;    // CSR Write listener MQ in RX mode
int app2sim_umsg_rx;      // UMsg listener MQ in RX mode
int app2sim_simkill_rx;   // Simkill listener in RX mode
int sim2app_intr_tx;      // INTR message queue in TX mode
#endif

// '1' indicates that teardown is in progress
int self_destruct_in_progress = 0;

// FPGA offset aggregator
// uint64_t fpga_membase_so_far = 0;


// ---------------------------------------------------------------------
// ase_mqueue_setup() : Set up DPI message queues
// Set up app2sim_rx, sim2app_tx and app2sim_mmio_rx message queues
// ---------------------------------------------------------------------
#if 0
void ase_mqueue_setup()
{
  FUNC_CALL_ENTRY;

#if 0
  mq_unlink(APP2SIM_SMQ_PREFIX);
  mq_unlink(SIM2APP_SMQ_PREFIX);
  mq_unlink(APP2SIM_CSR_WR_SMQ_PREFIX);
  mq_unlink(APP2SIM_UMSG_SMQ_PREFIX);

  // Depending on the calling function, activate the required queues
  app2sim_rx         = mqueue_create(APP2SIM_SMQ_PREFIX,         O_CREAT|O_RDONLY );
  sim2app_tx         = mqueue_create(SIM2APP_SMQ_PREFIX,         O_CREAT|O_WRONLY );
  app2sim_mmio_rx  = mqueue_create(APP2SIM_CSR_WR_SMQ_PREFIX,  O_CREAT|O_RDONLY );
  app2sim_umsg_rx    = mqueue_create(APP2SIM_UMSG_SMQ_PREFIX,    O_CREAT|O_RDONLY );
#if 0
  sim2app_intr_tx    = mqueue_create(SIM2APP_INTR_SMQ_PREFIX,    O_CREAT|O_WRONLY );
#endif
  app2sim_simkill_rx = mqueue_create(APP2SIM_SIMKILL_SMQ_PREFIX, O_CREAT|O_RDONLY );

#endif

  FUNC_CALL_EXIT;
}
#endif

// ---------------------------------------------------------------------
// ase_mqueue_teardown(): Teardown DPI message queues
// Close and unlink DPI message queues
// ---------------------------------------------------------------------
void ase_mqueue_teardown()
{
  FUNC_CALL_ENTRY;

  // Close message queues
  mqueue_close(app2sim_rx);       
  mqueue_close(sim2app_tx);       
  mqueue_close(app2sim_mmioreq_rx);
  mqueue_close(sim2app_mmiorsp_tx);
  mqueue_close(app2sim_umsg_rx);
#if 0
  mqueue_close(sim2app_intr_tx);       
#endif
  mqueue_close(app2sim_simkill_rx);

  int ipc_iter;
  for(ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_destroy(mq_array[ipc_iter].name);

  FUNC_CALL_EXIT;
}


// ---------------------------------------------------------------
// DPI Self destruct - Called if: error() occurs 
// Deallocate & Unlink all shared memories and message queues
// ---------------------------------------------------------------
void ase_perror_teardown()
{
  FUNC_CALL_ENTRY;

  // close the log file first, if exists
/* #ifdef ASE_CCI_TRANSACTION_LOGGER  */
/*   printf("SIM-C : Terminating log file !!\n"); */
/*   fclose(ase_cci_log_fd); */
/* #endif */

  //printf("PERROR: Goodbye, Cruel World\n");
  self_destruct_in_progress++;

  if (!self_destruct_in_progress)
    {      
      // Deallocate entire linked list
      ase_destroy();
      
      // Unlink all opened message queues
      ase_mqueue_teardown();
    }

  FUNC_CALL_EXIT;
}


// ------------------------------------------------------------------
// DPI recv message - Set up DPI to receive an 'allocate' request msg
// Receive a string and return a buffer_t structure with memsize,
// memname and index populated. 
// NOTE: This function must be called by DPI
// ------------------------------------------------------------------
int ase_recv_msg(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  char tmp_msg[ASE_MQ_MSGSIZE];

  // Receive a message on mqueue
  if(mqueue_recv(app2sim_rx, tmp_msg)==ASE_MSG_PRESENT)
    {
      // Convert the string to buffer_t
      ase_str_to_buffer_t(tmp_msg, mem);
      FUNC_CALL_EXIT;
      return ASE_MSG_PRESENT;
    }
  else
    {
      FUNC_CALL_EXIT;
      return ASE_MSG_ABSENT;
    }
}


// -------------------------------------------------------------------
// ase_send_msg : Send a ase reply 
// Convert a buffer_t to string and transmit string as a message
// -------------------------------------------------------------------
void ase_send_msg(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  // Temporary buffer
  char tmp_msg[ASE_MQ_MSGSIZE];

  // Convert buffer to string
  ase_buffer_t_to_str(mem, tmp_msg);

  // Send message out
  mqueue_send(sim2app_tx, tmp_msg);

  FUNC_CALL_EXIT;
}


// --------------------------------------------------------------------
// DPI ALLOC buffer action - Allocate buffer action inside DPI
// Receive buffer_t pointer with memsize, memname and index populated
// Calculate fd, pbase and fake_paddr
// --------------------------------------------------------------------
void ase_alloc_action(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  struct buffer_t *new_buf;

#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("SIM-C : Adding a new buffer \"%s\"...\n", mem->memname);
  END_YELLOW_FONTCOLOR;
#endif

  // Obtain a file descriptor
  mem->fd_ase = shm_open(mem->memname, O_RDWR, S_IRUSR|S_IWUSR);
  if(mem->fd_ase < 0)
    {
      /* perror("shm_open"); */
      ase_error_report("shm_open", errno, ASE_OS_SHM_ERR);
      ase_perror_teardown();
      start_simkill_countdown(); // RRS: exit(1);
    }

  // Add to IPC list
#ifdef SIM_SIDE
  add_to_ipc_list ("SHM", mem->memname);
#endif

  // Mmap to pbase, find one with unique low 38 bit
  mem->pbase = (uint64_t)mmap(NULL, mem->memsize, PROT_READ|PROT_WRITE, MAP_SHARED, mem->fd_ase, 0);
  if(mem->pbase == (uint64_t)NULL)
    {
      ase_error_report("mmap", errno, ASE_OS_MEMMAP_ERR);
      /* perror("mmap"); */
      ase_perror_teardown();
      start_simkill_countdown(); // RRS: exit(1);
    }
  ftruncate(mem->fd_ase, (off_t)mem->memsize);

  // Record fake address
  mem->fake_paddr = get_range_checked_physaddr(mem->memsize);
  mem->fake_paddr_hi = mem->fake_paddr + (uint64_t)mem->memsize;

  // Received buffer is valid
  mem->valid = ASE_BUFFER_VALID;

  // Create a buffer and store the information
  new_buf = (struct buffer_t *)ase_malloc(BUFSIZE);
  memcpy(new_buf, mem, BUFSIZE);

  // Append to linked list
  ll_append_buffer(new_buf);

  // Reply to MEM_ALLOC_REQ message with MEM_ALLOC_REPLY
  // Set metadata to reply mode
  mem->metadata = HDR_MEM_ALLOC_REPLY;
  
  // Convert buffer_t to string
  ase_send_msg(mem);

   // If memtest is enabled
#ifdef ASE_MEMTEST_ENABLE
  ase_dbg_memtest(mem);
#endif

  if (mem->index == 0)
    {
      // Pin CSR address
      mmio_afu_vbase = (uint64_t*) mem->pbase;
#ifdef ASE_DEBUG
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : Global CSR Base address = %p\n", (void*)mmio_afu_vbase);
      END_YELLOW_FONTCOLOR;
#endif

      // If UMSG is enabled, write information to CSR region
#if 0
      ase_umsg_init(mem->pbase);
#endif
    }

  FUNC_CALL_EXIT;
}


// --------------------------------------------------------------------
// DPI dealloc buffer action - Deallocate buffer action inside DPI
// Receive index and invalidates buffer
// --------------------------------------------------------------------
void ase_dealloc_action(struct buffer_t *buf)
{
  FUNC_CALL_ENTRY;

  // Traversal pointer
  struct buffer_t *dealloc_ptr;

  // Search buffer and Invalidate
  dealloc_ptr = ll_search_buffer(buf->index);

  //  If deallocate returns a NULL, dont get hosed
  if(dealloc_ptr != NULL)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : Command to invalidate \"%s\" ...\n", dealloc_ptr->memname);
      END_YELLOW_FONTCOLOR;
      dealloc_ptr->valid = ASE_BUFFER_INVALID;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : NULL deallocation request received ... ignoring.\n");
      END_YELLOW_FONTCOLOR;
    }

  FUNC_CALL_EXIT;
}

// --------------------------------------------------------------------
// ase_empty_buffer: create an empty buffer_t object
// Create a buffer with all parameters set to 0
// --------------------------------------------------------------------
void ase_empty_buffer(struct buffer_t *buf)
{
  buf->fd_app = 0;
  buf->fd_ase = 0;
  buf->index = 0;
  buf->valid = ASE_BUFFER_INVALID;
  buf->metadata = 0;
  strcpy(buf->memname, "");
  buf->memsize = 0;
  buf->vbase = (uint64_t)NULL;
  buf->pbase = (uint64_t)NULL;
  buf->fake_paddr = (uint64_t)NULL;
  buf->next = NULL;
}


// --------------------------------------------------------------------
// ase_destroy : Destroy everything, called before exiting
// OPERATION:
// Traverse trough linked list
// - Remove each shared memory region
// - Remove each buffer_t 
// --------------------------------------------------------------------
void ase_destroy()
{
  FUNC_CALL_ENTRY;

  struct buffer_t *ptr;
  int ret;

  char rm_shm_path[50];

  // Traverse through linked list
  ptr = head;
  while(ptr != NULL)
    {
      // Set rm_shm_path to NULLs
      memset(rm_shm_path, '\0', sizeof(rm_shm_path));

      // Unmap Shared memory
      ret = munmap((void*)ptr->pbase, (size_t)ptr->memsize);
      if (ret == -1)
	ase_error_report("munmap", errno, ASE_OS_MEMMAP_ERR);
      /* perror("munmap"); */

      // Unlink related shared memory region
      if(shm_unlink(ptr->memname) != 0)
	  ase_error_report("shm_unlink", errno, ASE_OS_SHM_ERR);
	  /* perror("shm_unlink"); */
      
      // Delete the SHM region
      strcat(rm_shm_path, "rm -f /dev/shm");
      strcat(rm_shm_path, ptr->memname);
      system( rm_shm_path );
      
      // Find and destroy node
      ll_remove_buffer(ptr);
      
      // Traverse to next node
      ptr = ptr->next;
    }
  
  FUNC_CALL_EXIT;
}


// ------------------------------------------------------------------------------
// ase_dbg_memtest : A memory read write test (DEBUG feature)
// To run the test ASE_MEMTEST_ENABLE must be enabled.
// - This test runs alongside a process shm_dbg_memtest.
// - shm_dbg_memtest() is started before MEM_ALLOC_REQ message is sent to DPI
//   The simply starts writing 0xCAFEBABE to memory region
// - ase_dbg_memtest() is started after the MEM_ALLOC_REPLY message is sent back
//   This reads all the data, verifies it is 0xCAFEBABE and writes 0x00000000 there
// PURPOSE: To make sure all the shared memory regions are initialised correctly
// -------------------------------------------------------------------------------
void ase_dbg_memtest(struct buffer_t *mem)
{
  uint32_t *memptr;
  uint32_t *low_addr, *high_addr;

  // Memory test errors counter
  int memtest_errors = 0;

  // Calculate DPI low and high address
  low_addr = (uint32_t*)mem->pbase;
  high_addr = (uint32_t*)((uint64_t)mem->pbase + mem->memsize);

  // Start checker
  for(memptr = low_addr; memptr < high_addr; memptr++)
    {
      if(*memptr != 0xCAFEBABE)
	memtest_errors++;
      *memptr = 0x0;
    }

  // Print result
  if(memtest_errors == 0)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : MEMTEST -> Passed !!\n");
      END_YELLOW_FONTCOLOR;
    }
  else
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : MEMTEST -> Failed with %d errors !!\n", memtest_errors);
      END_YELLOW_FONTCOLOR;
    }
}




/*
 * Range check a Physical address to check if used
 * Created to integrate Sysmem & CAPCM and prevent corner case overwrite
 *   issues (Mon Oct 13 13:33:59 PDT 2014)
 * Operation: When allocating a fake physical address, this function
 * will return an unused physical address range
 * This will be used by SW allocate buffer funtion ONLY
 */
uint64_t get_range_checked_physaddr(uint32_t size)
{
  int unique_physaddr_needed = 1;
  uint64_t ret_fake_paddr;
  uint32_t search_flag;
  uint32_t opposite_flag;  

  // Generate a new address
  while(unique_physaddr_needed)
    {
      // Generate a random physical address for system memory
      ret_fake_paddr = sysmem_phys_lo + ase_rand64() % sysmem_size;
      // 2MB align and sanitize
      // ret_fake_paddr = ret_fake_paddr & 0x00003FFFE00000 ;          
      ret_fake_paddr = ret_fake_paddr & PHYS_ADDR_PREFIX_MASK ;          

      // Check for conditions
      // Is address in sysmem range, go back
      search_flag = check_if_physaddr_used(ret_fake_paddr);

      // Is HI smaller than LO, go back
      opposite_flag = 0;
      if ((ret_fake_paddr + (uint64_t)size) < ret_fake_paddr) 
	opposite_flag = 1;

      // If all OK
      unique_physaddr_needed = search_flag | opposite_flag;
    }
  
  return ret_fake_paddr;
}


/*
 * ASE Physical address to virtual address converter
 * Takes in a simulated physical address from AFU, converts it 
 *   to virtual address
 */
uint64_t* ase_fakeaddr_to_vaddr(uint64_t req_paddr)
{
  FUNC_CALL_ENTRY; 

  // Clean up address of signed-ness
  req_paddr = req_paddr & 0x0000003FFFFFFFFF;

  // DPI pbase address
  uint64_t *ase_pbase;
  
  // This is the real offset to perform read/write
  uint64_t real_offset, calc_pbase;
  
  // Traversal ptr
  struct buffer_t *trav_ptr;

  // For debug only
#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("req_paddr = %p | ", (void *)req_paddr);
  END_YELLOW_FONTCOLOR;
#endif

  // Search which buffer offset_from_pin lies in
  trav_ptr = head;
  while(trav_ptr != NULL)
    {
      if((req_paddr >= trav_ptr->fake_paddr) && (req_paddr < trav_ptr->fake_paddr_hi))
	{
	  real_offset = (uint64_t)req_paddr - (uint64_t)trav_ptr->fake_paddr;
	  calc_pbase = trav_ptr->pbase;
	  ase_pbase = (uint64_t*)(calc_pbase + real_offset);
	  // Debug only
#ifdef ASE_DEBUG
	  BEGIN_YELLOW_FONTCOLOR;
	  printf("offset = 0x%016lx | pbase_off = %p\n", real_offset, (void *)ase_pbase);
	  END_YELLOW_FONTCOLOR;
#endif
	  return ase_pbase;
	}
      else
	{
	  trav_ptr = trav_ptr->next;
	}
    }

  if(trav_ptr == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("@ERROR: ASE has detected a memory operation to an unallocated memory region.\n");
      printf("@ERROR: Simulation cannot continue, please check the code.\n");
      printf("@ERROR: Failure @ phys_addr = %016lx \n", req_paddr );
      printf("@ERROR: See ERROR log file => ase_error.log");
      END_RED_FONTCOLOR;

      // Write error to file
      error_fp = fopen("ase_error.log", "wb");
      fprintf(error_fp, "*** ASE stopped on an illegal memory access ERROR ***\n");
      fprintf(error_fp, "        AFU requested access @ physical memory %p\n", (void*)req_paddr);
      fprintf(error_fp, "        Address not found in requested workspaces listed in workspace_info.log\n");
      fprintf(error_fp, "        Timestamped transaction to this address is listed in transactions.tsv\n");
      fflush(error_fp);
      fclose(error_fp);

      // ase_perror_teardown();
      // final_ipc_cleanup();
      // Request SIMKILL
      start_simkill_countdown(); // RRS: exit(1);
    }

  return (uint64_t*)NOT_OK;

  FUNC_CALL_EXIT;
}

