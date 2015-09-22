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
/// @file AIAService.cpp
/// @brief Universal Application Interface Adaptor.
/// @ingroup AIAService
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
///                            DEFINE_SERVICE_FACTORY_ACCESSOR(AIAService) because
///                            of Namespacing change in AIAService.h
/// 12/10/2008     HM       Added Logger
/// 12/14/2008     HM       Added result_code accessor / mutator
/// 01/04/2009     HM       Updated Copyright
/// 02/01/2009     HM       Put "Returned" in Logger
/// 02/14/2009     HM       Modified UnBind, and expanded MessageHandler
/// 02/20/2009     HM       Added AALLogger messages to
///                            AIAServiceMangement::MessageHandler
/// 05/18/2009     HM       Process_Event() now forwards messages with error
///                            codes on up the stack
/// 06/06/2009     HM       Message pump shutdown
/// 06/07/2009     HM       Calls to AALLogger in ~AIAService currently execute after
///                            exit() from main(). By that time the Logger is
///                            gone, segfault. Removed Logging calls from dtor.
///                         Will reinstate once SystemStop() correctly shuts
///                            down AIAService before exiting.
/// 06/22/2009     JG       Massive changes to support new proxy mechanism and
///                            to fix build dependencies that required external
///                            linking to the module, breaking plug-in model.
/// 07/06/2009     HM/JG    Fixed double-destruction of AIAService. Refined AIAService shut-
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

#include "aalsdk/AALLoggerExtern.h"

#include "aalsdk/aas/AALInProcServiceFactory.h"
#include <aalsdk/osal/OSServiceModule.h>

#include "AIA-internal.h"
#include "aalsdk/Dispatchables.h"
#include "ALIAFUProxy.h"

//#include "aalsdk/AALTypes.h"

//#include "aalsdk/AALTransactionID.h"

//#include "aalsdk/uaia/FAPPIP_AFUdev.h"


//#include "aalsdk/kernel/KernelStructs.h"

//#include "UIDriverInterfaceAdapter.h"
//#include "aalsdk/CAALEvent.h"


//========================================
// Implements the AIA entry point for the
// load and bind operation
//========================================
#ifndef AIASERVICE_VERSION_CURRENT
# define AIASERVICE_VERSION_CURRENT  4
#endif // AIASERVICE_VERSION_CURRENT
#ifndef AIASERVICE_VERSION_REVISION
# define AIASERVICE_VERSION_REVISION 2
#endif // AIASERVICE_VERSION_REVISION
#ifndef AIASERVICE_VERSION_AGE
# define AIASERVICE_VERSION_AGE      0
#endif // AIASERVICE_VERSION_AGE
#ifndef AIASERVICE_VERSION
# define AIASERVICE_VERSION          "4.2.0"
#endif // AIASERVICE_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef AIA_SERVICE_EXPORTS
#    define AIA_SERVICE_API __declspec(dllexport)
# else
#    define AIA_SERVICE_API __declspec(dllimport)
# endif // AIA_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define AIA_SERVICE_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define SERVICE_FACTORY AAL::InProcSingletonSvcsFact< AIAService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__


AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libaia, AIA_SERVICE_API, AIASERVICE_VERSION, AIASERVICE_VERSION_CURRENT, AIASERVICE_VERSION_REVISION, AIASERVICE_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


USING_NAMESPACE(AAL)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////               AIA SERVICE                 ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================


//=============================================================================
// Name: init()
// Description: Initialize the service
// Interface: public
// Inputs: Optional Transaction ID
// Outputs: none.
// Comments:  This function may be called more than once per process but the
//            AIAService and its UIDriverClient are singletons per process.
//=============================================================================
btBool AIAService::init( IBase *pclientBase,
                         NamedValueSet const &optArgs,
                         TransactionID const &rtid )
{
   AutoLock(this);      // Prevent init() reenterancy
   AAL_INFO(LM_UAIA, "AIAService::init. in\n");

   //Singleton service already initialized
   if(!m_bIsOK){

      // Create a channel to the UI Driver now
      m_uida.Open();
      if (!m_uida.IsOK()) {
         m_bIsOK = false;
         initFailed( new CExceptionTransactionEvent( NULL,
                                                     rtid,
                                                     errCreationFailure,
                                                     reasCauseUnknown,
                                                     "AIAService::Init Failed to open UI Driver"));
         return true;
      }

      // Create the Message delivery thread
      m_pMDT = new OSLThread(AIAService::MessageDeliveryThread,
                             OSLThread::THREADPRIORITY_NORMAL,
                             this);

      // Make sure that the kernel pipe to the database is open.
      //  The Wait is posted in the AIAService:MessageDeliveryThread
      //  indicating that it has started
      SemWait();
      m_bIsOK = true;
      AAL_INFO(LM_UAIA, "AIAService::Create, out\n");
   }

   // The AIA object is responsible for marshaling messages to and from the kernel. It also acts
   //  as a factory for the AFUProxy, which is the actual Service object presented to the client.
   // The AFUProxy is used by the client to send and receive messages to and from the AFU. The proxy uses
   //  the AIA as a transport.
   //
   // Allocate an AFU Proxy. The AFU Proxy appears as a Service so will be returned
   //  via  serviceAllocated() to the caller of init(). The ServiceClient (owner) will be the caller of init()
   //  NOT the AIA. The AIA does have to keep a reference count of AFUProxies to properly shutdown.
   AFUProxyGet(pclientBase, OptArgs(), rtid);

   return true;
}

//=============================================================================
// Name: ~AIAService()
// Description: Universal AIA class. Singleton service that supports multiple
//              instances of AIA Proxies.
// Interface: public
// Outputs: none.
//=============================================================================
AIAService::~AIAService(void)
{
   AAL_INFO(LM_UAIA, "AIAService::~AIAService. in\n");
   Destroy();
}

//=============================================================================
// Name: Destroy()
// Description: Method invoked for destroying the AIA prior to unload,
// Interface: public
// Outputs: none.
//=============================================================================
void AIAService::Destroy(void)
{
   // Shutdown the message pump by putting in a reqid_Shutdown to force a
   //    wake-up and let the kernel know it is going down.
#if 0
   char szFunc[] = "AIAService::Destroy";
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
#endif
   m_bIsOK = false;

}  // AIAService::Destroy

//=============================================================================
// Name: SemWait()
// Description: Wait on semaphore
// Interface: protected
// Outputs: none.
//=============================================================================
void AIAService::SemWait()
{
   m_Semaphore.Wait();
}


//=============================================================================
// Name: SemPost()
// Description: Post Semaphore
// Interface: protected
// Outputs: none.
//=============================================================================
void AIAService::SemPost()
{
   m_Semaphore.Post(1);
}

//=============================================================================
// Name: AFUProxyGet()
// Description: Gets a new Proxy to an AFU
// Interface: protected
// Returns: none.
// Comments: The design of the AIA is such that the AFUProxy could be a
//           separate AAL Service but is currently implemented as an internal
//           object.
//=============================================================================
void AIAService::AFUProxyGet( IBase *pServiceClient,
                              NamedValueSet const &OptArgs,
                              TransactionID const &rtid)
{
   // Make a copy of the Optargs so we can add a field
   NamedValueSet Manifest(OptArgs);

   // Proxy will need access to the AIA.  Passing as a Manifest argument will make
   //  it easier to change the Proxy to a proper AAL Service in the future
   Manifest.Add(AIA_SERVICE_BASE_INTERFACE, static_cast<btAny>(dynamic_cast<IBase*>(this)));

   // Construct the new AFUProxy
   ALIAFUProxy *pnewProxy = new ALIAFUProxy(pAALServiceModule(), getRuntime());

   // Initialize the Service.  The Service will register with the AIA.
   pnewProxy->_init(pServiceClient, rtid, Manifest, NULL);

}

//=============================================================================
// Name: AFUProxyRelease()
// Description: De-register an AFU Proxy
// Inputs: pAFUbase - IBase of AFU Proxy
// Interface: public
// Returns: true - success.
//=============================================================================
void AIAService::AFUProxyRelease(IBase *pAFUbase)
{
   AutoLock(this);
   AFUListDel(pAFUbase);
}


//=============================================================================
// Name: AFUProxyAdd()
// Description: Register an initialized AFU Proxy
// Inputs: pAFUbase - IBase of AFU Proxy
// Interface: public
// Returns: true - success.
//=============================================================================
void AIAService::AFUProxyAdd( IBase *pAFUProxy )
{

   // Bug in AFU Proxy
   ASSERT(NULL==pAFUProxy);
   AFUListAdd(pAFUProxy);
}

//=============================================================================
// Name: AFUListAdd()
// Description: Add an AFUProxy to the list
// Interface: protected
// Returns: true - success.
//=============================================================================
btBool AIAService::AFUListAdd( IBase *pAFU)
{
   {
      AutoLock(this);

      // Do not allow duplicates
      AFUList_itr iter = find(m_mAFUList.begin(), m_mAFUList.end(), pAFU);
      if( m_mAFUList.end() != iter ) {
         return false;
      }

      // Make sure its not already on the list
      m_mAFUList.push_back(pAFU);
   }

   return true;
}

//=============================================================================
// Name: AFUListDel()
// Description: Delete an AFUProxy from the list
// Interface: protected
// Returns: true - success.
//=============================================================================
btBool AIAService::AFUListDel(IBase *pAFU)
{
   AutoLock(this);

   AFUList_itr iter = find(m_mAFUList.begin(), m_mAFUList.end(), pAFU);
   if ( m_mAFUList.end() != iter ) {
      m_mAFUList.erase(iter);
      return true;
   }

   return false;;
}


//=============================================================================
// Name: SendMessage()
// Description: Send a message down the UIDriverInterfaceAdapter
// Interface: public
// Outputs: none.
//=============================================================================
void AIAService::SendMessage( AAL::btHANDLE devHandle,
                              IAIATransaction *pMessage,
                              IAFUProxyClient *pProxyClient)
{

   // Pass it to the low level transport
   m_uida.SendMessage(devHandle, pMessage, pProxyClient);

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
void AIAService::MessageDeliveryThread(OSLThread *pThread,
                                       void *pContext)
{
#if 0
   //Get a pointer to this objects context
   AIAService *This = (AIAService*)pContext;

   This->m_bIsOK=true;

   // Wake up the ctor
   This->SemPost();

   // Run the message pump
   This->Process_Event();

   // Message pump exited, so AIAService shutting down. Signal that by setting flag.
   This->m_pUIDC->IsOK(false);

   // cerr << "AIAService::~AIAService: shutting down UI Client file\n";
   AAL_DEBUG(LM_UAIA,"AIAService::MessageDeliveryThread: shutting down UI Client file\n");

   // Close the channel
   This->m_pUIDC->Close();

   // cout << "AIAService::~AIAService: done\n";
   AAL_INFO(LM_UAIA, "AIAService::MessageDeliveryThread: done\n");

   // Final release
   This->Released();
#endif

}  // AIAService::MessageDeliveryThread

//=============================================================================
// Name: Process_Event
// Description: Process RMS event
// Interface: protected
// Inputs: none
// Outputs: none.
// Comments: Processes all events until none are left
//=============================================================================
void
AIAService::Process_Event()
{
#if 0
   uidrvMessage *pMessage = new uidrvMessage;
   AAL_INFO(LM_UAIA, "AIAService::Process_Event. in\n");

   while (m_pUIDC->GetMessage(pMessage) != false) {
      AAL_DEBUG(LM_UAIA, "AIAService::Process_Event: GetMessage Returned\n");
      if (pMessage->result_code() != uid_errnumOK) {
         AAL_WARNING(LM_UAIA, "AIAService::Process_Event: pMessage->result_code() is not uid_errnumOK, but is " <<
                  pMessage->result_code() << std::endl);
      }

      if (rspid_UID_Shutdown == pMessage->id()) { // Are we done?
         // Important to Lock here to make sure the m_pShutdownThread has been set
         {
            AutoLock(this);

            AAL_INFO(LM_UAIA, "AIAService::Process_Event: Shutdown Seen\n");

            //Shutdown complete
            ASSERT(NULL != m_pShutdownThread);
            if ( NULL != m_pShutdownThread ) {
               m_pShutdownThread->Join();
            }
         }
         delete pMessage;
         return;
      }else {

         // Generate the event - No need to destroy message as it being passed to event and will be
         // destroyed there.  TODO - Consider cleaner memory management - Try to avoid copies
         uidrvMessageRoute const &msgroute = pMessage->msgRoute();
         IBase *proxybase = pMessage->msgRoute().AIAProxybasep();
         btEventHandler handler = pMessage->msgRoute().Handler();
         TransactionID tranID = pMessage->tranID();

         UIDriverClientEvent *pEvent = new UIDriverClientEvent(proxybase,
                                                              pMessage,
                                                              tranID);
         pEvent->setHandler(handler);
         getRuntime()->schedDispatchable(pEvent);

         pMessage = new uidrvMessage;

      }
   } // while()

   // catastrophic failure.  try to clean up after ourself.
   delete pMessage;
#endif
} // AIAService::Process_Event


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////       AIAService SERVICE PROVIDER INTERFACE     ///////////////////
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
void AIAService::ShutdownThread(OSLThread *pThread,
                                void *pContext)
{
#if 0
   struct shutdownparms_s *pParms = static_cast<struct shutdownparms_s*>(pContext);

   pParms->pAIA->WaitForShutdown( ui_shutdownReasonNormal,
                                  pParms->shutdowntime,
                                  pParms->tid_t);

   delete pParms;
#endif
   }


//=============================================================================
// Name: WaitForShutdown()
// Description: Waits for all resources to release the AIA.
// Inputs: reason - Reason the service was brought down
//         waittime - amount of time to wait
// Interface: public
// Outputs: none.
//=============================================================================
void AIAService::WaitForShutdown(ui_shutdownreason_e      reason,
                           btTime                   waittime,
                           stTransactionID_t const &rTranID_t)
{
#if 0
   btBool     notimeout = true;
   btBool     DoWait    = false;
   CAALEvent *pTheEvent = NULL;

   {
      AutoLock(this);

      DEBUG_CERR("AIAService::WaitForShutdown() refcount is " << m_refcount << ". 1 of 3\n");
      AAL_DEBUG(LM_Shutdown,"AIAService::WaitForShutdown() refcount is " << m_refcount << ". 1 of 3\n");

      // If the refcount is not zero setup a count up
      // semaphore to wait until all resources release
      if ( m_refcount ) {
         DEBUG_CERR("AIAService::WaitForShutdown() refcount is non-zero. 2 of 3\n");
         AAL_DEBUG(LM_Shutdown,"AIAService::WaitForShutdown() refcount is non-zero. 2 of 3\n");

         m_pShutdownSem = new CSemaphore();
         // Negative count means the semaphore counts up
         m_pShutdownSem->Create(-m_refcount, 1);

         DoWait = true;

      } else {
         DEBUG_CERR("AIAService::WaitForShutdown() refcount was 0. 2 of 3\n");
         AAL_DEBUG(LM_Shutdown,"AIAService::WaitForShutdown() refcount was 0. 2 of 3\n");
      }

   }

   if ( DoWait ) {

      // The semaphore returns true if it unblocked with no timeout
      if ( AAL_INFINITE_WAIT == waittime ) {
         notimeout = m_pShutdownSem->Wait();
      } else {
         notimeout = m_pShutdownSem->Wait(waittime);
      }

      {
         AutoLock(this);
         delete m_pShutdownSem;
         m_pShutdownSem = NULL;
      }
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
         pTheEvent->setHandler(m_Listener);
         getRuntime()->schedDispatchable(pTheEvent);
         DEBUG_CERR("AIAService::WaitForShutdown() exiting. 3 of 3\n");
         AAL_DEBUG(LM_Shutdown,"AIAService::WaitForShutdown() exiting. 3 of 3\n");
      }else{
         DEBUG_CERR("AIAService::WaitForShutdown() FATAL ERROR. CANNOT CREATE EVENT\n");
         AAL_DEBUG(LM_Shutdown,"AIAService::WaitForShutdown() FATAL ERROR. CANNOT CREATE EVENT\n");
      }
   }

   // TODO timeout should be adjusted for above waitimes elapsed
   IssueShutdownMessageWorker(rTranID_t, waittime );
#endif
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
btBool AIAService::IssueShutdownMessageWorker(stTransactionID_t const &rTranID_t,
                                        btTime                   timeout)
{
#if 0
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
#endif
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
btBool AIAService::Release(TransactionID const &rTranID,
                           btTime               timeout)
{
#if 0
   DEBUG_CERR(__AAL_FUNC__ << ": Hello.\n");
   AAL_DEBUG(LM_Shutdown, __AAL_FUNC__ << ": Hello.\n");

   //--------------------------------------------------------------
   // The shutdown process involves a thread ShutdownThread
   //   that waits for all resources currently using the AIA to
   //   release or until a timeout occurs.  At that point the
   //   message delivery engine is shutdown and the AIA is done.
   //--------------------------------------------------------------
   struct shutdownparms_s *pParms = new struct shutdownparms_s(this,timeout,rTranID);

   //Important to lock here and not unlock until after the assignment are  the Shutdown
   //  thread can come and go before the assignment, resulting in the join off of m_pShutdownThread
   //  being off of a NULL pointer.

   {
      AutoLock(this);

      // Create the Shutdown thread
      m_pShutdownThread = new(std::nothrow) OSLThread(AIAService::ShutdownThread,
                                                      OSLThread::THREADPRIORITY_NORMAL,
                                                      pParms);
      ASSERT(NULL != m_pShutdownThread);
   }

   DEBUG_CERR(__AAL_FUNC__ << ": Goodbye.\n");
   AAL_DEBUG(LM_Shutdown, __AAL_FUNC__ << ": Goodbye.\n");
#endif
   ServiceBase::Release(rTranID, timeout);
   return true;
   //return (m_pShutdownThread != NULL);
}
