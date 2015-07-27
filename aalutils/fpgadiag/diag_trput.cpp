#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBTrput::RunTest(const NLBCmdLine &cmd, btWSSize wssize)
{
   btInt res = 0;
   btWSSize  sz = CL(cmd.begincls);

   m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
   m_pCCIAFU->WorkspaceAllocate(wssize, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
   m_pCCIAFU->WorkspaceAllocate(wssize, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

   // Synchronize with the workspace allocation event notifications.
   m_pMyApp->ClientWait();

   if ( !m_pMyApp->ClientOK() ) {
      ERR("Workspace allocation failed");
      return 1;
   }

   btVirtAddr pInputUsrVirt  = m_pMyApp->InputVirt();
   btVirtAddr pOutputUsrVirt = m_pMyApp->OutputVirt();

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   if ( 0 != ResetHandshake() ) {
      return 1;
   }

   ReadQLPCounters();
   SaveQLPCounters();

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
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   csr_type cfg = (csr_type)NLB_TEST_MODE_TRPUT|NLB_TEST_MODE_CONT;
   //m_pCCIAFU->CSRWrite(CSR_CFG, NLB_TEST_MODE_TRPUT|NLB_TEST_MODE_CONT);

   //Check for write through mode and add to CSR_CFG
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WT))
   {
     cfg |= (csr_type)NLB_TEST_MODE_WT;
   }

   m_pCCIAFU->CSRWrite(CSR_CFG, cfg);

   cout << endl;
   if ( flag_is_clr(cmd.cmdflags, NLB_CMD_FLAG_SUPPRESSHDR) ) {
			 //0123456789 0123456789 01234567890 012345678901 012345678901 0123456789012 0123456789012 0123456789 0123456789012
	  cout << "Cachelines Read_Count Write_Count Cache_Rd_Hit Cache_Wr_Hit Cache_Rd_Miss Cache_Wr_Miss   Eviction 'Ticks(@"
		   << Normalized(cmd)  << ")'";

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
   const Timer     absolute = Timer() + Timer(&ts);
#endif // OS

   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StopTimeoutMillis);
   while ( sz <= CL(cmd.endcls) )
      {

	   	   // Set the number of cache lines for the test
	      m_pCCIAFU->CSRWrite(CSR_NUM_LINES, (csr_type)(sz / CL(1)));

		   // Start the test
		   m_pCCIAFU->CSRWrite(CSR_CTL, 3);


		   // Wait for test completion
		   while ( ( 0 == pAFUDSM->test_complete ) &&
				   ( Timer() < absolute ) ) {

			   SleepNano(10);
		   }

		   // Stop the device
		   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

		   ReadQLPCounters();

		   PrintOutput(cmd, (sz / CL(1)));

		   SaveQLPCounters();
		   sz += CL(1);
       }

   while ( ( 0 == pAFUDSM->test_complete ) &&
           ( MaxPoll >= 0 ) ) {
      MaxPoll -= 500;
      SleepNano(500);
   }

   m_pCCIAFU->CSRWrite(CSR_CTL, 0);

   ReadQLPCounters();

   // Check the device status
   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for test stop was exceeded." << endl;
      ++res;
   }

   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   m_RdBw = CalcReadBandwidth(cmd);
   m_WrBw = CalcWriteBandwidth(cmd);

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

void  CNLBTrput::PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls)
{
	nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();
	bt64bitCSR ticks;
    bt64bitCSR rawticks     = pAFUDSM->num_clocks;
    bt32bitCSR startpenalty = pAFUDSM->start_overhead;
    bt32bitCSR endpenalty   = pAFUDSM->end_overhead;

	  cout << setw(10) << cls 					<< ' '
	       << setw(10) << pAFUDSM->num_reads    << ' '
	       << setw(11) << pAFUDSM->num_writes   << ' '
	       << setw(12) << GetQLPCounter(0)      << ' '
	       << setw(12) << GetQLPCounter(1)      << ' '
	       << setw(13) << GetQLPCounter(2)      << ' '
	       << setw(13) << GetQLPCounter(3)      << ' '
	       << setw(10) << GetQLPCounter(10)     << ' ';

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

