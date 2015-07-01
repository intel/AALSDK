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
 * Module Info: Private memory access functions - C Module (no main() here)
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 * 
 * Revisions
 * RRS         3rd Mar 2014      - Private memory subsystem, not connected to
 *                                 linked list, private buffer only
 *
 */

#include "ase_common.h"


/*
 * capcm_init : Initialize a private memory buffer, not accessible
 *              to software application
 * CA-PCM is implemented as a region in /dev/shm
 * This will also allow dumping to file if needed
 *
 */
void capcm_init()
{
  FUNC_CALL_ENTRY;

  int mem_i;
  int ret;
  char gen_memname[ASE_SHM_NAME_LEN];

  // Zero check
  if (capcm_size == 0)
    {
      printf("SIM-C : CAPCM size requested was zero bytes - NO memory allocated\n");
    }
  else
    {
      // Calculate number of chunks required for implementing CAPCM
      capcm_num_buffers = capcm_size/CAPCM_CHUNKSIZE;
      printf("SIM-C : CAPCM will use %lu chunks of 0x%lx bytes each \n", capcm_num_buffers, CAPCM_CHUNKSIZE );
      
      // Malloc control structures
      capcm_buf = (struct buffer_t *)malloc(capcm_num_buffers * sizeof(struct buffer_t));
      if (capcm_buf == NULL) 
	{
	  printf("SIM-C : CAPCM control structures could not be allocated... EXITING\n");
	  ase_error_report("malloc", errno, ASE_OS_MALLOC_ERR);
	  start_simkill_countdown();
	}

      // Start adding chain of CAPCMs to buffer chain
      for(mem_i = 0; mem_i < capcm_num_buffers; mem_i++)
	{
	  // Set memory name & open file descriptor
	  ret = sprintf( gen_memname, "%s%d.%s", CAPCM_BASENAME, mem_i, get_timestamp(0) );
	  if (ret < 0)
	    {
	      ase_error_report("sprintf", errno, ASE_OS_STRING_ERR);
	      start_simkill_countdown();
	    }
	  strcpy(capcm_buf[mem_i].memname, gen_memname); 
	  capcm_buf[mem_i].index = 1000000 + mem_i;
	  add_to_ipc_list ("SHM", capcm_buf[mem_i].memname);	  
	  
	  // Map memory name, calculate memory high, ftruncate to given size
	  capcm_buf[mem_i].fd_ase = shm_open(capcm_buf[mem_i].memname, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	  if (capcm_buf[mem_i].fd_ase < 0)
	    {
	      ase_error_report("shm_open", errno, ASE_OS_SHM_ERR);
	      ase_perror_teardown();
	      start_simkill_countdown();
	    }

	  capcm_buf[mem_i].fd_app = 0;

	  // Map to virtual memory system	  
	  capcm_buf[mem_i].vbase = 0;
	  capcm_buf[mem_i].pbase = (uint64_t)  mmap(NULL,
						     CAPCM_CHUNKSIZE,
						     PROT_READ|PROT_WRITE,
						     MAP_PRIVATE,
						     capcm_buf[mem_i].fd_ase,
						     0);
	  if(capcm_buf[mem_i].pbase == (uint64_t) MAP_FAILED)
	    {
	      ase_error_report("mmap", errno, ASE_OS_MEMMAP_ERR);
	      ase_perror_teardown();
	      start_simkill_countdown();
	    }

	  // Extend the map
	  ret = ftruncate(capcm_buf[mem_i].fd_ase, (off_t) CAPCM_CHUNKSIZE);
	  if(0 != ret)
	    {
	      ase_error_report("ftruncate", errno, ASE_OS_MEMMAP_ERR);
	      ase_perror_teardown();
	      start_simkill_countdown();
	    }


	  // Set physical memsize, LO and HI
	  capcm_buf[mem_i].memsize = CAPCM_CHUNKSIZE;
	  capcm_buf[mem_i].fake_paddr = capcm_phys_lo + mem_i * CAPCM_CHUNKSIZE;
	  capcm_buf[mem_i].fake_paddr_hi = capcm_phys_lo + CAPCM_CHUNKSIZE - 1;

	  // Set buffer flags
	  capcm_buf[mem_i].valid = ASE_BUFFER_VALID;
	  capcm_buf[mem_i].is_privmem = 1;
	  capcm_buf[mem_i].is_csrmap = 0;
	    
	  // Append buffer to ASE-control
	  ll_append_buffer( &capcm_buf[mem_i] );
	}    
      
      // Print notice
      printf("SIM-C : CAPCM buffer space has been allocated\n");  
      ll_traverse_print();      
    }

  FUNC_CALL_EXIT;
}


