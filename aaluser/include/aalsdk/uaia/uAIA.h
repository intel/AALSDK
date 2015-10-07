// Copyright (c) 2007-2015, Intel Corporation
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
/// @file uAIA.h
/// @brief Defines Service for the Universal AIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Alvin Chen, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/25/2008     JG       Added the CAFUDev implementation
/// 11/30/2008     HM       Reformatted (by machine - messed up some formatting),
///                            added AFUTransaction types, modified AFUTransaction
///                            interface definition
/// 12/01/2008     HM       Moved FAP10-specific material to FAP10.h.
///                         Reset Namespace usage to be definition
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2009     HM       Modified signature of UnBind
/// 06/22/2009     JG       Massive changes to support new proxy mechanism and
///                            to fix build dependencies that required external
///                            linking to the module, breaking plug-in model.
/// 07/06/2009     HM/JG    Fixed double-destruct of uAIA. Refined uAIA shut-
///                            down code a bit more.
///                         Added IssueShutdownMessageWorker().
/// 07/20/2009     AC       Added 'Create', and remove 'm_UIDC' to fix a bug for
///							shutdown(SystemStop) and re-start( SystemInit) in a single
///							process.
/// 06/02/2010     JG       Modified AIA for asynchronous shutdown of clients.
///                            prior the MDP was shutdown first preventing
///                            AFUs from cleaning up.
///                         Added support for a default handler.
/// 06/02/2011     JG       Added NamedValueSet to Initialize for Service 2.0
/// 08/30/2011     JG       Ported to Service 2.0
///                         Removed IAIA and IServiceProvider
///                         Removed Proxys
/// 10/26/2011     JG       Added CAIA proxy@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_UAIA_H__
#define __AALSDK_UAIA_UAIA_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/AALBase.h>                     // IBase
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/aas/AALService.h>              // ServiceBase
#include <aalsdk/uaia/AIA.h>                    // IProvisioning
#include <aalsdk/uaia/AALuAIA_Messaging.h>      // UIDriverClient_uidrvManip, UIDriverClient_uidrvMarshaler_t
#include <aalsdk/uaia/uAIASession.h>            // uAIASession
#include <aalsdk/uaia/AALuAIA_UIDriverClient.h> // UIDriverClient
#include <aalsdk/aas/AALService.h>              // ServiceBase


/// @todo Document uAIA and related.

BEGIN_NAMESPACE(AAL)

class DeviceServiceBase;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                                     uAIA
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------



//=============================================================================
// Name: uAIA
// Description: Implementation of that AAL 1.0 Universal AIA Service
// Comments: Exported methods must be declared virtual so as to enable call
//           through vtable.
//=============================================================================
class UAIA_API uAIA : public ServiceBase
{
public:

   DECLARE_AAL_SERVICE_CONSTRUCTOR(uAIA, ServiceBase),
      m_pUIDC(NULL),
      m_Semaphore(),
      m_pMDT(NULL),
      m_pShutdownThread(NULL),
      m_IProvisioning(NULL),
      m_refcount(0),
      m_pShutdownSem(NULL),
      m_quietRelease(true)
   {
      SetInterface(iidAIA_uAIA, dynamic_cast <uAIA *>(this));
      m_bIsOK = false;
   }

   virtual ~uAIA();

   // IAALService
   btBool Release(TransactionID const &rTranID,
                  btTime timeout = AAL_INFINITE_WAIT);
   // Quite release
   btBool Delete(btTime timeout = AAL_INFINITE_WAIT);

   void   Destroy(void);

   // Hook to allow the object to initialize
   btBool init(IBase *pclientBase,
               NamedValueSet const &optArgs,
               TransactionID const &rtid);


   // Shutdown methods
   void   WaitForShutdown(ui_shutdownreason_e      reason,
                          btTime                   timeout,
                          stTransactionID_t const &rTranID_t);
   btBool IssueShutdownMessageWorker(stTransactionID_t const &rTranID_t,
                                     btTime                   timeout = 0);

   // Accessor to UIDC
   btBool MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet);
   void UnMapWSID(btVirtAddr ptr, btWSSize Size);

   void SendMessage(UIDriverClient_uidrvManip fncObj);

   UIDriverClient_uidrvMarshaler_t GetMarshaller();

protected:
   void SemWait() { m_Semaphore.Wait();  }
   void SemPost() { m_Semaphore.Post(1); }

   void Process_Event();
   static void MessageDeliveryThread(OSLThread *pThread, void *pContext);

   // Used by ShutdownThread
   struct shutdownparms_s
   {
	   shutdownparms_s(uAIA *This, btTime time, TransactionID const &tid) :
         pAIA(This),
         shutdowntime(time),
         tid_t(stTransactionID_t(tid))
      {}

      uAIA              *pAIA;
      btTime             shutdowntime;        // Timeout for shutdown
      stTransactionID_t  tid_t;
   };

   static void ShutdownThread(OSLThread *pThread, void *pContext);

   UIDriverClient      *m_pUIDC;               // UI Driver Client

   CSemaphore           m_Semaphore;           // General synchronization as needed
   OSLThread           *m_pMDT;                // Message delivery thread
   OSLThread           *m_pShutdownThread;     // Shutdown thread
   IProvisioning       *m_IProvisioning;
   btInt                m_refcount;

   btEventHandler       m_Listener;
   btApplicationContext m_ListenerContext;

   CSemaphore          *m_pShutdownSem;        // Used for to count resources during shutdown
   btBool               m_quietRelease;        // Used when service is realeased
};


//=============================================================================
// Name: CAIA
// Description: AIA Proxy to the singleton uAIA
// Comments:
//=============================================================================
class UAIA_API CAIA : public ServiceBase
{
public:
   DECLARE_AAL_SERVICE_CONSTRUCTOR(CAIA, ServiceBase), m_AIA(NULL)
   {
      SetInterface(iidAIA, dynamic_cast <CAIA *>(this));
      m_bIsOK = false;
   }

   virtual ~CAIA();

   UIDriverClient_uidrvMarshaler_t GetMarshaller() { return m_AIA->GetMarshaller(); }

   // Accessor to UIDC
   btBool MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet) { return m_AIA->MapWSID(Size, wsid, pRet); }
   void UnMapWSID(btVirtAddr ptr, btWSSize Size)                { m_AIA->UnMapWSID(ptr, Size); }
   void SendMessage(UIDriverClient_uidrvManip fncObj)           { m_AIA->SendMessage(fncObj);  }
   btBool init( IBase *pclientBase,
                NamedValueSet const &optArgs,
                TransactionID const &rtid)
   {
      initComplete(rtid);
   }

   // IService
   btBool Release(TransactionID const &rTranID,
                  btTime timeout=AAL_INFINITE_WAIT) { return true; }
   // Quiet release
   btBool Delete(btTime timeout=AAL_INFINITE_WAIT) {getRuntime()->releaseRuntimeProxy(); delete this; return true; }

   void SetuAIA(uAIA *puAIA)
   {
      m_AIA   = puAIA;
      m_bIsOK = true;
   }

   uAIASession * CreateAIASession(IBase                *pOwnerBase,
                                  DeviceServiceBase    *pDevService,
                                  btEventHandler        EventHandler,
                                  ServiceBase          *pServiceBase);
   void DestroyAIASession(uAIASession *pSess);

private :
   uAIA *m_AIA;
};


END_NAMESPACE(AAL)

#endif // __AALSDK_UAIA_UAIA_H__

