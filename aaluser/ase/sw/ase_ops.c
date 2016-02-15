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
 * Module Info: ASE operations functions
 * Language   : C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 * 
 */

#include "ase_common.h"

struct buffer_t *head;
struct buffer_t *end;

// -----------------------------------------------------------
// ase_dump_to_file : Dumps a shared memory region into a file
// Dump contents of shared memory to a file
// -----------------------------------------------------------
int ase_dump_to_file(struct buffer_t *mem, char *dump_file)
{
  FILE *fileptr;
  uint32_t *memptr;

  // Open file
  fileptr = fopen(dump_file,"wb");
  if(fileptr == NULL)
    {
#ifdef SIM_SIDE
      ase_error_report ("fopen", errno, ASE_OS_FOPEN_ERR);
#else
      perror("fopen");
#endif
      return NOT_OK;
    }

  // Start dumping
  for(memptr=(uint32_t*)mem->vbase; memptr < (uint32_t*)(mem->vbase + mem->memsize); (uint32_t*)memptr++)
    fprintf(fileptr,"%08x : 0x%08x\n", (uint32_t)((uint64_t)memptr-(uint64_t)mem->vbase), *memptr);

  // Close file
  fclose(fileptr);
  return OK;
}


// -------------------------------------------------------------
// ase_buffer_info : Print out information about the buffer
// -------------------------------------------------------------
void ase_buffer_info(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;  
  
  BEGIN_YELLOW_FONTCOLOR;
  printf("BUFFER parameters...\n");
  printf("\tfd_app      = %d \n",    mem->fd_app);
  printf("\tfd_ase      = %d \n",    mem->fd_ase);
  printf("\tindex       = %d \n",    mem->index);
  printf("\tvalid       = %s \n",    (mem->valid == 0xffff) ? "VALID" : "INVALID" );
  printf("\tAPPVirtBase = %p \n",    (void *)mem->vbase); 
  printf("\tSIMVirtBase = %p \n",    (void *)mem->pbase); 
  printf("\tBufferSize  = %x \n",    mem->memsize);  
  printf("\tBufferName  = \"%s\"\n", mem->memname);  
  printf("\tPhysAddr LO = %p\n",     (void *)mem->fake_paddr); 
  printf("\tPhysAddr HI = %p\n",     (void *)mem->fake_paddr_hi);
  printf("\tisMMIOMap   = %s\n",     (mem->is_mmiomap == 1) ? "YES" : "NO");
  printf("\tisUMAS      = %s\n",     (mem->is_umas == 1) ? "YES" : "NO");
  END_YELLOW_FONTCOLOR;

  FUNC_CALL_EXIT;
}


/* 
 * ase_buffer_oneline : Print one line info about buffer
 */
void ase_buffer_oneline(struct buffer_t *mem)
{
  BEGIN_YELLOW_FONTCOLOR;

  printf("%d  ", mem->index);
  if (mem->valid == ASE_BUFFER_VALID) 
    printf("ADDED   ");
  else
    printf("REMOVED ");
  printf("%5s \t", mem->memname);
  printf("\n");

  END_YELLOW_FONTCOLOR;
}


// -------------------------------------------------------------------
// buffer_t_to_str : buffer_t to string conversion
// Converts buffer_t to string 
// -------------------------------------------------------------------
void ase_buffer_t_to_str(struct buffer_t *buf, char *str)
{
  FUNC_CALL_ENTRY;

  memcpy(str, (char*)buf, sizeof(struct buffer_t));

  FUNC_CALL_EXIT;
}


// --------------------------------------------------------------
// ase_str_to_buffer_t : string to buffer_t conversion
// All fields are space separated, use strtok to decode
// --------------------------------------------------------------
void ase_str_to_buffer_t(char *str, struct buffer_t *buf)
{
  FUNC_CALL_ENTRY;
  
  memcpy((char*)buf, str, sizeof(struct buffer_t));

  FUNC_CALL_EXIT;
}


/*
 * ASE memory barrier
 */
void ase_memory_barrier()
{
  // asm volatile("" ::: "memory");
  __asm__ __volatile__ ("" ::: "memory");
}


/*
 * Evaluate Session directory
 * If SIM_SIDE is set, Return "$ASE_WORKDIR/work/"
 *               else, Return "$PWD/work/"
 *               Both must be the same location
 *
 * PROCEDURE:
 * - Check if PWD/ASE_WORKDIR exists:
 *   - Most cases, it will exist, created by Makefile
 *     - Check if "work" directory already exists, if not create one
 *   - If not Error out
 */
char* ase_eval_session_directory()
{
  FUNC_CALL_ENTRY;
  
  char *workdir_path;
  char *env_path;
    
  workdir_path = ase_malloc (ASE_FILEPATH_LEN);

  // Evaluate basename location
#ifdef SIM_SIDE
  env_path = getenv ("PWD");
#else
  env_path = getenv ("ASE_WORKDIR");
#endif
      
  // Locate work directory
  if( env_path) 
    {
      // strcat( workdir_path, env_path );
      memcpy(workdir_path, env_path, ASE_FILEPATH_LEN);
    } 
  /* else  */
  /*   { */
  /*     *workdir_path = '\0'; */
  /*   } */

  // strcat( workdir_path, "/work/" );  || RRS:

  // *FIXME*: Idiot-proof the work directory

  FUNC_CALL_EXIT;
  
  return workdir_path;
}


/*
 * ASE malloc 
 * Malloc wrapped with ASE closedown if failure accures
 */
char* ase_malloc (size_t size)
{
  FUNC_CALL_ENTRY;

  char *buffer;

  buffer = malloc (size);
  // posix_memalign((void**)&buffer, (size_t)getpagesize(), size);
  if (buffer == NULL)
    {
      ase_error_report ("malloc", errno, ASE_OS_MALLOC_ERR);
    #ifdef SIM_SIDE
      printf("SIM-C : Malloc failed\n");
      start_simkill_countdown();
    #else
      printf("  [APP] Malloc failed\n");
      exit(1);
    #endif
    }   
  else
    {
      memset (buffer, 0, size);
    }

  FUNC_CALL_EXIT;
  return buffer;
}


