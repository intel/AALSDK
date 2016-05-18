// Copyright(c) 2015-2016, Intel Corporation
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
//****************************************************************************
// @file diag_lpbk1.cpp
// @brief NLB LPBK1 test application file.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 09/17/2015     SC      Initial version.@endverbatim
//****************************************************************************

//CCIP: This is a memory copy test. AFU copies CSR_NUM_LINES from source buffer to destination buffer.
//On test completion, the software compares the source and destination buffers.
#include <aalsdk/kernel/ccipdriver.h>
#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

// non-continuous mode.
// no cache treatment.
btInt CNLBLpbk1::RunTest(const NLBCmdLine &cmd)
{
   btInt 	 res = 0;
   btWSSize  sz = CL(cmd.begincls);
   uint_type mcl = cmd.multicls;
   uint_type NumCacheLines = cmd.begincls;

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = StopTimeoutMillis;

   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt = m_pMyApp->InputVirt();

   ::memset((void *)pInputUsrVirt, 0, m_pMyApp->InputSize());

   btUnsigned32bitInt  InputData = 0x00000000;
   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
                                     	 	(m_pMyApp->InputSize() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) {
      *pInput = InputData++;
   }

   volatile btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();

   // zero the output buffer
   ::memset((void *)pOutputUsrVirt, 0, m_pMyApp->OutputSize());

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // Initiate AFU Reset
   if (0 != m_pALIResetService->afuReset()){
      ERR("AFU reset failed. Exiting test.");
      return 1;
   }

   //Set DSM base, high then low
   m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   // Assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // De-assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

   // Set input workspace address
   m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set output workspace address
   m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the test mode
   csr_type cfg = (csr_type)NLB_TEST_MODE_LPBK1;
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT)){
	  cfg |= (csr_type)NLB_TEST_MODE_CONT;
   }

   // Check for write through mode and add to CSR_CFG
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WT)){
	  cfg |= (csr_type)NLB_TEST_MODE_WT;
   }

   // Set the read flags.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDI)){
	  cfg |= (csr_type)NLB_TEST_MODE_RDI;
   }

   // Select the channel.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_VL0)){
	  cfg |= (csr_type)NLB_TEST_MODE_VL0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_VH0)){
	  cfg |= (csr_type)NLB_TEST_MODE_VH0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_VH1)){
	  cfg |= (csr_type)NLB_TEST_MODE_VH1;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_VR)){
     cfg |= (csr_type)NLB_TEST_MODE_VR;
   }

   // Set Multi CL CSR.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_MULTICL))
   {
      if(2 == cmd.multicls){
         cfg |= (csr_type)NLB_TEST_MODE_MCL2;
      }
      else if(4 == cmd.multicls){
         cfg |= (csr_type)NLB_TEST_MODE_MCL4;
      }
   }

   m_pALIMMIOService->mmioWrite32(CSR_CFG, cfg);

   ReadPerfMonitors();
   SavePerfMonitors();

#if   defined( __AAL_WINDOWS__ )
#error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec ts       = cmd.timeout;
   Timer     absolute = Timer() + Timer(&ts);
#endif // OS

   while ( sz <= CL(cmd.endcls) )
   {
	    // Assert Device Reset
	    m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

		// Clear the DSM status fields
		::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

		// De-assert Device Reset
		m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

	    // Set the number of cache lines for the test
		m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, (csr_type)(sz / CL(1)));

	    // Start the test
		m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);

	    // In cont mode, send a stop signal after timeout. Wait till DSM complete register goes high
	    if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))
	    {
		   //Wait till timeout.
		   while(Timer() < absolute){
			   SleepNano(10);
		   }

		   // Stop the device
		   m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

		   //wait for DSM register update or timeout
		   while ( 0 == pAFUDSM->test_complete &&
				 ( MaxPoll >= 0 )) {
			   	 MaxPoll -= 1;
			   	 SleepMilli(1);
		   }

		   //Update timer.
		   absolute = Timer() + Timer(&ts);
	    }
	    else{	//In non-cont mode, wait till test completes and then stop the device.
	    		// Wait for test completion or timeout
		   while ( 0 == pAFUDSM->test_complete &&
				 ( MaxPoll >= 0 )) {
			   	 MaxPoll -= 1;
			   	 SleepMilli(1);
		   }

		   // Stop the device
		   m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);
	    }

	    ReadPerfMonitors();

	    // Check the device status
       if ( MaxPoll < 0 ) {
         cerr << "The maximum timeout for test stop was exceeded." << endl;
         ++res;
         break;
       }

       // Verify the device
       if ( 0 != pAFUDSM->test_error ) {
           cerr << "Error bit set in DSM.\n";
           cout << "DSM Test Error: 0x" << std::hex << pAFUDSM->test_error << endl;

           cout << "Mode error vector: " << endl;
           for (int i=0; i < 8; i++)
           {
             cout << "[" << i << "]: 0x" << pAFUDSM->mode_error[i] << endl;
           }
           cout << std::dec << endl;

           ++res;
           break;
       }

       // Verify the buffers
       if ( ::memcmp((void *)pInputUsrVirt, (void *)pOutputUsrVirt, NumCacheLines) != 0 ){
          cerr << "Data mismatch in Input and Output buffers.\n";
           ++res;
           break;
       }

       //Checking for num_clocks underflow.
        if(pAFUDSM->num_clocks < (pAFUDSM->start_overhead + pAFUDSM->end_overhead)){
           cerr << "Number of Clocks underflow.\n";
           ++res;
           break;
       }

	    PrintOutput(cmd, NumCacheLines);
	    SavePerfMonitors();

	   //Increment number of cachelines
	   sz += CL(mcl);
	   NumCacheLines += mcl;

	   MaxPoll = StopTimeoutMillis;
   }

   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // Initiate AFU Reset
   if (0 != m_pALIResetService->afuReset()){
      ERR("AFU reset failed after test completion.");
      ++res;
   }

   return res;
}

void  CNLBLpbk1::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
   bt64bitCSR ticks;
   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

   cout << endl << endl;
   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
            //0123456789 0123456789 01234567890 012345678901 012345678901 0123456789012 0123456789012 0123456789 0123456789012
      cout << "Cachelines Read_Count Write_Count Cache_Rd_Hit Cache_Wr_Hit Cache_Rd_Miss Cache_Wr_Miss   Eviction 'Clocks(@"
          << Normalized(cmd) << ")'";

      if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
                // 01234567890123 01234567890123
         cout << "   Rd_Bandwidth   Wr_Bandwidth";
      }
      cout << endl;
   }

   cout << setw(10) << cls                         << ' '
       << setw(10) << pAFUDSM->num_reads           << ' '
       << setw(11) << pAFUDSM->num_writes          << ' '
       << setw(12) << GetPerfMonitor(READ_HIT)     << ' '
       << setw(12) << GetPerfMonitor(WRITE_HIT)    << ' '
       << setw(13) << GetPerfMonitor(READ_MISS)    << ' '
       << setw(13) << GetPerfMonitor(WRITE_MISS)   << ' '
       << setw(10) << GetPerfMonitor(EVICTIONS)    << ' ';

   if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT) ) {
      ticks = rawticks - startpenalty;
   }
   else{
      ticks = rawticks - (startpenalty + endpenalty);
   }
   cout  << setw(16) << ticks;

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
       double rdbw = 0.0;
       double wrbw = 0.0;

       cout << "  "
            << setw(14) << CalcReadBandwidth(cmd) << ' '
            << setw(14) << CalcWriteBandwidth(cmd);
   }
   cout << endl << endl;

   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
               //0123456789012 012345678901 012345678901 012345678901 012345678901 012345678901
         cout << "VH0_Rd_Count VH0_Wr_Count VH1_Rd_Count VH1_Wr_Count VL0_Rd_Count VL0_Wr_Count " << endl;
      }

   cout << setw(12) << GetPerfMonitor(PCIE0_READ)     << ' '
        << setw(12) << GetPerfMonitor(PCIE0_WRITE)    << ' '
        << setw(12) << GetPerfMonitor(PCIE1_READ)     << ' '
        << setw(12) << GetPerfMonitor(PCIE1_WRITE)    << ' '
        << setw(12) << GetPerfMonitor(UPI_READ)       << ' '
        << setw(12) << GetPerfMonitor(UPI_WRITE)      << ' '
        << endl << endl;

}
