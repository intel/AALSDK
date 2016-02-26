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

#include "ase_common.h"


/*
 * Write simulation seed to file
 */
void ase_write_seed(uint64_t seed)
{
  FILE *fp_seed = (FILE *)NULL;
  fp_seed = fopen(ASE_SEED_FILE, "w");
  if (fp_seed == NULL)
    {
      printf("SIM-C : ASE Seed file could not be written\n");
      ase_error_report("fopen", errno, ASE_OS_FOPEN_ERR);
    }
  else
    {
      fprintf(fp_seed, "%lu", seed);
      fclose(fp_seed);
    }  
}


/*
 * Readback simulation seed - used if ENABLE_REUSE_SEED is enabled
 */
uint64_t ase_read_seed()
{
  uint64_t readback_seed;
  uint64_t new_seed;
  FILE *fp_seed = (FILE *)NULL;
  fp_seed = fopen(ASE_SEED_FILE, "r");
  if (fp_seed == NULL)
    {
      printf("SIM-C : ASE Seed file could not be read\n");
      printf("        Old seed unusable --- creating a new seed\n");
      new_seed = time(NULL);
      ase_write_seed(new_seed);      
      return new_seed;
    }
  else
    {
      fscanf(fp_seed, "%lu", &readback_seed);
      printf("SIM-C : Readback seed %lu\n", readback_seed);
      fclose(fp_seed);
      return readback_seed;
    }
}


/*
 * Generate 64-bit random number
 */
uint64_t ase_rand64()
{
  uint64_t random;
  random = rand();
  random = (random << 32) | rand();
  return random;
}

/*
 * Shuffle an array of numbers
 * USAGE: For setting up latency_scoreboard
 */
void shuffle_int_array(int *array, int num_items)
{
  int i, j;
  int tmp;

  if (num_items > 1)
    {
      for (i = 0; i < num_items-1; i++)
        {
          j = i + rand() / (RAND_MAX / (num_items - i) + 1);
          tmp = array[j];
          array[j] = array[i];
          array[i] = tmp;
        }
    }  
}

