// Copyright (c) 2015, Intel Corporation
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
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
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
#include <aalsdk/kernel/aalui.h>
#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBCcipSW::RunTest(const NLBCmdLine &cmd)
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

    for ( ; pInput < pEndInput ; ++pInput ) {
         *pInput = InputData;
      }

    volatile btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();
    volatile btVirtAddr pUMsgUsrVirt = m_pMyApp->UMsgVirt();
    volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

    /*
	if ( 0 != CacheCooldown(pOutputUsrVirt, m_pMyApp->OutputPhys(), m_pMyApp->OutputSize()) ) {
		return 1;
	}
*/

    // Initiate AFU Reset
    m_pALIResetService->afuReset();

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

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDI))
   {
	 cfg |= (csr_type)NLB_TEST_MODE_RDI;
	}
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDO))
   {
	 cfg |= (csr_type)NLB_TEST_MODE_RDO;
   }
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE))
   {
	 cfg |= (csr_type)NLB_TEST_MODE_CSR_WRITE;
   }
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA))
   {
	 cfg |= (csr_type)NLB_TEST_MODE_UMSG_DATA;
   }
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT))
   {
	 cfg |= (csr_type)NLB_TEST_MODE_UMSG_HINT;
   }
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WT)) //Check for write through mode and add to CSR_CFG
    {
	 cfg |= (csr_type)NLB_TEST_MODE_WT;
    }
   // Select the channel.
     if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_QPI))
     {
      cfg |= (csr_type)NLB_TEST_MODE_QPI;
     }
     else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_PCIE0))
     {
      cfg |= (csr_type)NLB_TEST_MODE_PCIE0;
     }
     else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_PCIE1))
     {
      cfg |= (csr_type)NLB_TEST_MODE_PCIE1;
     }

     m_pALIMMIOService->mmioWrite32(CSR_CFG, (csr_type)cfg);

#if   defined( __AAL_WINDOWS__ )
#error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec ts;
   ts.tv_sec = 10;
   Timer     timeout = Timer() + Timer(&ts);
#endif // OS

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);

   cout << endl;
   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
		  //0123456789 0123456789 01234567890 0123456789012
   cout << "Cachelines Read_Count Write_Count 'Clocks(@"
	    << Normalized(cmd)  << ")'";

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
			  // 01234567890123 01234567890123
	 cout << "   Rd_Bandwidth   Wr_Bandwidth";
  }

  cout << endl;
  }

   while ( sz <= CL(cmd.endcls))
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

	  timeout = Timer() + Timer(&ts);

	  //Test flow
	  //1. CPU polls on Addr N+1
	  while(*(btUnsigned32bitInt *)(pOutputUsrVirt + sz) != HIGH)
	  {
		  if(Timer() > timeout )
		  {
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
	  if( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CSR_WRITE))
	  {
		  m_pALIMMIOService->mmioWrite32(CSR_SW_NOTICE, 0x10101010);
	  }
	  else if( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA) || flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT))
	  {
		  *(btUnsigned32bitInt *)pUMsgUsrVirt = HIGH;
	  }
	  else
	  {
		  *(btUnsigned32bitInt *)(pInputUsrVirt + sz)     = HIGH;
		  *(btUnsigned32bitInt *)(pInputUsrVirt + sz + 4) = HIGH;
		  *(btUnsigned32bitInt *)(pInputUsrVirt + sz + 8) = HIGH;
	  }

	  // Wait for test completion
	  timeout = Timer() + Timer(&ts);

	  while ( ( 0 == pAFUDSM->test_complete ) &&
			   ( 0 == res) )
	  {
		  if(Timer() > timeout )
		  {
			  res++;
			  cerr << "Maximum Timeout for test complete was exceeded.\n";
			  break;
		  }
	  }


	   // Stop the device
	  m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

	   while ( ( 0 == pAFUDSM->test_complete ) &&
			 ( MaxPoll >= 0 ) )
	   {
		 MaxPoll -= 500;
		 SleepNano(500);
	   }

	   PrintOutput(cmd, (sz / CL(1)));

	   //Increment number of cachelines
	   sz += CL(1);

	   // Check the device status
	   if ( MaxPoll < 0 ) {
		   cerr << "The maximum timeout for test stop was exceeded." << endl;
		   ++res;
		   break;
	   }

	   MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);
   }
   //Disable UMsgs upon test completion
   //m_pALIMMIOService->mmioWrite32(CSR_UMSG_BASE, 0);

   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   if ( 0 != pAFUDSM->test_error ) {
	   cerr << "Test error bit was set in DSM.\n";
	   ++res;
   }

      return res;
}

void  CNLBCcipSW::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
	nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
	bt64bitCSR ticks;
    bt64bitCSR rawticks     = pAFUDSM->num_clocks;
    bt32bitCSR startpenalty = pAFUDSM->start_overhead;
    bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

	  cout << setw(10) << cls 					<< ' '
	       << setw(10) << pAFUDSM->num_reads    << ' '
	       << setw(11) << pAFUDSM->num_writes   ;

	  if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT) ) {
		  ticks = rawticks - startpenalty;
	  }
	  else
	  {
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

	    cout << endl;
}
