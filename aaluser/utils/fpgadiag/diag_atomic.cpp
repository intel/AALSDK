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
// @file diag_write.cpp
// @brief NLB Write test application file.
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

//WRITE: This is a write only test with no data checking. AFU writes CSR_NUM_LINES
//starting from CSR_DST_ADDR location. This test is used to stress the write
//path and measure 100% write bandwidth and latency.
#include <aalsdk/kernel/aalui.h>
#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBAtomic::RunTest(const NLBCmdLine &cmd)
{
   btInt res = 0;
   btWSSize  sz = CL(cmd.begincls);

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   btVirtAddr pCoolOffUsrVirt = m_pMyApp->InputVirt();
   btVirtAddr pOutputUsrVirt  = m_pMyApp->OutputVirt();

   // Zero the dest buffer.
   ::memset((void*)pOutputUsrVirt, 0, m_pMyApp->OutputSize());

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   const btUnsigned32bitInt WriteBufData = 0x0;
   volatile btUnsigned32bitInt *pOutput    = (volatile btUnsigned32bitInt *)pOutputUsrVirt;
   volatile btUnsigned32bitInt *pEndOutput = (volatile btUnsigned32bitInt *)pOutput +
                                        	 	(m_pMyApp->OutputSize() / sizeof(btUnsigned32bitInt));

   for ( ; pOutput < pEndOutput ; ++pOutput ) {
         *pOutput = WriteBufData;
   }

   pOutput    = (volatile btUnsigned32bitInt *)pOutputUsrVirt;
   pEndOutput = (volatile btUnsigned32bitInt *)pOutput +
				    (m_pMyApp->OutputSize() / sizeof(btUnsigned32bitInt));

   // Initiate AFU Reset
   if (0 != m_pALIResetService->afuReset())
   {
      ERR("AFU reset failed. Exiting test.");
      return 1;
   }

   //Set DSM base, high then low
   m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   // Assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // De-assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

   // Set output workspace address
   m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the test mode
   m_pALIMMIOService->mmioWrite32(CSR_CFG, 0);
   csr_type cfg = (csr_type)NLB_TEST_MODE_ATOMIC | NLB_CMD_FLAG_VL0;

   if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_ST))
   {
	   cfg |= NLB_TEST_MODE_ST;
   }

   csr_type csr_hqw = cmd.hqw;
   csr_hqw = csr_hqw << 17;
   csr_hqw &= TEST_MODE_CSR_HQW;
   cfg |= csr_hqw;

   m_pALIMMIOService->mmioWrite32(CSR_CFG, (csr_type)cfg);

#if   defined( __AAL_WINDOWS__ )
#error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec ts       = cmd.timeout;
   Timer     absolute = Timer() + Timer(&ts);
#endif // OS

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = StopTimeoutMillis;

   cout << endl;
   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
		 	   //0123456789 0123456789 01234567890 0123456789012
		cout << "Cachelines Read_Count Write_Count 'Clocks(@"
		 << Normalized(cmd) << ")' Total_CPU_CX ";

		if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
				     // 01234567890123 01234567890123
			cout << "   Rd_Bandwidth   Wr_Bandwidth";
		}
		cout << endl;
   }

   btInt numSWCx = 0;
   btInt numHWCx = 0;
   btInt input_numCX = cmd.cx;
   btInt cmp_val, new_val, sw_add_val;
   if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_ST))
   {
	   sw_add_val = 1;

	   if (0 != (input_numCX % 2))
	   {
		   --input_numCX;
		   numSWCx = input_numCX / 2;
		   numHWCx = (input_numCX / 2) + 1;
	   }
	   else
	   {
		   numSWCx = input_numCX / 2;
		   numHWCx = input_numCX / 2;
	   }
   }
   else
   {
	   sw_add_val = 0;

	   numSWCx = input_numCX;
	   numHWCx = input_numCX;
   }
#if 0
   m_pALIMMIOService->mmioWrite32(CSR_CFG_H, (csr_type)numHWCx);

   // Set the number of cache lines for the test
   m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, (csr_type)(sz / CL(1)));

   // Start the test
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);
#endif
//SW Thread Execution

   pOutput += cmd.sqw;
   btBool sw_timeout_flag = false;
   btInt sw_retry = 0;
   btBool bCmpXchg = false;
   btBool bSWCmp = false;
   btUnsigned64bitInt total_cpu_numCX = 0;
   cmp_val = sw_add_val;
   new_val = 1 + sw_add_val;

   for(int i = 0; i <= numSWCx; i++)
   {
	   sw_retry = 0;
	   bCmpXchg = false;

	   do{
		   bCmpXchg = __sync_bool_compare_and_swap(pOutput, cmp_val, new_val);
		   ++sw_retry;
		   ++total_cpu_numCX;

		   if (sw_retry > 99999)
		   {
			   bCmpXchg = true;
			   sw_timeout_flag = true;
		   }
	   }while(!bCmpXchg);

	   if(sw_timeout_flag)
	   {
		   cerr << "SW thread timeout.\n";
		   ++res;
		   break;
	   }

	   cmp_val += (sw_add_val + 1);
	   new_val += (sw_add_val + 1);
   }
#if 0
   // Wait for test completion
   while ( 0 == pAFUDSM->test_complete ) {
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
#endif
   PrintOutput(cmd, (sz / CL(1)), total_cpu_numCX);

#if 0
   // Check the device status
   if ( MaxPoll < 0 ) {
	  cerr << "The maximum timeout for test stop was exceeded." << endl;
	  ++res;
   }

   if ( 0 != pAFUDSM->test_error ) {
	  cerr << "Error bit set in DSM.\n";
	  ++res;
   }
#endif
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // Initiate AFU Reset
   if (0 != m_pALIResetService->afuReset())
   {
      ERR("AFU reset failed after test completion.");
      ++res;
   }

   return res;
}

void  CNLBAtomic::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls, const btInt cpu_cx)
{
	nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
	bt64bitCSR ticks;
	bt64bitCSR rawticks     = pAFUDSM->num_clocks;
	bt32bitCSR startpenalty = pAFUDSM->start_overhead;
	bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

	cout << setw(10) << cls 							<< ' '
	     << setw(10) << pAFUDSM->num_reads    	<< ' '
		  << setw(11) << pAFUDSM->num_writes   	<< ' ';

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

	cout << setw(15) << cpu_cx << ' ';

	cout << "  "
		 << setw(13) << CalcReadBandwidth(cmd) << ' '
		 << setw(13) << CalcWriteBandwidth(cmd);
	}

	cout << endl;
}
