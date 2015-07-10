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

uint64_t csr_fake_pin;
struct timeval start;

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
  printf("Shared BUFFER parameters...\n");
  printf("\tfd_app      = %d \n",    mem->fd_app);
  printf("\tfd_ase      = %d \n",    mem->fd_ase);
  printf("\tindex       = %d \n",    mem->index);
  printf("\tvalid       = %x \n",    mem->valid);
  printf("\tAPPVirtBase = %p \n",    (uint32_t*)mem->vbase); 
  printf("\tSIMVirtBase = %p \n",    (uint32_t*)mem->pbase); 
  printf("\tBufferSize  = %x \n",    mem->memsize);  
  printf("\tBufferName  = \"%s\"\n", mem->memname);  
  printf("\tPhysAddr LO = %p\n", (uint32_t*)mem->fake_paddr); 
  printf("\tPhysAddr HI = %p\n", (uint32_t*)mem->fake_paddr_hi);
  /* printf("\tIsDSM       = %d\n", mem->is_csrmap);  */
  /* printf("\tIsPrivMem   = %d\n", mem->is_privmem);  */
  BEGIN_YELLOW_FONTCOLOR;

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
  /* printf("%p  ", (uint32_t*)mem->vbase); */
  /* printf("%p  ", (uint32_t*)mem->pbase); */
  /* printf("%p (%08x) ", (uint32_t*)mem->fake_paddr, (uint32_t)(mem->fake_paddr >> 6) ); */
  /* printf("%x  ", mem->memsize); */
  /* printf("%d  ", mem->is_csrmap); */
  /* printf("%d  ", mem->is_privmem); */
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

  // Initialise string to nulls
  memset(str, '\0', ASE_MQ_MSGSIZE);// strlen(str));

  if(buf->metadata == HDR_MEM_ALLOC_REQ)
    {
      // Form an allocate message request
      sprintf(str, "%d %d %s %d %ld %d %ld", buf->metadata, buf->fd_app, buf->memname, buf->valid, (long int)buf->memsize, buf->index, (long int)buf->vbase);
    }
  else if (buf->metadata == HDR_MEM_ALLOC_REPLY)
    {
      // Form an allocate message reply
      sprintf(str, "%d %d %ld %ld %ld", buf->metadata, buf->fd_ase, buf->pbase, buf->fake_paddr, buf->fake_paddr_hi);
    }
  else if (buf->metadata == HDR_MEM_DEALLOC_REQ)
    {
      // Form a deallocate request
      sprintf(str, "%d %d %s", buf->metadata, buf->index, buf->memname);
    }

  FUNC_CALL_EXIT;
}


// --------------------------------------------------------------
// ase_str_to_buffer_t : string to buffer_t conversion
// All fields are space separated, use strtok to decode
// --------------------------------------------------------------
void ase_str_to_buffer_t(char *str, struct buffer_t *buf)
{
  FUNC_CALL_ENTRY;

  char *pch;
  
  pch = strtok(str, " ");
  buf->metadata = atoi(pch);
  if(buf->metadata == HDR_MEM_ALLOC_REQ)
    {
      // Tokenize remaining fields of ALLOC_MSG
      pch = strtok(NULL, " ");
      buf->fd_app = atoi(pch);     // APP-side file descriptor
      pch = strtok(NULL, " ");
      strcpy(buf->memname, pch);   // Memory name
      pch = strtok(NULL, " ");
      buf->valid = atoi(pch);      // Indicates buffer is valid
      pch = strtok(NULL, " ");
      buf->memsize = atoi(pch);    // Memory size
      pch = strtok(NULL, " ");
      buf->index = atoi(pch);      // Buffer ID
      pch = strtok(NULL, " ");
      buf->vbase = atol(pch);      // APP-side virtual base
    }
  else if(buf->metadata == HDR_MEM_ALLOC_REPLY)
    {
      // Tokenize remaining 2 field of ALLOC_REPLY
      pch = strtok(NULL, " "); 
      buf->fd_ase = atoi(pch);     // DPI-side file descriptor
      pch = strtok(NULL, " "); 
      buf->pbase = atol(pch);      // DPI sude virtual address
      pch = strtok(NULL, " ");  
      buf->fake_paddr = atol(pch); // Fake physical address
      pch = strtok(NULL, " ");  
      buf->fake_paddr_hi = atol(pch); // Fake high point in offsets
    }
  else if(buf->metadata == HDR_MEM_DEALLOC_REQ)
    {
      pch = strtok(NULL, " ");
      buf->index = atoi(pch);      // Index
      pch = strtok(NULL, " ");
      strcpy(buf->memname, pch);   // Memory name
    }

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
  /* struct stat s; */
  /* int err; */
    
  workdir_path = malloc (ASE_FILEPATH_LEN);
  if (!workdir_path) return NULL;

  // Evaluate basename location
#ifdef SIM_SIDE
  env_path = getenv ("PWD");
#else
  env_path = getenv ("ASE_WORKDIR");
#endif
      
  // Locate work directory
  if( env_path) {
     strcat( workdir_path, env_path );
  } else {
     *workdir_path = '\0';
  }
  strcat( workdir_path, "/work/" ); 

  // *FIXME*: Idiot-proof the work directory

  FUNC_CALL_EXIT;
  
  return workdir_path;
}
//char* ase_eval_session_directory()
//{
//  FUNC_CALL_ENTRY;
//
//  char *workdir_path;
//  /* struct stat s; */
//  /* int err; */
//
//  workdir_path = malloc (ASE_FILEPATH_LEN);
//  // Evaluate basename location
//#ifdef SIM_SIDE
//  workdir_path = getenv ("PWD");
//#else
//  workdir_path = getenv ("ASE_WORKDIR");
//#endif
//
//  // Locate work directory
//  strcat( workdir_path, "/work/" );
//
//  // *FIXME*: Idiot-proof the work directory
//
//  FUNC_CALL_EXIT;
//
//  return workdir_path;
//}

