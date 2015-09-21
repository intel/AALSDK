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
/// @file AIA.h
/// @brief Defines Service for the User Mode Application Interface Adapter
///        (AIA).
/// @ingroup AIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 8/21/2015      JG       Initial version
//****************************************************************************
#ifndef __AALSDK_AIA_INTERNAL_H__
#define __AALSDK_AIA_INTERNAL_H__
//#include <aalsdk/AALTypes.h>
//#include <aalsdk/AALTransactionID.h>
//#include <aalsdk/osal/OSSemaphore.h>
//#include <aalsdk/osal/CriticalSection.h>
//#include <aalsdk/osal/Thread.h>
//#include <aalsdk/AALBase.h>                        // IBase
//#include <aalsdk/aas/AALServiceModule.h>
//#include <aalsdk/aas/AALService.h>                 // ServiceBase
//#include <aalsdk/uaia/AIA.h>                       // AIA interfaces
//#include <aalsdk/uaia/AALuAIA_Messaging.h>         // UIDriverClient_uidrvManip, UIDriverClient_uidrvMarshaler_t
//#include <aalsdk/uaia/uAIASession.h>               // uAIASession
#include <aalsdk/aas/AALService.h>                 // ServiceBase
#include <aalsdk/uaia/IAFUProxy.h>                 // AFUProxy

#include <aalsdk/INTCDefs.h>                       // AIA IDs

#include "UIDriverInterfaceAdapter.h"              // UIDriverInterfaceAdapter
#include "AIATransactions.h"

/// @todo Document uAIA and related.

class DeviceServiceBase;

USING_NAMESPACE(AAL);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////               AIAService                  ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
#define AIA_SERVICE_BASE_INTERFACE "AIA_Service_Base_Interface"

//=============================================================================
// Name: AIAService
// Description: Implementation of the AFU Interface Adapter Service
// Comments:
//=============================================================================
class UAIA_API AIAService: public AAL::ServiceBase//, public AAL::IServiceClient
{
   public:

      DECLARE_AAL_SERVICE_CONSTRUCTOR(AIAService, AAL::ServiceBase),
         m_uida(),
         m_Semaphore(),
         m_pMDT(NULL),
         m_pShutdownThread(NULL)
      {
         SetInterface(iidAIAService, dynamic_cast <AIAService *>(this));
         m_bIsOK = false;
      }

      virtual ~AIAService();

      // <IAALService>
      btBool Release(TransactionID const &rTranID,
                     btTime timeout = AAL_INFINITE_WAIT);

      // Hook to allow the object to initialize
      btBool init( IBase *pclientBase,
                   NamedValueSet const &optArgs,
                   TransactionID const &rtid);
      // </IAALService>
// TODO THESE COULD BE MADE INTO AN IAIA  SO THAT THE PROXY DOES NOT SEE Release() METHOD ABOVE
      void AFUProxyRelease(IBase *pAFUProxy);

      void AFUProxyAdd(IBase *pAFUProxy);

      void SendMessage(AAL::btHANDLE devhandle, IAIATransaction *pMessage, IAFUProxyClient *pClient);

   protected:
      void SemWait(void);
      void SemPost(void);

      void Destroy(void);



      void Process_Event();

      static void MessageDeliveryThread(OSLThread *pThread,
                                        void *pContext);
      static void ShutdownThread(OSLThread *pThread,
                                 void *pContext);
      void WaitForShutdown(ui_shutdownreason_e      reason,
                           btTime                   waittime,
                           stTransactionID_t const &rTranID_t);
#if 0
      // <IServiceClient> - Used to Get Proc
      void serviceAllocated(IBase *pServiceBase,TransactionID const &rTranID = TransactionID());
      void serviceAllocateFailed(const IEvent &rEvent);
      void serviceReleased(TransactionID const &rTranID = TransactionID());
      void serviceReleaseFailed(const IEvent &rEvent);
      void serviceEvent(const IEvent &rEvent);
       // </IServiceClient>
#endif
      void AFUProxyGet( IBase *pServiceClient,
                        NamedValueSet const &Args,
                        TransactionID const &rtid);         // Allocates a Proxy to the AFU

      btBool AFUListAdd(IBase *pAFU);
      btBool AFUListDel(IBase *pDev);

      btBool IssueShutdownMessageWorker(stTransactionID_t const &rTranID_t,
                                        btTime                   timeout);

   protected:
      typedef std::list<IBase *>    AFUList;
      typedef AFUList::iterator     AFUList_itr;

      // Variables
      UIDriverInterfaceAdapter   m_uida;                                         // Kernel Mode Driver Interface Adapter
      CSemaphore                 m_Semaphore;                                    // General synchronization as needed
      OSLThread                 *m_pMDT;                                         // Message delivery thread
      OSLThread                 *m_pShutdownThread;                              // Shutdown thread
      AFUList                    m_mAFUList;                                     // Map of Runtime Proxys
};


#endif // __AALSDK_AIA_INTERNAL_H__

