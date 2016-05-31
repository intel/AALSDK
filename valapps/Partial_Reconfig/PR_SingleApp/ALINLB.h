


#ifndef __ALI_NLB__
#define __ALI_NLB__

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>

#include <aalsdk/service/IALIAFU.h>

#include <string.h>

using namespace std;
using namespace AAL;


class AllocatesNLBService: public CAASBase, public IServiceClient
{
public:

   AllocatesNLBService();
   ~AllocatesNLBService();

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,TransactionID const &rTranID);
   void serviceAllocateFailed(const IEvent &rEvent);
   void serviceReleased(const AAL::TransactionID&);
   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
   void serviceReleaseFailed(const AAL::IEvent&);
   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   btBool FreeNLBService();
   btBool AllocateNLBService(Runtime *pRuntime);
   btInt  runInLoop();
   btInt  run();
   btID getErrnum() {  return m_errNum; }
   void setReleaseService(btBool releaseService)  { m_ReleaseService= releaseService; }
   static void NLBThread(OSLThread *pThread, void *pContext);

protected:
   Runtime       *m_pRuntime;           ///< AAL Runtime
   IBase         *m_pNLBService;       ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
   CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt          m_Result;            ///< Returned result value; 0 if success

   // Workspace info
   btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
   btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
   btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
   btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
   btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
   btWSSize       m_InputSize;      ///< Input workspace size in bytes.
   btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
   btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
   btWSSize       m_OutputSize;     ///< Output workspace size in bytes.
   btID           m_errNum;
   btBool         m_ReleaseService;
};

#endif
