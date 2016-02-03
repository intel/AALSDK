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
// @file diag_trput.cpp
// @brief NLB Trput test application file.
// @ingroup
// @verbatim
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 05/30/2013     TSW      Initial version.
// 07/30/2105     SC	   fpgadiag version@endverbatim
//****************************************************************************

//TRPUT: This test combines the read and write streams. There is no data checking
//and no dependency between read and writes. It reads CSR_NUM_LINES starting from
//CSR_SRC_ADDR location and writes CSR_NUM_LINS to CSR_DST_ADDR. It is also used
//to measure 50% read + 50% write bandwidth.
#include <aalsdk/kernel/aalui.h>
#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBCcipTrput::RunTest(const NLBCmdLine &cmd)
{
   btInt res = 0;
   btWSSize  sz = CL(cmd.begincls);

   btVirtAddr pInputUsrVirt  = m_pMyApp->InputVirt();
   btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // Initiate AFU Reset
   m_pALIResetService->afuReset();

   //Set DSM base, high then low
   m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   // Assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // De-assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

   //__sync_synchronize();

   // Set input workspace address
   m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set output workspace address
   m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys())+512); //offset added to avoid clashes in cache

   // Set the test mode
   m_pALIMMIOService->mmioWrite32(CSR_CFG, 0);
   csr_type cfg = (csr_type)NLB_TEST_MODE_TRPUT;
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))
     {
       cfg |= (csr_type)NLB_TEST_MODE_CONT;
     }

   //Check for write through mode and add to CSR_CFG
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WT))
   {
     cfg |= (csr_type)NLB_TEST_MODE_WT;
   }

   // Set the read type flags of the CSR_Config.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDI))
   {
	cfg |= (csr_type)NLB_TEST_MODE_RDI;
   }
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_RDO))
   {
    cfg |= (csr_type)NLB_TEST_MODE_RDO;
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

   m_pALIMMIOService->mmioWrite32(CSR_CFG, cfg);

   ReadPerfMonitors();
   SavePerfMonitors();

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

#if   defined( __AAL_WINDOWS__ )
#error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec ts       = cmd.timeout;
   Timer     absolute = Timer() + Timer(&ts);
#endif // OS

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = StopTimeoutMillis;


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
		   else	//In non-cont mode, wait till test completes and then stop the device.
		   {
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

		   PrintOutput(cmd, (sz / CL(1)));

		   SavePerfMonitors();

		   // Increment number of cachelines.
		   sz += CL(1);

		   // Check the device status
		   if ( MaxPoll < 0 ) {
		      cerr << "The maximum timeout for test stop was exceeded." << endl;
		      ++res;
		      break;
		   }
		   MaxPoll = StopTimeoutMillis;

		   if ( 0 != pAFUDSM->test_error ) {
			  cerr << "Error bit set in DSM.\n";
		      ++res;
		      break;
		   }
   }

   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   return res;
}

void  CNLBCcipTrput::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
	nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
	bt64bitCSR ticks;
    bt64bitCSR rawticks     = pAFUDSM->num_clocks;
    bt32bitCSR startpenalty = pAFUDSM->start_overhead;
    bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

    cout << setw(10) << cls 								<< ' '
	     << setw(10) << pAFUDSM->num_reads    				<< ' '
	     << setw(11) << pAFUDSM->num_writes   				<< ' '
	     << setw(12) << GetPerfMonitor(READ_HIT)      		<< ' '
	     << setw(12) << GetPerfMonitor(WRITE_HIT)      		<< ' '
	     << setw(13) << GetPerfMonitor(READ_MISS)      		<< ' '
	     << setw(13) << GetPerfMonitor(WRITE_MISS)      	<< ' '
	     << setw(10) << GetPerfMonitor(EVICTIONS)     		<< ' ';

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

