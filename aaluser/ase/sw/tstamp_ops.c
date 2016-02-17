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
 * Module Info: Timestamp based session control functions
 * Language   : System{Verilog} | C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 */ 

#include "ase_common.h"


// -----------------------------------------------------------------------
// Timestamp based isolation
// -----------------------------------------------------------------------
#if defined(__i386__)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;
}
#elif defined(__x86_64__)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#else
#error "Host Architecture unidentified, timestamp wont work"
#endif


// -----------------------------------------------------------------------
// Write timestamp
// -----------------------------------------------------------------------
void put_timestamp()
{
  FUNC_CALL_ENTRY;

  FILE *fp;
  char tstamp_path[ASE_FILEPATH_LEN];


  memset(tstamp_path, 0, ASE_FILEPATH_LEN);
  sprintf(tstamp_path, "%s/%s", ase_workdir_path, TSTAMP_FILENAME);

  fp = fopen(tstamp_path, "wb");
  if (fp == NULL) 
    {
#ifdef SIM_SIDE
      ase_error_report("fopen", errno, ASE_OS_FOPEN_ERR);
#else
      perror("fopen");
#endif
      exit(1);
    }
  fprintf(fp, "%lld", (unsigned long long)rdtsc() );

  fclose(fp);

  FUNC_CALL_EXIT;
}


// -----------------------------------------------------------------------
// Read timestamp 
// -----------------------------------------------------------------------
char* get_timestamp(int dont_kill)
{
  FUNC_CALL_ENTRY;

  FILE *fp;

  unsigned long long readback;

  char *tstamp_str;
  tstamp_str = ase_malloc(20);
  if (tstamp_str == NULL)
    {
      ase_error_report("malloc", errno, ASE_OS_MALLOC_ERR);
    #ifdef SIM_SIDE
      start_simkill_countdown();
    #else
      exit(1);
    #endif		       
    }

  char *tstamp_filepath;
  tstamp_filepath = (char*)ase_malloc(ASE_FILEPATH_LEN);

  // Generate tstamp_filepath
  memset(tstamp_filepath, 0, ASE_FILEPATH_LEN);
  sprintf(tstamp_filepath, "%s/%s", ase_workdir_path, TSTAMP_FILENAME);

#ifdef ASE_DEBUG
  printf("  [DEBUG] tstamp_filepath = %s\n", tstamp_filepath);
#endif

  fp = fopen(tstamp_filepath, "r");
  if (dont_kill) 
    {
      BEGIN_YELLOW_FONTCOLOR;
      printf(" Timestamp gone ! .. "); 
      END_YELLOW_FONTCOLOR;
    }
  else
    {
      if (fp == NULL) 
	{
        #ifdef SIM_SIDE
	  ase_error_report("fopen", errno, ASE_OS_FOPEN_ERR);
        #else
	  perror("fopen");
        #endif
	  exit(1);
	}
    }
  
  fread(&readback, sizeof(unsigned long long), 1, fp);
  fclose(fp);
  
  sprintf(tstamp_str, "%lld", readback);

#ifdef ASE_DEBUG
  printf("  [DEBUG] tstamp_str = %s\n", tstamp_str);
#endif

  FUNC_CALL_EXIT;

  return tstamp_str;
}

// -----------------------------------------------------------------------
// Check for session file to be created
// Used by session_init() to wait until .ase_timestamp existance is confirm
// -----------------------------------------------------------------------
void poll_for_session_id()
{
  char tstamp_filepath[ASE_FILEPATH_LEN];
  sprintf(tstamp_filepath, "%s/%s", ase_workdir_path, TSTAMP_FILENAME);

  printf("SIM-C : Waiting till session ID is created by ASE ... ");

  while ( access(tstamp_filepath, F_OK) == -1 )
    {
      usleep(1000);
    }
  printf("DONE\n");
}

