#include "diag_defaults.h"
#include "diag-common.h"
#include "nlb-specific.h"
#include "diag-nlb-common.h"

btInt CNLBRead::RunTest(const NLBCmdLine &cmd, btWSSize wssize)
{
	btInt res = 0;


	m_pCCIAFU->WorkspaceAllocate(NLB_DSM_SIZE, 		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_DSM));
	m_pCCIAFU->WorkspaceAllocate(wssize,       		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_IN));
    m_pCCIAFU->WorkspaceAllocate(wssize,       		TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));
    //m_pCCIAFU->WorkspaceAllocate(MAX_UMSG_WKSPC,    TransactionID((bt32bitInt)CMyCCIClient::WKSPC_OUT));

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
      //volatile btVirtAddr pUMsgUsrVirt = m_pMyApp->UMsgVirt();
      return res;
}
