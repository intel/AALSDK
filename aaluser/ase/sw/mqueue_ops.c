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
 * Module Info: Message queue functions
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 */

#include "ase_common.h"

// Message queue attribute (optional use)
// struct mq_attr attr;

/*
 * ipc_init: Initialize IPC messaging structure
 *           DOES not create or open the IPC, simply initializes the structures
 */
void ipc_init()
{
  FUNC_CALL_ENTRY;

  int ipc_iter;

  strcpy(mq_array[0].name, "app2sim_bufping_smq");
  strcpy(mq_array[1].name, "app2sim_mmioreq_smq");
  strcpy(mq_array[2].name, "app2sim_umsg_smq");
  strcpy(mq_array[3].name, "app2sim_simkill_smq");
  strcpy(mq_array[4].name, "sim2app_bufpong_smq");
  strcpy(mq_array[5].name, "sim2app_mmiorsp_smq");
  
  // Calculate path 
  for(ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    sprintf(mq_array[ipc_iter].path, "%s/%s", ase_workdir_path, mq_array[ipc_iter].name);

#ifdef SIM_SIDE
  mq_array[0].perm_flag = O_RDONLY|O_NONBLOCK;
  mq_array[1].perm_flag = O_RDONLY|O_NONBLOCK;
  mq_array[2].perm_flag = O_RDONLY|O_NONBLOCK;
  mq_array[3].perm_flag = O_RDONLY|O_NONBLOCK;
  mq_array[4].perm_flag = O_WRONLY;
  mq_array[5].perm_flag = O_WRONLY;
#else
  mq_array[0].perm_flag = O_WRONLY;
  mq_array[1].perm_flag = O_WRONLY;
  mq_array[2].perm_flag = O_WRONLY;
  mq_array[3].perm_flag = O_WRONLY;
  mq_array[4].perm_flag = O_RDONLY;  
  mq_array[5].perm_flag = O_RDONLY;  
#endif 

  // Remove IPCs if already there
#ifdef SIM_SIDE
  for(ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    unlink(mq_array[ipc_iter].path);
#endif

  FUNC_CALL_EXIT;
}


/*
 * mqueue_create: Create a simplex mesaage queue by passing a name
 */
void mqueue_create(char* mq_name_suffix)
{
  FUNC_CALL_ENTRY;

  char *mq_path;
  int ret;
  
  mq_path = ase_malloc (ASE_FILEPATH_LEN);
  sprintf(mq_path, "%s/%s", ase_workdir_path, mq_name_suffix);

/* #ifdef ASE_DEBUG */
/*   printf("mq_path = %s\n", mq_path); */
/* #endif */

  // ret = mkfifo(mq_path, 0666);
  ret = mkfifo(mq_path, S_IRUSR|S_IWUSR );
  if (ret == -1)
    {
      BEGIN_RED_FONTCOLOR;
      printf("Error creating IPC\n");
      END_RED_FONTCOLOR;
    }

  // Add IPC to list
#ifdef SIM_SIDE
  add_to_ipc_list("MQ", mq_path);
  fflush(local_ipc_fp);
#endif

  FUNC_CALL_EXIT;
}


/*
 * mqueue_open : Open IPC messaging device
 *
 * NOTES:
 * - Named pipes require reader to be ready for non-blocking open to
 *   proceed. This may not be possible in an ASE environment.       
 * - This may be solved by having a dummy reader the WRONLY fifo, then
 *   close it after ASE's real fd is created successfully. 
 *
 */
int mqueue_open(char *mq_name, int perm_flag)
{
  FUNC_CALL_ENTRY;

  int mq;
  char *mq_path;

  mq_path = ase_malloc (ASE_FILEPATH_LEN);
  memset (mq_path, '\0', ASE_FILEPATH_LEN);
  sprintf(mq_path, "%s/%s", ase_workdir_path, mq_name);
  
/* #ifdef ASE_DEBUG */
/*   printf("mq_path = %s\n", mq_path); */
/* #endif */

  // Dummy function to open WRITE only MQs
  // Named pipe requires non-blocking write-only move on from here
  // only when reader is ready.
#ifdef SIM_SIDE
  int dummy_fd;
  if (perm_flag == O_WRONLY)
    {
      // printf("Opening IPC in write-only mode with dummy fd\n");
      dummy_fd = open(mq_path, O_RDONLY|O_NONBLOCK);
    }
#endif

  mq = open(mq_path, perm_flag);
  if (mq == -1) 
    {
      printf("Error opening IPC\n");
#ifdef SIM_SIDE
      ase_error_report("open", errno, ASE_OS_FOPEN_ERR);
      start_simkill_countdown();
#else
      perror("open");
      exit(1);
#endif
    }
  
#ifdef SIM_SIDE
  if (perm_flag == O_WRONLY)
    {
      close(dummy_fd);
    }
#endif

  FUNC_CALL_EXIT;
  
  return mq;
}


// -------------------------------------------------------
// mqueue_close(int): close MQ by descriptor
// -------------------------------------------------------
void mqueue_close(int mq)
{
  FUNC_CALL_ENTRY;

  int ret;
  ret = close (mq);
  if (ret == -1) 
    {
#ifdef SIM_SIDE
 #ifdef ASE_DEBUG
      printf("Error closing IPC\n");
 #endif
#endif
    }

  FUNC_CALL_EXIT;
}


// -----------------------------------------------------------
// mqueue_destroy(): Unlink message queue, must be called from API
// -----------------------------------------------------------
void mqueue_destroy(char* mq_name_suffix)
{
  FUNC_CALL_ENTRY;

  char *mq_path;
  int ret;

  mq_path = ase_malloc (ASE_FILEPATH_LEN);
  if (mq_path != NULL) 
    {
      memset (mq_path, '\0', ASE_FILEPATH_LEN);
      sprintf(mq_path, "%s/%s", ase_workdir_path, mq_name_suffix); 
      ret = unlink ( mq_path );
      if (ret == -1) 
	{
	  printf("Message queue %s could not be removed, please remove manually\n", mq_name_suffix);
	}
    }
  else
    {
      printf("Could not remove MQ, please clean by removing work directory\n");
    }
  
  FUNC_CALL_EXIT;
}


// ------------------------------------------------------------
// mqueue_send(): Easy send function
// - Typecast any message as a character array and ram it in.
// ------------------------------------------------------------
void mqueue_send(int mq, char* str)
{
  FUNC_CALL_ENTRY;
  
  write(mq, str, ASE_MQ_MSGSIZE);  
#ifdef ASE_MSG_VIEW
  printf("ASEmsg TX => %s\n", str);
#endif
  
  FUNC_CALL_EXIT;
}


// ------------------------------------------------------------------
// mqueue_recv(): Easy receive function
// - Typecast message back to a required type
// ------------------------------------------------------------------

int mqueue_recv(int mq, char* str)
{
  FUNC_CALL_ENTRY;
  
  int ret;

  ret = read(mq, str, ASE_MQ_MSGSIZE);
  FUNC_CALL_EXIT;
  if (ret > 0)
    {
       return ASE_MSG_PRESENT;       
    }
  else
    {
      return ASE_MSG_ABSENT;
    }   
}
