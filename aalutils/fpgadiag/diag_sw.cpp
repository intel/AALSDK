#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBSW::RunTest(const NLBCmdLine &cmd, btWSSize wssize)
{
	btInt res = 0;
	btWSSize  sz = CL(cmd.begincls);

	m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, 		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
	m_pCCIAFU->WorkspaceAllocate(wssize,       		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
    m_pCCIAFU->WorkspaceAllocate(wssize,       		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
    m_pCCIAFU->WorkspaceAllocate(MAX_UMSG_SIZE,		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_UMSG));

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

    if ( 0 != ResetHandshake() ) {
    	return 1;
	}

	if ( 0 != CacheCooldown(pOutputUsrVirt, m_pMyApp->OutputPhys(), m_pMyApp->OutputSize()) ) {
		return 1;
	}

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

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_DATA) || flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT))
   {
	   btUnsigned32bitInt umsg_base_cl = CACHELINE_ALIGNED_ADDR(m_pMyApp->UMsgPhys()) & 0xfffffffc;
	   m_pCCIAFU->CSRWrite(CSR_UMSG_BASE, (umsg_base_cl | 0x1));

	   btUnsigned32bitInt umsgmode_csr_val = (flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_UMSG_HINT)) ? HIGH : 0;
	   m_pCCIAFU->CSRWrite(CSR_UMSG_MODE, umsgmode_csr_val);
   }

   btCSRValue umsg_status = 0;
   do {
	   m_pCCIAFU->CSRRead(CSR_CIRBSTAT, &umsg_status);
   }while(0 == (umsg_status & 0x1));

   // Set the test mode
   m_pCCIAFU->CSRWrite(CSR_CFG, 0);
   csr_type cfg = (csr_type)NLB_TEST_MODE_MASK; //TODO Reconfirm this

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

   m_pCCIAFU->CSRWrite(CSR_CFG, (csr_type)cfg);

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
		  //0123456789 0123456789 01234567890 012345678901 012345678901 0123456789012 0123456789012 0123456789 0123456789012
   cout << "Cachelines Read_Count Write_Count Cache_Rd_Hit Cache_Wr_Hit Cache_Rd_Miss Cache_Wr_Miss   Eviction 'Ticks(@"
	    << Normalized(cmd)  << ")'";

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_BANDWIDTH) ) {
			  // 01234567890123 01234567890123
	 cout << "   Rd_Bandwidth   Wr_Bandwidth";
  }

  cout << endl;
  }

   while ( sz <= CL(cmd.endcls))
   {
	  // Set the number of cache lines for the test
	  m_pCCIAFU->CSRWrite(CSR_NUM_LINES, (csr_type)(sz / CL(1)));

	  // Start the test
	  m_pCCIAFU->CSRWrite(CSR_CTL, 3);

	  //Test flow
	  //1. CPU polls on Addr N+1
	  while(*(btUnsigned32bitInt *)(pOutputUsrVirt + sz) != HIGH)
	  {
		  if(Timer() > timeout )
		  {
			  res++;
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
		  m_pCCIAFU->CSRWrite(CSR_SW_NOTICE, 0x10101010);
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
	  timeout = Timer(&ts);
	  while ( ( 0 == pAFUDSM->test_complete ) &&
			   ( 0 == res) )
	  {
		  if(Timer() > timeout )
		  {
			  res++;
			  break;
		  }
	  }

	   // Stop the device
	   m_pCCIAFU->CSRWrite(CSR_CTL, 7);

	   ReadQLPCounters();

	   PrintOutput(cmd, (sz / CL(1)));

	   SaveQLPCounters();
	   sz += CL(1);

   }
   //Disable UMsgs upon test completion
   m_pCCIAFU->CSRWrite(CSR_UMSG_BASE, 0);
   while ( ( 0 == pAFUDSM->test_complete ) &&
		 ( MaxPoll >= 0 ) )
   {
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

   // Clean up..

     // Release the Workspaces
     m_pCCIAFU->WorkspaceFree(pInputUsrVirt,       TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
     m_pCCIAFU->WorkspaceFree(pOutputUsrVirt,      TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
     m_pCCIAFU->WorkspaceFree(pUMsgUsrVirt,        TransactionID((bt32bitInt)CMyCCIClient::WKSPC_UMSG));
     m_pCCIAFU->WorkspaceFree((btVirtAddr)pAFUDSM, TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));

     // Synchronize with the workspace free event notifications.
     m_pMyApp->ClientWait();

     if ( !m_pMyApp->ClientOK() ) {
        ERR("Workspace free failed");
        return 1;
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
