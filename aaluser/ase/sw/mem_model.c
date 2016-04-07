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


// ---------------------------------------------------------------
// ASE graceful shutdown - Called if: error() occurs 
// Deallocate & Unlink all shared memories and message queues
// ---------------------------------------------------------------
void ase_perror_teardown()
{
  FUNC_CALL_ENTRY;

  self_destruct_in_progress = 1;

  /* if (!self_destruct_in_progress) */
  /*   {       */
  // Deallocate entire linked list
  ase_destroy();
  
  // Unlink all opened message queues
  // ase_mqueue_teardown();
  /* } */

  FUNC_CALL_EXIT;
}


// ------------------------------------------------------------------
// DPI recv message - Set up DPI to receive an 'allocate' request msg
// Receive a string and return a buffer_t structure with memsize,
// memname and index populated. 
// NOTE: This function must be called by DPI
// ------------------------------------------------------------------
#if 0
int ase_recv_msg(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  char tmp_msg[ASE_MQ_MSGSIZE];

  // Receive a message on mqueue
  if(mqueue_recv(app2sim_alloc_rx, tmp_msg)==ASE_MSG_PRESENT)
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
#endif

// -------------------------------------------------------------------
// ase_send_msg : Send a ase reply 
// Convert a buffer_t to string and transmit string as a message
// -------------------------------------------------------------------
#if 0
void ase_send_msg(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  // Temporary buffer
  char tmp_msg[ASE_MQ_MSGSIZE];

  // Convert buffer to string
  ase_buffer_t_to_str(mem, tmp_msg);

  // Send message out
  mqueue_send(sim2app_alloc_tx, tmp_msg);

  FUNC_CALL_EXIT;
}
#endif

// --------------------------------------------------------------------
// DPI ALLOC buffer action - Allocate buffer action inside DPI
// Receive buffer_t pointer with memsize, memname and index populated
// Calculate fd, pbase and fake_paddr
// --------------------------------------------------------------------
void ase_alloc_action(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  struct buffer_t *new_buf;
  int fd_alloc;

#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("SIM-C : Adding a new buffer \"%s\"...\n", mem->memname);
  END_YELLOW_FONTCOLOR;
#endif

  // Obtain a file descriptor
  fd_alloc = shm_open(mem->memname, O_RDWR, S_IRUSR|S_IWUSR);
  if(fd_alloc < 0)
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
  mem->pbase = (uint64_t)mmap(NULL, mem->memsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd_alloc, 0);
  if(mem->pbase == (uint64_t)NULL)
    {
      ase_error_report("mmap", errno, ASE_OS_MEMMAP_ERR);
      /* perror("mmap"); */
      ase_perror_teardown();
      start_simkill_countdown(); // RRS: exit(1);
    }
  ftruncate(fd_alloc, (off_t)mem->memsize);
  close(fd_alloc);

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
#ifdef ASE_LL_VIEW
  BEGIN_YELLOW_FONTCOLOR;
  ll_traverse_print();
  END_YELLOW_FONTCOLOR;
#endif

  // Reply to MEM_ALLOC_REQ message with MEM_ALLOC_REPLY
  // Set metadata to reply mode
  mem->metadata = HDR_MEM_ALLOC_REPLY;
  
  // Convert buffer_t to string
  mqueue_send(sim2app_alloc_tx, (char*)mem, ASE_MQ_MSGSIZE);

   // If memtest is enabled
#ifdef ASE_MEMTEST_ENABLE
  ase_dbg_memtest(mem);
#endif

  if (mem->is_mmiomap == 1)
    {
      // Pin CSR address
      mmio_afu_vbase = (uint64_t*)((uint64_t)mem->pbase + MMIO_AFU_OFFSET);
#ifdef ASE_DEBUG
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : Global CSR Base address = %p\n", (void*)mmio_afu_vbase);
      END_YELLOW_FONTCOLOR;
#endif

      // If UMSG is enabled, write information to CSR region
      // *FIXME*: Maybe BB DFH has to be updated here
    }

#ifdef ASE_DEBUG  
  if (fp_pagetable_log != NULL) 
    {
      if (mem->index % 20 == 0) 
	{
	  fprintf(fp_pagetable_log, 
		  "Index\tAppVBase\tASEVBase\tBufsize\tBufname\t\tPhysBase\n");
	  // "Index\tfd_app\tfd_ase\tAppVBase\tASEVBase\tBufsize\tBufname\t\tPhysBase\n");
	}
      
      fprintf(fp_pagetable_log, 
	      /* "%d\t%d\t%d\t%p\t%p\t%x\t%s\t\t%p\n", */
	      "%d\t%p\t%p\t%x\t%s\t\t%p\n",
	      mem->index,
	      /* mem->fd_app, */
	      /* mem->fd_ase, */
	      (void*)mem->vbase, 
	      (void*)mem->pbase,
	      mem->memsize,
	      mem->memname,
	      (void*)mem->fake_paddr
	      );
    }
#endif

  FUNC_CALL_EXIT;
}


// --------------------------------------------------------------------
// DPI dealloc buffer action - Deallocate buffer action inside DPI
// Receive index and invalidates buffer
// --------------------------------------------------------------------
void ase_dealloc_action(struct buffer_t *buf)
{
  FUNC_CALL_ENTRY;

  char buf_str[ASE_MQ_MSGSIZE];
  memset (buf_str, 0, ASE_MQ_MSGSIZE);

  // Traversal pointer
  struct buffer_t *dealloc_ptr;
  dealloc_ptr = (struct buffer_t *) ase_malloc(sizeof(struct buffer_t));
  /* memset(dealloc_ptr, 0, sizeof(struct buffer_t)); */

  // Search buffer and Invalidate
  dealloc_ptr = ll_search_buffer(buf->index);

  //  If deallocate returns a NULL, dont get hosed
  if(dealloc_ptr != NULL)
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf("SIM-C : Command to invalidate \"%s\" ...\n", dealloc_ptr->memname);
      END_YELLOW_FONTCOLOR;

      // Mark buffer as invalid & deallocate
      dealloc_ptr->valid = ASE_BUFFER_INVALID; 
      munmap((void*)dealloc_ptr->vbase, (size_t)dealloc_ptr->memsize );
      shm_unlink(dealloc_ptr->memname);
      
      // Respond back
      dealloc_ptr->metadata = HDR_MEM_DEALLOC_REPLY;
      ll_remove_buffer(dealloc_ptr);
      memcpy(buf_str, dealloc_ptr, sizeof(struct buffer_t));
      mqueue_send(sim2app_dealloc_tx, buf_str, ASE_MQ_MSGSIZE);
    #ifdef ASE_LL_VIEW
      BEGIN_YELLOW_FONTCOLOR;
      ll_traverse_print();
      END_YELLOW_FONTCOLOR;
    #endif

      // Remove fd
      //close(dealloc_ptr->fd_ase);
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
  /* buf->fd_app = 0; */
  /* buf->fd_ase = 0; */
  buf->index = 0;
  buf->valid = ASE_BUFFER_INVALID;
  buf->metadata = 0;
  //  strcpy(buf->memname, "");
  memset(buf->memname, 0, ASE_SHM_NAME_LEN);
  buf->memsize = 0;
  buf->vbase = (uint64_t)NULL;
  buf->pbase = (uint64_t)NULL;
  buf->fake_paddr = (uint64_t)NULL;
  buf->next = NULL;
}


// --------------------------------------------------------------------
// ase_destroy : Destroy everything, called before exiting OR to
// reset simulation environment
//
// OPERATION:
// Traverse trough linked list
// - Remove each shared memory region
// - Remove each buffer_t 
// --------------------------------------------------------------------
void ase_destroy()
{
  FUNC_CALL_ENTRY;

#ifdef ASE_DEBUG
  char str[256];
  sprintf(str, "ASE destroy called");
  buffer_msg_inject(str);
#endif

  struct buffer_t *ptr;
  // ptr = (struct buffer_t *)ase_malloc(sizeof(struct buffer_t));

/* #ifdef ASE_DEBUG */
/*   ll_traverse_print(); */
/* #endif */

//  ptr = end;  
#if 0
  while((head != NULL)||(end != NULL))
    {
      ptr = end;
      if(ptr->valid == ASE_BUFFER_VALID)
	{
	  ase_dealloc_action(ptr);
	}
      else
	{
	  ll_remove_buffer(ptr);
	}
    } 
#else
  ptr = head;
  if (head != NULL)
    {
      while (ptr != (struct buffer_t*)NULL)
	{
	  ase_dealloc_action(ptr);
	  // ll_remove_buffer(ptr);
	  ptr = ptr->next;
	}
    }
#endif

/* #ifdef ASE_DEBUG */
/*   ll_traverse_print(); */
/* #endif */
  
  FUNC_CALL_EXIT;
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
  uint32_t zero_pbase_flag;
#ifdef ASE_DEBUG
  int tries = 0;
#endif

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

      // Zero base flag
      zero_pbase_flag = 0;
      if (ret_fake_paddr == 0)
	zero_pbase_flag = 1;

      // If all OK
      unique_physaddr_needed = search_flag | opposite_flag | zero_pbase_flag;
    #ifdef ASE_DEBUG
      tries++;
    #endif
    }
  
#ifdef ASE_DEBUG
  if (fp_memaccess_log != NULL)
    {
      fprintf(fp_memaccess_log, "  [DEBUG]  ASE took %d tries to generate phyaddr\n", tries);
    }
#endif

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

  // Traversal ptr
  struct buffer_t *trav_ptr = (struct buffer_t *)NULL;
  // int buffer_found = 0;

  if (req_paddr != 0)
    {
      // Clean up address of signed-ness
      req_paddr = req_paddr & 0x0000003FFFFFFFFF;

      // DPI pbase address
      uint64_t *ase_pbase;
 
      // This is the real offset to perform read/write
      uint64_t real_offset, calc_pbase;
  
      // For debug only
#ifdef ASE_DEBUG
      if (fp_memaccess_log != NULL) 
	{
	  fprintf(fp_memaccess_log, "req_paddr = %p | ", (void *)req_paddr);
	}
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
	      // buffer_found = 1;
	      
	      // Debug only
#ifdef ASE_DEBUG
	      if (fp_memaccess_log != NULL)
		{
		  fprintf(fp_memaccess_log, "offset=0x%016lx | pbase=%p\n", real_offset, (void *)ase_pbase);
		}
#endif
	      return ase_pbase;
	    }
	  else
	    {
	      trav_ptr = trav_ptr->next;
	    }
	}  
    }
  else 
    {
      // buffer_found = 0;
      trav_ptr = NULL;
    }

  // If accesses are correct, ASE should not reach this point
  if(trav_ptr == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("@ERROR: ASE has detected a memory operation to an unallocated memory region.\n"
	     "        Simulation cannot continue, please check the code.\n"
	     "        Failure @ phys_addr = %p \n"
	     "        See ERROR log file => ase_memory_error.log\n"
	     "@ERROR: Please check that previously requested memories have not been deallocated before an AFU transaction could access them\n"
	     "        NOTE: If your application polls for an AFU completion message, and you deallocate after that, consider using a WriteFence before AFU status message\n"
	     "              The simulator may be committing AFU transactions out of order\n",
	     (void*)req_paddr);
      END_RED_FONTCOLOR;

      // Write error to file
      error_fp = (FILE*) NULL;
      error_fp = fopen("ase_memory_error.log", "w");
      if (error_fp != NULL) 
	{
	  fprintf(error_fp, 
		  "*** ASE stopped on an illegal memory access ERROR ***\n"
		  "        AFU requested access @ physical memory %p\n"
		  "        Address not found in requested workspaces\n"
		  "        Timestamped transaction to this address is listed in ccip_transactions.tsv\n"
		  "        Check that previously requested memories have not been deallocated before an AFU transaction could access them"
		  "        NOTE: If your application polls for an AFU completion message, and you deallocate after that, consider using a WriteFence before AFU status message\n"
		  "              The simulator may be committing AFU transactions out of order\n",
		  (void*)req_paddr );
	  //fflush(error_fp);
	  fclose(error_fp);
	}

      // Request SIMKILL
      start_simkill_countdown();
    }

  return (uint64_t*)NOT_OK;

  FUNC_CALL_EXIT;
}

