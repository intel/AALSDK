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
/// @file uAIA.cpp
/// @brief Universal Application Interface Adaptor.
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
/// 12/02/2008     HM       Moved using namespaces above
///                            DEFINE_SERVICE_FACTORY_ACCESSOR(uAIA) because
///                            of Namespacing change in uAIA.h
/// 12/10/2008     HM       Added Logger
/// 12/14/2008     HM       Added result_code accessor / mutator
/// 01/04/2009     HM       Updated Copyright
/// 02/01/2009     HM       Put "Returned" in Logger
/// 02/14/2009     HM       Modified UnBind, and expanded MessageHandler
/// 02/20/2009     HM       Added AALLogger messages to
///                            uAIAMangement::MessageHandler
/// 05/18/2009     HM       Process_Event() now forwards messages with error
///                            codes on up the stack
/// 06/06/2009     HM       Message pump shutdown
/// 06/07/2009     HM       Calls to AALLogger in ~uAIA currently execute after
///                            exit() from main(). By that time the Logger is
///                            gone, segfault. Removed Logging calls from dtor.
///                         Will reinstate once SystemStop() correctly shuts
///                            down uAIA before exiting.
/// 06/22/2009     JG       Massive changes to support new proxy mechanism and
///                            to fix build dependencies that required external
///                            linking to the module, breaking plug-in model.
/// 07/06/2009     HM/JG    Fixed double-destruction of uAIA. Refined uAIA shut-
///                            down code a bit more.
///                         Added IssueShutdownMessageWorker().
/// 07/15/2009     HM       Instrumented Shutdown code
/// 07/16/2009     HM       Fixed a bug for Shutdown more than once in the same process
/// 07/31/2009     AC       Fixed a leakage issue of the UIDriverClientEvent
/// 08/16/2009     HM       Added debug statements to BindProxy() and Release()
/// 08/16/2009     HM       Backed down the debugs from DEBUG to VERBOSE
/// 06/02/2010     JG       Modified AIA for asynchronous shutdown of clients.
///                            prior the MDP was shutdown first preventing
///                            AFUs from cleaning up.
///                         Added support for a default handler.
/// 06/02/2011     JG       Added NamedValueSet to Initialize for Service 2.0
/// 09/01/2011     JG       Redesigned AIA to use Service 2.0 framework.
///                         Eliminated Proxys.  Simplified class hierarchy.
/// 10/26/2011     JG       Added back proxy in the form of CAIA to properly
///                           handle singletons with cotext
/// 01/16/2012     JG       Fixed init() for latest SDK change@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"
#include "aalsdk/uaia/uAIA.h"

#include "aalsdk/AALTransactionID.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/uaia/FAPPIP_AFUdev.h"
#include "aalsdk/AALLoggerExtern.h"

#include "aalsdk/kernel/KernelStructs.h"
#include "aalsdk/aas/AALInProcServiceFactory.h"
#include "aalsdk/uaia/uAIAService.h"


//========================================
// Implements the AIA entry point for the
// load and bind operation
//========================================

#define SERVICE_FACTORY AAL::InProcSingletonSvcsFact< AAL::uAIA >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AASUAIA_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
AASUAIA_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)


BEGIN_C_DECLS
extern UIDriverClient & msgMarshaller(UIDriverClient & ,
                                      uid_msgIDs_e ,
                                      UIDriverClient_msgPayload ,
                                      stTransactionID_t const * ,
                                      uidrvMessageRoute * ,
                                      btObjectType );
END_C_DECLS


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                               uAIAManagement
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          uAIA MANAGEMENT INTERFACE        ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              uAIA INTERFACE               ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: UIDriverClientEvent
// Description: Constructor
// Interface: public
// Inputs: - Class name.
// Outputs: Pointer to factory.
// Comments:
//=============================================================================
class UIDriverClientEvent : public IUIDriverClientEvent,
                            public CTransactionEvent
{
public:
   UIDriverClientEvent( IBase *pObject,
                        uidrvMessage *pmessage,
                        TransactionID const &TransID)
   : CTransactionEvent(pObject,TransID),
     m_pmessage(pmessage)
   {
      SetSubClassInterface(tranevtUIDriverClientEvent,
                            dynamic_cast<IUIDriverClientEvent*>(this));

   }

   void                     *DevHandle()  const { return m_pmessage->handle();   }
   uid_msgIDs_e              MessageID()  const { return m_pmessage->id();       }
   btVirtAddr                Payload()    const { return m_pmessage->payload();  }
   btWSSize                  PayloadLen() const { return m_pmessage->size();     }
   stTransactionID_t const & msgTranID()  const { return m_pmessage->tranID();   }
   uidrvMessageRoute const & msgRoute()   const { return m_pmessage->msgRoute(); }

   // HM 20081213
   uid_errnum_e              ResultCode() const { return m_pmessage->result_code(); }
   void                      ResultCode(uid_errnum_e e) { return m_pmessage->result_code(e); }


   virtual ~UIDriverClientEvent() { if(m_pmessage != NULL) delete m_pmessage; }

private:
   // No copying allowed
   UIDriverClientEvent(const UIDriverClientEvent & );
   UIDriverClientEvent & operator = (const UIDriverClientEvent & );

protected:
   uidrvMessage *m_pmessage;

   UIDriverClientEvent();
 };

UIDriverClientEvent::UIDriverClientEvent() {/*empty*/}
UIDriverClientEvent::UIDriverClientEvent(const UIDriverClientEvent & ) {/*empty*/}
UIDriverClientEvent & UIDriverClientEvent::operator = (const UIDriverClientEvent & ) { return *this; }


//=============================================================================
// Name: init()
// Description: Initialize the service
// Interface: public
// Inputs: Optional Trsnaction ID
// Outputs: none.
// Comments:  This function may be called more than once per process but the
//            uAIA and its UIDriverClient are singletons per process.
//=============================================================================
void uAIA::init( TransactionID const& rtid )
{
   AAL_INFO(LM_UAIA, "uAIA::Create. in\n");

   CAIA *pCAIA = new CAIA(m_pcontainer);

   pCAIA->SetuAIA(this);

   //pCAIA->_init(Handler(), Context(), rtid, OptArgs());
   pCAIA->init(rtid);

   //Singleton service already initialized
   if(!m_bIsOK){

      m_pUIDC = new UIDriverClient;

      // Create a channel to the UI Driver now
      m_pUIDC->Open();
      if (!m_pUIDC->IsOK()) {
         m_bIsOK = false;
         QueueAASEvent(new ObjectCreatedExceptionEvent( getRuntimeClient(),
                                                        Client(),
                                                        dynamic_cast<IBase*>(this),
                                                        rtid,
                                                        errCreationFailure,
                                                        reasCauseUnknown,
                                                        "Failed to open UI Driver"));

         return;
      }

      // Create the Message delivery thread
      m_pMDT = new OSLThread(uAIA::MessageDeliveryThread,
                             OSLThread::THREADPRIORITY_NORMAL,
                             this);

      // Make sure that the kernel pipe to the database is open.
      //  The Wait is posted in the uAIA:MessageDeliveryThread
      //  indicating that it has started
      SemWait();
      m_bIsOK = true;
      AAL_INFO(LM_UAIA, "uAIA::Create, out\n");
   }
   // Create the object
   QueueAASEvent(new ObjectCreatedEvent(getRuntimeClient(),
                                        Client(),
                                        dynamic_cast<IBase*>(pCAIA),rtid));
   return;
}

//=============================================================================
// Name: ~uAIA()
// Description: Universal AIA class. Singleton service that supports multiple
//              instances of AIA Proxies.
// Interface: public
// Outputs: none.
//=============================================================================
uAIA::~uAIA(void)
{
   std::cout << "AIA Gone" << std::endl;
   Destroy();
}

//=============================================================================
// Name: Destroy()
// Description: Method invoked for destroying the AIA prior to unload,
// Interface: public
// Outputs: none.
//=============================================================================
void uAIA::Destroy(void)
{
   // Shutdown the message pump by putting in a reqid_Shutdown to force a
   //    wake-up and let the kernel know it is going down.

   char szFunc[] = "uAIA::Destroy";
//   IssueShutdownMessageWorker( szFunc);

   // if message pump thread is running, need to wait for it to terminate
   if ( m_pMDT ) {

      DEBUG_CERR(szFunc << ": waiting for Receive Thread to Join. 1 of 2\n");
      AAL_DEBUG(LM_Shutdown, szFunc << ": waiting for Receive Thread to Join. 1 of 2\n");

      // Wait for the Message delivery thread to terminate
      m_pMDT->Join();

      DEBUG_CERR(szFunc << ": Receive Thread has Joined. 2 of 2\n");
      AAL_DEBUG(LM_Shutdown, szFunc << ": Receive Thread has Joined. 2 of 2\n");

      delete m_pMDT;
      m_pMDT = NULL;
      delete m_pUIDC;
   }
   m_bIsOK = false;

}  // uAIA::~uAIA


//=============================================================================
// Name: SendMessage()
// Description: Send a message down the UIDC
// Interface: public
// Outputs: none.
//=============================================================================
void uAIA::SendMessage(UIDriverClient_uidrvManip fncObj)
{
   (*m_pUIDC) << fncObj;

}


//=============================================================================
// Name: GetMarshaller()
// Description: Get a pointer to the message marshaller
// Interface: public
// Outputs: none.
//=============================================================================
UIDriverClient_uidrvMarshaler_t uAIA::GetMarshaller()
{
   AutoLock(this);
   return msgMarshaller;
}

//=============================================================================
// Name: MessageDeliveryThread
// Description: Message Delivery Thread
// Interface: public
// Inputs: pThread - thread object
//         pContext - context
// Outputs: none.
// Comments: Opens rmc device an dispatches events
//=============================================================================
void uAIA::MessageDeliveryThread(OSLThread *pThread,
                                 void *pContext)
{
   //Get a pointer to this objects context
   uAIA *This = (uAIA*)pContext;

   This->m_bIsOK=true;

   // Wake up the ctor
   This->SemPost();

   // Run the message pump
   This->Process_Event();

   // Message pump exited, so uAIA shutting down. Signal that by setting flag.
   This->m_pUIDC->IsOK(false);

   // cerr << "uAIA::~uAIA: shutting down UI Client file\n";
   AAL_DEBUG(LM_UAIA,"uAIA::MessageDeliveryThread: shutting down UI Client file\n");

   // Close the channel
   This->m_pUIDC->Close();

   // cout << "uAIA::~uAIA: done\n";
   AAL_INFO(LM_UAIA, "uAIA::MessageDeliveryThread: done\n");

   // Final release
   This->Released();

}  // uAIA::MessageDeliveryThread

//=============================================================================
// Name: MapWSID
// Description: Map a workspace ID
// Interface: protected
// Inputs: none
// Outputs: none.
// Comments: Processes all events until none are left
//=============================================================================
btBool uAIA::MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet)
{
   return m_pUIDC->MapWSID(Size, wsid, pRet);
}

//=============================================================================
// Name: UnMapWSID
// Description: Unmap a workspace ID
// Interface: protected
// Inputs: none
// Outputs: none.
// Comments: Processes all events until none are left
//=============================================================================
void uAIA::UnMapWSID(btVirtAddr ptr, btWSSize Size)
{
   m_pUIDC->UnMapWSID(ptr, Size);
}



//=============================================================================
// Name: Process_Event
// Description: Process RMS event
// Interface: protected
// Inputs: none
// Outputs: none.
// Comments: Processes all events until none are left
//=============================================================================
void
uAIA::Process_Event()
{

   uidrvMessage *pMessage = new uidrvMessage;
   AAL_INFO(LM_UAIA, "uAIA::Process_Event. in\n");

   while (m_pUIDC->GetMessage(pMessage) != false) {
      AAL_DEBUG(LM_UAIA, "uAIA::Process_Event: GetMessage Returned\n");
      if (pMessage->result_code() != uid_errnumOK) {
         AAL_WARNING(LM_UAIA, "uAIA::Process_Event: pMessage->result_code() is not uid_errnumOK, but is " <<
                  pMessage->result_code() << std::endl);
      }

      if (rspid_UID_Shutdown == pMessage->id()) { // Are we done?
         // Important to Lock here to make sure the m+pShutdownThread has been set
         Lock();
         AAL_INFO(LM_UAIA, "uAIA::Process_Event: Shutdown Seen\n");
         //Shutdown complete
         if(NULL == m_pShutdownThread) {
            std::cerr << "NULL THREAD" << std::endl;
         }else{
            m_pShutdownThread->Join();
         }
         Unlock();
         delete pMessage;
         return;
      }else {

         // Generate the event - No need to destroy message as it being passed to event and will be
         // destroyed there.  TODO - Consider cleaner memory management - Try to avoid copies
         uidrvMessageRoute const &msgroute = pMessage->msgRoute();
         IBase *proxybase = pMessage->msgRoute().AIAProxybasep();
         btEventHandler handler = pMessage->msgRoute().Handler();
         TransactionID tranID = pMessage->tranID();
         QueueAASEvent(handler,
                                 new UIDriverClientEvent(proxybase,
                                                                pMessage,
                                                                tranID));
/*
         QueueAASEvent(pMessage->msgRoute().Handler(),
                                        new UIDriverClientEvent(dynamic_cast <IBase*> (pMessage->msgRoute().AIAProxybasep()),
                                                                pMessage,
                                                                pMessage->tranID()));
*/
         pMessage = new uidrvMessage;

      }
   } // while()

   // catastrophic failure.  try to clean up after ourself.
   delete pMessage;
} // uAIA::Process_Event


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////       uAIA SERVICE PROVIDER INTERFACE     ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//=============================================================================
// Name: ShutdownThread
// Description: Thread responsible for ordered shutdown
// Interface: public
// Inputs: pThread - thread object
//         pContext - context
// Outputs: none.
// Comments: Opens rmc device an dispatches events
//=============================================================================
void uAIA::ShutdownThread(OSLThread *pThread,
                          void *pContext)
{
   struct shutdownparms_s *pParms = static_cast<struct shutdownparms_s*>(pContext);

   pParms->pAIA->WaitForShutdown( ui_shutdownReasonNormal,
                                  pParms->shutdowntime,
                                  pParms->tid_t);

   delete pParms;

}


//=============================================================================
// Name: WaitForShutdown()
// Description: Waits for all resources to release the AIA.
// Inputs: reason - Reason the service was brought down
//         waittime - amount of time to wait
// Interface: public
// Outputs: none.
//=============================================================================
void uAIA::WaitForShutdown(ui_shutdownreason_e      reason,
                           btTime                   waittime,
                           stTransactionID_t const &rTranID_t)
{
   btBool notimeout=true;

   CAALEvent * pTheEvent=NULL;

   Lock();


   DEBUG_CERR("uAIA::WaitForShutdown() refcount is " << m_refcount << ". 1 of 3\n");
   AAL_DEBUG(LM_Shutdown,"uAIA::WaitForShutdown() refcount is " << m_refcount << ". 1 of 3\n");

   // If the refcount is not zero setup a count up
   // semaphore to wait until all resources release
   if(m_refcount){
      DEBUG_CERR("uAIA::WaitForShutdown() refcount is non-zero. 2 of 3\n");
      AAL_DEBUG(LM_Shutdown,"uAIA::WaitForShutdown() refcount is non-zero. 2 of 3\n");

      m_pShutdownSem = new CSemaphore();
      // Negative count means the semaphore counts up
      m_pShutdownSem->Create(-m_refcount, 1);
      Unlock();

      // The semaphore returns true if it unblocked with no timeout
      if ( AAL_INFINITE_WAIT == waittime ) {
         notimeout = m_pShutdownSem->Wait();
      }else{
         notimeout = m_pShutdownSem->Wait(waittime);
      }
      Lock();
      delete m_pShutdownSem; m_pShutdownSem=NULL;
      Unlock();

   } else {
      DEBUG_CERR("uAIA::WaitForShutdown() refcount was 0. 2 of 3\n");
      AAL_DEBUG(LM_Shutdown,"uAIA::WaitForShutdown() refcount was 0. 2 of 3\n");
      Unlock();   // match the initial Lock
   }

   // Generate an event if the shutdown was called by the quite version of Release()
   //  This is the normal case for this service.  This code may be capable of being
   //  simplified as the event version is no longer used. It remains implemented as the
   //  IAALService interface musst define both types of Release()
   if ( !m_quietRelease ) {
      // Send the event
      if(true == notimeout){
         pTheEvent = new CTransactionEvent(dynamic_cast<IBase*>(this),
                                           tranevtServiceShutdown,
                                           TransactionID(rTranID_t));
      }else{
           // Timeout event
         pTheEvent= new CExceptionTransactionEvent(dynamic_cast<IBase*>(this),
                                                exttranevtServiceShutdown,
                                                TransactionID(rTranID_t),
                                                errSystemTimeout,
                                                reasSystemTimeout,
                                                const_cast<btString>(strSystemTimeout));
      }

      if(pTheEvent){
         //Shutdown complete
         QueueAASEvent(Handler(), pTheEvent);
         DEBUG_CERR("uAIA::WaitForShutdown() exiting. 3 of 3\n");
         AAL_DEBUG(LM_Shutdown,"uAIA::WaitForShutdown() exiting. 3 of 3\n");
      }else{
         DEBUG_CERR("uAIA::WaitForShutdown() FATAL ERROR. CANNOT CREATE EVENT\n");
         AAL_DEBUG(LM_Shutdown,"uAIA::WaitForShutdown() FATAL ERROR. CANNOT CREATE EVENT\n");
      }
   }

   // TODO timeout should be adjusted for above waitimes elapsed
   IssueShutdownMessageWorker(rTranID_t, waittime );
   return;
}

//=============================================================================
// Name:        IssueShutdownMessageWorker()
// Description: Worker function called from several places. Creates and sends
//                 shutdown message to the kernel
// Interface:   public
// Inputs:      rTrabID_t - reference to a transaction ID structure
//              timeout - timeout to wait
// Outputs:     none
// Comments:    Encapsulates the fields of a reqid_UID_Shutdown message
//=============================================================================
btBool uAIA::IssueShutdownMessageWorker(stTransactionID_t const &rTranID_t,
                                        btTime                   timeout)
{
   if (m_pUIDC->IsOK()) {
      DEBUG_CERR("IssueShutdownMessageWorker: sending reqid_UID_Shutdown via IssueShutdownMessageWorker. 1 of 1\n");
      AAL_DEBUG(LM_Shutdown,"IssueShutdownMessageWorker: sending reqid_UID_Shutdown via IssueShutdownMessageWorker. 1 of 1\n");

      struct big {
         struct aalui_ioctlreq  req;
         struct aalui_Shutdown  shutdown;
      };
      struct big message;
      memset( &message, 0, sizeof( message));
      message.req.id = reqid_UID_Shutdown;
      message.req.tranID = rTranID_t;

      message.req.size           = sizeof( message.shutdown );
      message.shutdown.m_reason  = ui_shutdownReasonNormal;
      message.shutdown.m_timeout = timeout;

      m_pUIDC->SendMessage( AALUID_IOCTL_SENDMSG, &message.req);
      return true;
   }
   else {
      DEBUG_CERR("IssueShutdownMessageWorker: no reqid_UID_Shutdown sent, UIDC already gone. 1 of 1\n");
      AAL_DEBUG(LM_Shutdown, "IssueShutdownMessageWorker: no reqid_UID_Shutdown sent, UIDC already gone. 1 of 1\n");
      return false;
   }
}  // IssueShutdownMessageWorker



//=============================================================================
// Name: Release
// Description: Shutdown the AIA. This should only be called by the factory
// Interface: public
// Inputs: rTranID - transaction ID
//         timeout - max timeout in milliseconds
// Outputs: none.
// Comments: Shutdown waits until all of its clients have released. It is
//           implemented as a passive operation in that it will not try
//           and coerces its clients to release.
//=============================================================================
btBool uAIA::Release(TransactionID const &rTranID,
                     btTime               timeout)
{
   DEBUG_CERR(__AAL_FUNC__ << ": Hello.\n");
   AAL_DEBUG(LM_Shutdown, __AAL_FUNC__ << ": Hello.\n");

   //--------------------------------------------------------------
   // The shutdown process involves a thread ShutdownThread
   //   that waits for all resources currently using the AAI to
   //   release or until a timeout occurs.  At that point the
   //   message delivery engine is shutdown and the AIA is done.
   //--------------------------------------------------------------
   struct shutdownparms_s *pParms = new struct shutdownparms_s(this,timeout,rTranID);

   //Important to lock here and not unlock until after the assignment are  the Shutdown
   //  thread can come and go before the assignment, resulting in the join off of m_pShutdownThread
   //  being off of a NULL poiter
   Lock();
   // Create the Shutdown thread
   m_pShutdownThread = new OSLThread(uAIA::ShutdownThread,
                                     OSLThread::THREADPRIORITY_NORMAL,
                                     pParms);
   if ( NULL == m_pShutdownThread ) {
      std::cout << "youch null threa" << std::endl;
   }
   Unlock();

   DEBUG_CERR(__AAL_FUNC__ << ": Goodbye.\n");
   AAL_DEBUG(LM_Shutdown, __AAL_FUNC__ << ": Goodbye.\n");
   return (m_pShutdownThread != NULL);
}

//=============================================================================
// Name: Release
// Description: Quite release calls release but suppresses events
// Interface: public
// Inputs:
// Outputs: none.
// Comments: Used when preparing the service for unloading
//=============================================================================
btBool uAIA::Release(btTime timeout)
{
   m_quietRelease = true;
   return Release(TransactionID(), timeout);
}


CAIA::~CAIA() {}

uAIASession * CAIA::CreateAIASession(IBase                *pOwnerBase,
                                     DeviceServiceBase    *pDevService,
                                     btEventHandler        EventHandler,
                                     ServiceBase          *pServiceBase)
{
   return new uAIASession(pOwnerBase, *this, (btApplicationContext)pDevService, EventHandler, pServiceBase);
}

void CAIA::DestroyAIASession(uAIASession *pSess)
{
   if ( NULL != pSess ) {
      delete pSess;
   }
}


END_NAMESPACE(AAL)

