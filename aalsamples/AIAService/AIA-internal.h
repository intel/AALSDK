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

#include <aalsdk/aas/AALService.h>                 // ServiceBase
#include <aalsdk/uaia/IAFUProxy.h>                 // AFUProxy

#include <aalsdk/INTCDefs.h>                       // AIA IDs

#include "UIDriverInterfaceAdapter.h"              // UIDriverInterfaceAdapter
#include "AIATransactions.h"

/// @todo Document uAIA and related.

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

//============================================================================
// AAL Service Client
//============================================================================
class AFUProxyCallback : public IDispatchable
{
public:

   AFUProxyCallback(IAFUProxyClient          *pClient,
                    AAL::IEvent const        *pEvent) :
   m_pClient(pClient),
   m_pEvent(pEvent)
   {
      ASSERT(NULL != pClient);
      ASSERT(NULL != pEvent);
   }


void operator() ()
{
   // Process TransactionID
   const TransactionID &msgTid = dynamic_cast<const IUIDriverEvent*>(evtUIDriverClientEvent, m_pEvent)->msgTranID();
   if (msgTid.Filter() && msgTid.Handler() != NULL) {
      msgTid.Handler()(*m_pEvent);
   } else {
      m_pClient->AFUEvent(*m_pEvent);
   }
   delete m_pEvent;
   delete this;
}

virtual ~AFUProxyCallback() {}

protected:
   IAFUProxyClient         *m_pClient;
   AAL::IEvent const       *m_pEvent;
};


//=============================================================================
// Name: AIAService
// Description: Implementation of the AFU Interface Adapter Service
// Comments:
//=============================================================================
class UAIA_API AIAService: public AAL::ServiceBase, public AAL::IServiceClient
{
   public:

      DECLARE_AAL_SERVICE_CONSTRUCTOR(AIAService, AAL::ServiceBase),
         m_uida(),
         m_Semaphore(),
         m_pMDT(NULL),
         m_pShutdownThread(NULL),
         m_state(Uninitialized)
      {
         SetInterface(iidAIAService, dynamic_cast <AIAService *>(this));
         if(!m_Semaphore.Create(1)){
            return;
         }
         m_bIsOK = false;
      }

      virtual ~AIAService();

      // <IAALService>
      btBool Release( AAL::TransactionID const &rTranID,
                      AAL::btTime timeout = AAL_INFINITE_WAIT);

      // Hook to allow the object to initialize
      btBool init( AAL::IBase *pclientBase,
                   AAL::NamedValueSet const &optArgs,
                   AAL::TransactionID const &rtid);
      // </IAALService>
// TODO THESE COULD BE MADE INTO AN IAIA  SO THAT THE PROXY DOES NOT SEE Release() METHOD ABOVE
      void AFUProxyRelease(AAL::IBase *pAFUProxy);

      void AFUProxyAdd(AAL::IBase *pAFUProxy);

      void SendMessage(AAL::btHANDLE devhandle, IAIATransaction *pMessage, IAFUProxyClient *pClient);

      AAL::btBool MapWSID(AAL::btWSSize Size, AAL::btWSID wsid, AAL::btVirtAddr *pRet);
      void UnMapWSID(AAL::btVirtAddr ptr, AAL::btWSSize Size);


   protected:
      void SemWait(void);
      void SemPost(void);

      void Process_Event();

      static void MessageDeliveryThread(OSLThread *pThread,
                                        void *pContext);

      void WaitForShutdown(TransactionID const &rtid,
                           btTime timeout);


      // <IServiceClient> - Used only for serviceReleased and only so we can trap the Releases without
      //                    them going to real client
      void serviceAllocated(IBase *pServiceBase,TransactionID const &rTranID = TransactionID()){};
      void serviceAllocateFailed(const IEvent &rEvent){};
      void serviceReleased(TransactionID const &rTranID = TransactionID());
      void serviceReleaseFailed(const IEvent &rEvent);
      void serviceEvent(const IEvent &rEvent){};
       // </IServiceClient>

      void AFUProxyGet( IBase *pServiceClient,
                        NamedValueSet const &Args,
                        TransactionID const &rtid);         // Allocates a Proxy to the AFU

      btBool AFUListAdd(IBase *pAFU);
      btBool AFUListDel(IBase *pDev);

   protected:

      class ShutdownDisp : public IDispatchable
      {
      public:
         ShutdownDisp(AIAService *pAIA, btTime time, TransactionID const &tid);
         void operator() ();
         void ReleaseChildren(TransactionID const &tid);
      private:
         AIAService                 *m_pAIA;
         AAL::btTime                 m_timeout;
         AAL::TransactionID const   &m_tid;
      };


      // Variables
      UIDriverInterfaceAdapter   m_uida;                                         // Kernel Mode Driver Interface Adapter
      CSemaphore                 m_Semaphore;                                    // General synchronization as needed
      OSLThread                 *m_pMDT;                                         // Message delivery thread
      OSLThread                 *m_pShutdownThread;                              // Shutdown thread

      typedef std::list<IBase *>          AFUList;
      typedef AFUList::iterator           AFUList_itr;
      typedef AFUList::const_iterator     AFUList_citr;

      AFUList                    m_mAFUList;                                     // Map of Runtime Proxys

      enum state {
         Uninitialized,
         Initialized,
         Shuttingdown
      };

      enum state                 m_state;
};


#endif // __AALSDK_AIA_INTERNAL_H__

