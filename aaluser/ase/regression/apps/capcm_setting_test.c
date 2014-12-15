#include "ase_common.h"


/*
 * Calculate Sysmem & CAPCM ranges to be used by ASE
 */
#if 0
void calc_phys_memory_ranges(int memmap_sad_setting, int enable_capcm)
{
  uint32_t cipuctl_22;
  uint32_t cipuctl_21_19;

  cipuctl_22    = (memmap_sad_setting & 0xF) >> 3;
  cipuctl_21_19 = (memmap_sad_setting & 0x7);
  printf("        CIPUCTL[22] = %d | CIPUCTL[21:19] = %d\n", cipuctl_22, cipuctl_21_19 );

  // Memmory map calculation
  if (enable_capcm)
    {
      capcm_size = (uint64_t)( pow(2, cipuctl_21_19 + 1) * 1024 * 1024 * 1024);
      sysmem_size = (uint64_t)( (uint64_t)pow(2, FPGA_ADDR_WIDTH) - capcm_size);
      
      // Place CAPCM based on CIPUCTL[22]
      if (cipuctl_22 == 0)
	{
	  capcm_phys_lo = 0;
	  capcm_phys_hi = capcm_size - 1;
	  sysmem_phys_lo = capcm_size;
	  sysmem_phys_hi = (uint64_t)pow(2, FPGA_ADDR_WIDTH) - 1;
	} 
      else
	{
	  capcm_phys_hi = (uint64_t)pow(2,FPGA_ADDR_WIDTH) - 1;
	  capcm_phys_lo = capcm_phys_hi + 1 - capcm_size;
	  sysmem_phys_lo = 0;
	  sysmem_phys_hi = sysmem_phys_lo + sysmem_size;
	}
    }
  else
    {
      sysmem_size = (uint64_t)pow(2, FPGA_ADDR_WIDTH);
      sysmem_phys_lo = 0;
      sysmem_phys_hi = sysmem_size-1;
      capcm_size = 0;
      capcm_phys_lo = 0;
      capcm_phys_hi = 0;
    }

  BEGIN_YELLOW_FONTCOLOR;
  printf("        System memory range  => 0x%016lx-0x%016lx | %d~%d GB \n", 
	 sysmem_phys_lo, sysmem_phys_hi, sysmem_phys_lo/(uint64_t)pow(1024, 3), (uint64_t)(sysmem_phys_hi+1)/(uint64_t)pow(1024, 3) );
  if (enable_capcm)
    printf("        Private memory range => 0x%016lx-0x%016lx | %d~%d GB\n", 
	   capcm_phys_lo, capcm_phys_hi, capcm_phys_lo/(uint64_t)pow(1024, 3), (uint64_t)(capcm_phys_hi+1)/(uint64_t)pow(1024, 3) );
  END_YELLOW_FONTCOLOR;

  // Internal check messages
  if (enable_capcm)
    {
      if (capcm_size > (uint64_t)8*1024*1024*1024 )
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        Caching agent private memory size > 8 GB, this can cause a virtual memory hog\n");
	  printf("        Consider using a smaller memory for simulation !! \n");
	  printf("        Simulation will continue with requested setting, change to a smaller CAPCM in ase.cfg !!\n");
	  END_RED_FONTCOLOR;
	}
      if (sysmem_size == 0)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        System SAD setting has set System Memory size to 0 bytes. Please check that this is intended !\n");
	  printf("        Any SW Workspace Allocate action will FAIL !!\n");
	  printf("        Simulation will continue with requested setting...\n");
	  END_RED_FONTCOLOR;
	}
      if (capcm_size == 0)
	{
	  BEGIN_RED_FONTCOLOR;
	  printf("SIM-C : WARNING =>\n");
	  printf("        CAPCM is enabled and has size set to 0 bytes. Please check that this is intended !!\n");
	  printf("        Simulation will continue, but NO CAPCM regions will be created");
	  END_RED_FONTCOLOR;
	}
    }
}
#endif


int main()
{
  int i;

  uint64_t sysmem_phys_lo;
  uint64_t sysmem_phys_hi;
  uint64_t capcm_phys_lo;
  uint64_t capcm_phys_hi;
    
  sysmem_phys_lo = 0x0000000200000000;
  sysmem_phys_hi = 0x0000003fffffffff;
  
  capcm_phys_lo = 0x0000000000000000;
  capcm_phys_hi = 0x00000001ffffffff;

  get_range_checked_physaddr(0x200000);


  return 0;
}
