// Copyright (c) 2013-2014, Intel Corporation
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
// @brief NLB VAFU template application file.
// @ingroup
// @verbatim
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
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

#include "ccis_diag_defaults.h"
#include "ccis_diag-common.h"
#include "nlb-specific.h"
#include "ccis_diag-nlb-common.h"

//Redefining the CSR Map

/*#undef CSR_AFU_DSM_BASEL
#undef CSR_AFU_DSM_BASEH
#undef CSR_SRC_ADDR
#undef CSR_DST_ADDR
#undef CSR_NUM_LINES
#undef CSR_CTL
#undef CSR_CFG

#define CSR_AFU_DSM_BASEL         	0x0a00
#define CSR_AFU_DSM_BASEH         	0x0a04
#define CSR_SRC_ADDR              	0x0a20
#define CSR_DST_ADDR              	0x0a24
#define CSR_NUM_LINES             	0x0a28
#define CSR_CTL                   	0x0a2c
#define CSR_CFG                   	0x0a34*/

// non-continuous mode.
// no cache treatment.
btInt CNLBCcipLpbk1::RunTest(const NLBCmdLine &cmd, btWSSize wssize)
{
   btInt 	 res = 0;
   btWSSize  sz = CL(cmd.begincls);
   uint_type NumCacheLines = (cmd.endcls - cmd.begincls) + 1;

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   m_pCCIAFU->WorkspaceAllocate(wssize,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceAllocate(wssize,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt = m_pMyApp->InputVirt();

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

   //Set DSM base, high then low
   //m_pCCIAFU->CSRWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());
   m_pCCIAFU->CSRWrite(CSR_AFU_DSM_BASEH, m_pMyApp->DSMPhys() >> 32);
   m_pCCIAFU->CSRWrite(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys() & 0xffffffff);

   // Assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pCCIAFU->CSRWrite(CSR_CTL, 1);

   // Set input workspace address
   m_pCCIAFU->CSRWrite(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->InputPhys()));

   // Set output workspace address
   m_pCCIAFU->CSRWrite(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_pMyApp->OutputPhys()));

   // Set the test mode
   csr_type cfg = (csr_type)NLB_TEST_MODE_LPBK1;
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))
   {
	   cfg |= (csr_type)NLB_TEST_MODE_CONT;
   }
   // Check for write through mode and add to CSR_CFG
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WT))
   {
	   cfg |= (csr_type)NLB_TEST_MODE_WT;
   }
   // Set the read flags.
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

   m_pCCIAFU->CSRWrite(CSR_CFG, cfg);

   cout << endl;
   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
		     //0123456789 0123456789 01234567890 0123456789012
	  cout << "Cachelines Read_Count Write_Count 'Clocks(@"
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

   while ( sz <= CL(cmd.endcls) )
   {
	   // Assert Device Reset
		m_pCCIAFU->CSRWrite(CSR_CTL, 0);

		// Clear the DSM status fields
		::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

		// De-assert Device Reset
		m_pCCIAFU->CSRWrite(CSR_CTL, 1);

	    // Set the number of cache lines for the test
	    m_pCCIAFU->CSRWrite(CSR_NUM_LINES, (csr_type)(sz / CL(1)));

	    // Start the test
	    m_pCCIAFU->CSRWrite(CSR_CTL, 3);

	    // In cont mode, send a stop signal after timeout. Wait till DSM complete register goes high
	     if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))
	   {
		   //Wait till timeout.
		   while(Timer() < absolute){
			   SleepNano(10);
		   }

		   // Stop the device
		   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

		   //wait for DSM register update or timeout
		   while ( 0 == pAFUDSM->test_complete &&
				 ( MaxPoll >= 0 )) {
			   MaxPoll -= 500;
			   SleepNano(500);
		   }

		   //Update timer.
		   absolute = Timer() + Timer(&ts);
	    }
	    else	//In non-cont mode, wait till test completes and then stop the device.
	    {
		   // Wait for test completion or timeout
		   while ( 0 == pAFUDSM->test_complete &&
				 ( MaxPoll >= 0 )) {
			   MaxPoll -= 500;
			   SleepNano(500);
		   }

		   // Stop the device
		   m_pCCIAFU->CSRWrite(CSR_CTL, 7);
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

   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   // Verify the buffers
   if ( ::memcmp((void *)pInputUsrVirt, (void *)pOutputUsrVirt, NumCacheLines) != 0 )
   {
      ++res;
   }

   // Verify the device
   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   // Clean up..

   // Release the Workspaces
   m_pCCIAFU->WorkspaceFree(pInputUsrVirt,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceFree(pOutputUsrVirt,      TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
   m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

   // Synchronize with the workspace free event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace free failed");
      return 1;
   }

   return res;
}

void  CNLBCcipLpbk1::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
	  nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
	  bt64bitCSR ticks;
      bt64bitCSR rawticks     = pAFUDSM->num_clocks;
      bt32bitCSR startpenalty = pAFUDSM->start_overhead;
      bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

	  cout << setw(10) << std::dec << cls					<< ' '
		   << setw(10) << std::dec << pAFUDSM->num_reads    << ' '
		   << setw(11) << std::dec << pAFUDSM->num_writes   << ' ';

	  if(flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT) ) {
		  ticks = rawticks - startpenalty;
	  }
	  else
	  {
		  ticks = rawticks - (startpenalty + endpenalty);
	  }
	  cout  << setw(16) << std::dec << ticks;

	 if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
		double rdbw = 0.0;
		double wrbw = 0.0;

		cout << "  "
			 << setw(14) << CalcReadBandwidth(cmd) << ' '
			 << setw(14) << CalcWriteBandwidth(cmd);
	 }
	 cout << endl;
}


