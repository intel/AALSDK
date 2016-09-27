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


// -----------------------------------------------------------
// ase_dump_to_file : Dumps a shared memory region into a file
// Dump contents of shared memory to a file
// -----------------------------------------------------------
#if 0
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
#endif

// -------------------------------------------------------------
// ase_buffer_info : Print out information about the buffer
// -------------------------------------------------------------
void ase_buffer_info(struct buffer_t *mem)
{
  FUNC_CALL_ENTRY;

  BEGIN_YELLOW_FONTCOLOR;
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
    printf("\tADDED   ");
  else
    printf("\tREMOVED ");
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
void ase_eval_session_directory()
{
  FUNC_CALL_ENTRY;

  ase_workdir_path = ase_malloc (ASE_FILEPATH_LEN);

  // Evaluate location of simulator or own location
#ifdef SIM_SIDE
  ase_workdir_path = getenv ("PWD");
#else
  ase_workdir_path = getenv ("ASE_WORKDIR");

#ifdef ASE_DEBUG
  BEGIN_YELLOW_FONTCOLOR;
  printf("  [DEBUG]  env(ASE_WORKDIR) = %s\n", ase_workdir_path);
  END_YELLOW_FONTCOLOR;
#endif

  if (ase_workdir_path == NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("  [APP]  **ERROR** Environment variable ASE_WORKDIR could not be evaluated !!\n");
      printf("         **ERROR** ASE will exit now !!\n");
      END_RED_FONTCOLOR;
      perror("getenv");
      exit(1);
    }
  else
    {
      // Check if directory exists here
      DIR* ase_dir;
      ase_dir = opendir(ase_workdir_path);
      if (!ase_dir)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("  [APP]  ASE workdir path pointed by env(ASE_WORKDIR) does not exist !\n");
	  printf("         Cannot continue execution... exiting !");
	  END_RED_FONTCOLOR;
	  perror("opendir");
	  exit(1);
	}
      else
	{
	  closedir(ase_dir);
	}
    }
#endif
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


/*
 * ase_write_lock_file : Write ASE Lock file
 * To be used only by simulator, NOT application
 *
 * Writes a lock file about session specific items like this
 * ------------------------------
 * | pid = <pid>
 * | host = <hostname>
 * | dir = <$PWD>
 * | uid = <ASE Unique ID>
 * ------------------------------
 *
 */
#ifdef SIM_SIDE
void ase_write_lock_file()
{
  FUNC_CALL_ENTRY;

  int ret_err;

  // Create a filepath string
  ase_ready_filepath = ase_malloc(ASE_FILEPATH_LEN);
  snprintf(ase_ready_filepath, ASE_FILEPATH_LEN, "%s/%s", ase_workdir_path, ASE_READY_FILENAME);

  // Line 2
  ase_hostname = ase_malloc(ASE_FILENAME_LEN);

  // Open file
  fp_ase_ready = fopen(ase_ready_filepath, "w");
  if (fp_ase_ready == (FILE*)NULL)
    {
      BEGIN_RED_FONTCOLOR;
      printf("SIM-C : **ERROR** => ASE lock file could not be written, Exiting\n");
      END_RED_FONTCOLOR;
      start_simkill_countdown();
    }
  else
    {
      // ///////// Write specifics ////////////////
      // Line 1
      fprintf(fp_ase_ready, "pid  = %d\n", ase_pid);

      // Get hostname for comparison
      ret_err = gethostname(ase_hostname, ASE_FILENAME_LEN);
      if (ret_err != 0)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : **ERROR** => Hostname could not be calculated, Exiting\n");
	  END_RED_FONTCOLOR;

	  // Close file
	  fclose(fp_ase_ready);

	  // Remove buffers
	  free (ase_hostname);

	  // Issue Simkill
	  start_simkill_countdown();
	}
      else
	{
      fprintf(fp_ase_ready, "host = %s\n", ase_hostname);

      // Line 3
      fprintf(fp_ase_ready, "dir  = %s\n", ase_workdir_path);

      // Line 4
      fprintf(fp_ase_ready, "uid  = %s\n", ASE_UNIQUE_ID);

      ////////////////////////////////////////////
      // Close file
      fclose(fp_ase_ready);
      free (ase_hostname);

      // Notice on stdout
      printf("SIM-C : ASE lock file .ase_ready.pid written in work directory\n");
	}
    }


  FUNC_CALL_EXIT;
}
#endif


/*
 * ase_read_lock_file() : Read an existing lock file and decipher contents
 */
int ase_read_lock_file(const char *workdir)
{
  // Allocate string
  FILE *fp_exp_ready;
  char *exp_ready_filepath;
  char *line;
  size_t len;

  char *parameter;
  char *value;

  char *readback_workdir_path;
  char *readback_hostname;
  char *curr_hostname;
  char *readback_uid;
  char *curr_uid;
  int readback_pid = 0;
  int ret_err;

  // Null check and exit
  if (workdir == NULL)
    {
      BEGIN_RED_FONTCOLOR;
#ifdef SIM_SIDE
      printf("SIM-C : ");
#else
      printf("  [APP]  ");
#endif
      printf("ase_read_lock_file : Input ASE workdir path is NULL \n");
      END_RED_FONTCOLOR;
#ifdef SIM_SIDE
      start_simkill_countdown();
#else
      exit(1);
#endif
    }
  else
    {
      // Calculate ready file path
      exp_ready_filepath = ase_malloc(ASE_FILEPATH_LEN);
      snprintf(exp_ready_filepath, ASE_FILEPATH_LEN, "%s/%s", workdir, ASE_READY_FILENAME);

      // Check if file exists
      if (access(exp_ready_filepath, F_OK) != -1)  // File exists
	{
	  // Open file
	  fp_exp_ready = fopen(exp_ready_filepath, "r");
	  if (fp_exp_ready == NULL)
	    {
	      BEGIN_RED_FONTCOLOR;
#ifdef SIM_SIDE
	      printf("SIM-C : ");
#else
	      printf("  [APP]  ");
#endif
	      printf("Ready file couldn't be opened for reading, Exiting !\n");
	      END_RED_FONTCOLOR;
#ifdef SIM_SIDE
	      start_simkill_countdown();
#else
	      exit(1);
#endif
	    }
	  else
	  {
	  // Malloc/memset
	  line = ase_malloc(256);
	  readback_hostname = ase_malloc(ASE_FILENAME_LEN);
	  curr_hostname = ase_malloc(ASE_FILENAME_LEN);
	  readback_workdir_path = ase_malloc(ASE_FILEPATH_LEN);
	  readback_uid = ase_malloc(ASE_FILEPATH_LEN);

	  // Read file line by line
	  while( getline(&line, &len, fp_exp_ready) != -1)
	    {
	      // LHS/RHS tokenizing
	      parameter = strtok(line, "=");
	      value = strtok(NULL, "");
	      // Check for parameter being recorded as NULL
	      if ((parameter == NULL) || (value == NULL))
		{
		  BEGIN_RED_FONTCOLOR;
		  printf("** Error tokenizing paramter in lock file, EXIT !\n");
		  END_RED_FONTCOLOR;
#ifdef SIM_SIDE
		  start_simkill_countdown();
#else
		  exit(1);
#endif
		}
	      else
		{
		  // Trim contents
		  remove_spaces (parameter);
		  remove_tabs (parameter);
		  remove_newline (parameter);
		  remove_spaces (value);
		  remove_tabs (value);
		  remove_newline(value);
		  // Line 1/2/3/4 check
		  if ( strncmp (parameter, "pid", 4) == 0)
		    {
		      readback_pid = atoi(value);
		    }
		  else if ( strncmp (parameter, "host", 5) == 0)
		    {
		      // strncpy(readback_hostname, value, ASE_FILENAME_LEN);
		      ase_string_copy(readback_hostname, value, ASE_FILENAME_LEN);
		    }
		  else if ( strncmp (parameter, "dir", 4) == 0)
		    {
		      // strncpy(readback_workdir_path, value, ASE_FILEPATH_LEN);
		      ase_string_copy(readback_workdir_path, value, ASE_FILEPATH_LEN);
		    }
		  else if ( strncmp (parameter, "uid", 4) == 0)
		    {
		      // strncpy(readback_uid, value, ASE_FILEPATH_LEN);
		      ase_string_copy(readback_uid, value, ASE_FILEPATH_LEN);
		    }
		  else
		    {
		      BEGIN_RED_FONTCOLOR;
		      printf("** ERROR **: Session parameter could not be deciphered !\n");
		      END_RED_FONTCOLOR;
		    }
		}
	    }
	  fclose(fp_exp_ready);
	  }
	  ////////////////// Error checks //////////////////
	  // If hostname does not match
	  ret_err = gethostname(curr_hostname, ASE_FILENAME_LEN);
	  if (ret_err != 0)
	    {
	      BEGIN_RED_FONTCOLOR;
	      printf("**ERROR** => Hostname could not be calculated, Exiting\n");
	      END_RED_FONTCOLOR;
	      exit(1);
	    }
	  else
	    {
	      // Check here
	      if (strncmp(curr_hostname, readback_hostname, ASE_FILENAME_LEN) != 0)
		{
		  BEGIN_RED_FONTCOLOR;
		  printf("** ERROR ** => Hostname specified in ASE lock file (%s) is different as current hostname (%s)\n", readback_hostname, curr_hostname);
		  printf("** ERROR ** => Ensure that ASE Simulator and AAL application are running on the same host !\n");
		  END_RED_FONTCOLOR;
#ifdef SIM_SIDE
		  start_simkill_countdown();
#else
		  exit(1);
#endif
		}
	      else
		{
		  // If readback_uid (Readback unique ID from lock file) doesnt match ase_common.h
		  curr_uid = ase_malloc(ASE_FILENAME_LEN);
		  strncpy(curr_uid, ASE_UNIQUE_ID, ASE_FILENAME_LEN);

		  // Check
		  if (strncmp(curr_uid, readback_uid, ASE_FILENAME_LEN) != 0)
		    {
		      BEGIN_RED_FONTCOLOR;
		      printf("** ERROR ** => Application UID does not match known release UID\n");
		      printf("** ERROR ** => Simulator built with UID=%s, Application built with UID=%s\n", readback_uid, curr_uid );
		      printf("** ERROR ** => Ensure that ASE simulator and AAL application are compiled from the same System Release version !\n");
		      printf("** ERROR ** => Simulation cannot proceed ... EXITING\n");
		      END_RED_FONTCOLOR;
#ifdef SIM_SIDE
		      start_simkill_countdown();
#else
		      exit(1);
#endif
		    }

		  // Free curr_uid
		  free (curr_uid);
		}
	    }

	  // Free all buffers
	  free (line);
	  free (readback_hostname);
	  free (curr_hostname);
	  free (readback_workdir_path);
	  free (readback_uid);
	}
      else // File does not exist
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("ASE Ready file was not found at env(ASE_WORKDIR) !\n");
	  printf("This could be for one of two reasons =>\n");
	  printf(" - Simualtor is not running yet  \n");
	  printf(" - env(ASE_WORKDIR) is set to the wrong location \n");
	  END_RED_FONTCOLOR;
	}

      // Free expected filepath buffer
      free (exp_ready_filepath);
    }

  // Return PID of Simulator instance
  return readback_pid;
}


/*
 * Pretty print function - print_mmiopkt
 */
void print_mmiopkt(FILE *fp, char *activity, struct mmio_t *pkt)
{
  FUNC_CALL_ENTRY;

  char mmio_action_type[20];
  memset(mmio_action_type, 0, 20);

  snprintf(mmio_action_type, 20,
	   "MMIO-%s-%d-%s",
	   (pkt->write_en == MMIO_WRITE_REQ ? "Write" : "Read"),
	   pkt->width,
	   (pkt->resp_en == 0 ? "Req " : "Resp") );

  fprintf(fp, "%s\t%03x\t%s\t%x\t%llx\n", activity,
	  pkt->tid, mmio_action_type, pkt->addr, pkt->qword[0]);

  FUNC_CALL_EXIT;
}


/*
 * ase_free_buffer : Free memory if not NULL
 */
void ase_free_buffer(char *ptr)
{
  if (ptr != (char*)NULL)
    {
      free (ptr);
    }
}


/*
 * register_signal : Register Signal Handler
 */
void register_signal (int sig, void *handler )
{
  FUNC_CALL_ENTRY;

  struct sigaction cfg;

  // Configure signal handler
  cfg.sa_handler = handler;
  sigemptyset(&cfg.sa_mask);
  cfg.sa_flags = SA_RESTART;

  // Declare signal action
  sigaction (sig, &cfg, 0);

  FUNC_CALL_EXIT;
}


/*
 * ret_random_in_range : Return random number in a range
 */
uint32_t ret_random_in_range(int low, int high)
{
  return (rand() % (high+1-low) + low );
}


/*
 * ase_string_copy
 * ASE's own safe string copy insures a null-termination
 * NOTE: dest must be malloc'ed before use (use ase_malloc)
 */
void ase_string_copy(char *dest, const char *src, size_t num_bytes)
{
  FUNC_CALL_ENTRY;

  int dest_strlen;

  // Allocate memory if not already done
  if (dest == NULL)
    {
#ifdef ASE_DEBUG
      BEGIN_RED_FONTCOLOR;
      printf("  [DEBUG]  ase_string_copy => Destination string not allocated, running ase_malloc\n");
      END_RED_FONTCOLOR;
#endif
      dest = ase_malloc(num_bytes);
    }

  // Use snprintf as a copy mechanism
  snprintf(dest, num_bytes, "%s", src);

  // Find length
  dest_strlen = strlen(dest);

  // Terminate length, or kill
  if ( dest_strlen < ASE_MQ_NAME_LEN )
    {
      dest[dest_strlen] = '\0';
    }
  else
    {
      BEGIN_RED_FONTCOLOR;
#ifdef SIM_SIDE
      printf("SIM-C : ** Internal Error ** => Invalid null termination during string copy ");
      start_simkill_countdown();
#else
      printf("  [APP]  ** Internal Error ** => Invalid null termination during string copy ");
      exit(1);
#endif
      END_RED_FONTCOLOR;
    }

  FUNC_CALL_EXIT;
}
