#include "ase_common.h"


/*
 * Write simulation seed to file
 */
void ase_write_seed(uint64_t seed)
{
  FILE *fp_seed;
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
  FILE *fp_seed;
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

