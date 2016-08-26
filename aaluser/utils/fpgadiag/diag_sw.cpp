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
// @file diag_sw.cpp
// @brief NLB SW test application file.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 10/15/2015     SC	   fpgadiag version.@endverbatim
//****************************************************************************

//SW: This test measures the full round trip data movement latency between CPU and FPGA.
//The test can be configured to use 4 different CPU to FPGA messaging methods -
// 1. Polling from AFU
// 2. UMsg without data
// 3. UMsg with data
// 4. CSR write
#include <aalsdk/kernel/ccipdriver.h>
#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBSW::RunTest(const NLBCmdLine &cmd)
{
	btInt res = 0;
	btWSSize  sz = CL(cmd.begincls);

    // We need to initialize the input and output buffers, so we need addresses suitable
    // for dereferencing in user address space.
    // volatile, because the FPGA will be updating the buffers, too.
    volatile btVirtAddr pInputUsrVirt = m_pMyApp->InputVirt();

    const    btUnsigned32bitInt  InputData = 0xdecafbad;
    volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
    volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
	                                     (m_pMyApp->InputSize() / sizeof(btUnsigned32bitInt));

    volatile btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();
    volatile btVirtAddr pUMsgUsrVirt = m_pMyApp->UMsgVirt();
    volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

    if(NULL != pUMsgUsrVirt &&
      ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA) ||
        flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT))){

        NamedValueSet nvs;
        if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA)){
           nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)LOW);

        }else if (flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT)){
           nvs.Add(UMSG_HINT_MASK_KEY, (btUnsigned64bitInt)HIGH);
        }

        btBool ret = m_pALIuMSGService->umsgSetAttributes(nvs);
    }

    // Initiate AFU Reset
    if (0 != m_pALIResetService->afuReset()){
       ERR("AFU reset failed. Exiting test.");
       return 1;
    }

    if(NULL != m_pVTPService){
    	m_pVTPService->vtpReset();
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
    m_pALIMMIOService->mmioWrite32(CSR_CFG, 0);
    csr_type cfg = (csr_type)NLB_TEST_MODE_SW;

    if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDI)){
       cfg |= (csr_type)NLB_TEST_MODE_RDI;
	 }

    if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE)){
       cfg |= (csr_type)NLB_TEST_MODE_CSR_WRITE;
    }
    else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA)){
       cfg |= (csr_type)NLB_TEST_MODE_UMSG_DATA;
    }
    else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT)){
       cfg |= (csr_type)NLB_TEST_MODE_UMSG_HINT;
    }

    //Check for write-through or write-push mode and add to CSR_CFG
    if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRPUSH_I)){
	   cfg |= (csr_type)NLB_TEST_MODE_WRPUSH_I;
    }
     else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRLINE_I)){
	   cfg |= (csr_type)NLB_TEST_MODE_WRLINE_I;
    }

    // Select the read channel.
    if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VL0)){
      cfg |= (csr_type)NLB_TEST_MODE_READ_VL0;
    }
    else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH0)){
      cfg |= (csr_type)NLB_TEST_MODE_READ_VH0;
    }
    else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH1)){
      cfg |= (csr_type)NLB_TEST_MODE_READ_VH1;
    }
    else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VR)){
      cfg |= (csr_type)NLB_TEST_MODE_READ_VR;
    }

    // Select the write fence channel.
	bool wrfence_flag = false;
	if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRFENCE_VA)){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VA;
		wrfence_flag = true;
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRFENCE_VL0)){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VL0;
		wrfence_flag = true;
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRFENCE_VH0)){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VH0;
		wrfence_flag = true;
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRFENCE_VH1)){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VH1;
		wrfence_flag = true;
	}

	// Select the write channel.
	if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VL0)){
	 cfg |= (csr_type)NLB_TEST_MODE_WRITE_VL0;

	 if(!wrfence_flag){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VL0;
	 }
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH0)){
	 cfg |= (csr_type)NLB_TEST_MODE_WRITE_VH0;

	 if(!wrfence_flag){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VH0;
	  }
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH1)){
	 cfg |= (csr_type)NLB_TEST_MODE_WRITE_VH1;

	 if(!wrfence_flag){
		cfg |= (csr_type)NLB_TEST_MODE_WRFENCE_VH1;
	  }
	}
	else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VR)){
	 cfg |= (csr_type)NLB_TEST_MODE_WRITE_VR;
	}

    m_pALIMMIOService->mmioWrite32(CSR_CFG, (csr_type)cfg);

#if   defined( __AAL_WINDOWS__ )
#error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec ts;
   ts.tv_sec = 10;
   Timer     timeout = Timer() + Timer(&ts);
#endif // OS

   btInt StopTimeoutMillis = 250;
   if ( cmd.AFUTarget == ALIAFU_NVS_VAL_TARGET_ASE){
	StopTimeoutMillis = StopTimeoutMillis * 100000;
   }
   btInt MaxPoll = StopTimeoutMillis;

   ReadPerfMonitors();
   SavePerfMonitors();

   while ( sz <= CL(cmd.endcls)){

      // Clear the UMsg address space
      ::memset((void *)pUMsgUsrVirt, 0, m_pMyApp->UMsgSize());

      // zero the output buffer
      ::memset((void *)pOutputUsrVirt, 0, m_pMyApp->OutputSize());

      //Re-initialise the Input buffer
      pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
      for ( ; pInput < pEndInput ; ++pInput ) {
          *pInput = InputData;
      }

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

	   timeout = Timer() + Timer(&ts);

	   //Test flow
	   //1. CPU polls on Addr N+1
	   while ( *(btUnsigned32bitInt *)(pOutputUsrVirt + sz) != HIGH){
		  if ( Timer() > timeout ){
			 res++;
			 cerr << "Maximum timeout for CPU poll on Address N+1 was exceeded\n";
			 break;
		  }
	   }

	  //2. CPU copies from dst to src buffer
	  // Copy could perturb the latency numbers based on CPU load
	  ::memcpy((void *)pInputUsrVirt, (void *)pOutputUsrVirt, sz);

	  //fence operation
	  __sync_synchronize();

	  //3. CPU -> FPGA message. Select notice type
	  if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE)){
		 m_pALIMMIOService->mmioWrite32(CSR_SW_NOTICE, 0x10101010);
	  }
	  else if( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA) || flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT)){
		 *(btUnsigned32bitInt *)pUMsgUsrVirt = HIGH;
	  }
	  else{
		 *(btUnsigned32bitInt *)(pInputUsrVirt + sz)     = HIGH;
		 *(btUnsigned32bitInt *)(pInputUsrVirt + sz + 4) = HIGH;
		 *(btUnsigned32bitInt *)(pInputUsrVirt + sz + 8) = HIGH;
	  }

	  // Wait for test completion
	  timeout = Timer() + Timer(&ts);

	  while ( ( 0 == pAFUDSM->test_complete ) &&
			  ( 0 == res) ){
		  	if(Timer() > timeout ){
			  res++;
			  cerr << "Maximum Timeout for test complete was exceeded.\n";
			  break;
		  	}
	  }

	  // Stop the device
	  m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

	  while ( ( 0 == pAFUDSM->test_complete ) &&
			  ( MaxPoll >= 0 ) ){
		  	MaxPoll -= 1;
		  	SleepMilli(1);
	  }

	  ReadPerfMonitors();

	  // Check the device status
     if ( MaxPoll < 0 ) {
        cerr << "The maximum timeout for test stop was exceeded." << endl;
        ++res;
//        break;
     }

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
//        break;
     }

     //Checking for num_clocks underflow.
     if ( pAFUDSM->num_clocks < (pAFUDSM->start_overhead + pAFUDSM->end_overhead))
     {
        cerr << "Number of Clocks underflow.\n";
        ++res;
//        break;
     }

	  PrintOutput(cmd, (sz / CL(1)));

	  SavePerfMonitors();

	  //Increment number of cachelines
	  sz += CL(1);
	  MaxPoll = StopTimeoutMillis;
   }
   //Disable UMsgs upon test completion
   //m_pALIMMIOService->mmioWrite32(CSR_UMSG_BASE, 0);

   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // Initiate AFU Reset
   if ( 0 != m_pALIResetService->afuReset())
   {
      ERR("AFU reset failed after test completion.");
      ++res;
   }

   return res;
}

void  CNLBSW::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
   bt64bitCSR ticks;
   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

   cout << endl;
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
        << setw(10) << pAFUDSM->num_reads          << ' '
        << setw(11) << pAFUDSM->num_writes         << ' '
        << setw(12) << GetPerfMonitor(READ_HIT)    << ' '
        << setw(12) << GetPerfMonitor(WRITE_HIT)   << ' '
        << setw(13) << GetPerfMonitor(READ_MISS)   << ' '
        << setw(13) << GetPerfMonitor(WRITE_MISS)  << ' '
        << setw(10) << GetPerfMonitor(EVICTIONS)   << ' ';

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
