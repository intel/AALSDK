// Copyright(c) 2015-2016, Intel Corporation
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
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
//  8/21/2015      JG       Initial vesions
///                @endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALLoggerExtern.h"
#include "aalsdk/kernel/ccipdriver.h"

#include "aalsdk/uaia/AIAService.h"
#include "aalsdk/aas/AALInProcServiceFactory.h"
#include "aalsdk/osal/ThreadGroup.h"

#include "AIA-internal.h"
#include "aalsdk/aas/Dispatchables.h"
#include "ALIAFUProxy.h"

//========================================
// Implements the AIA entry point for the
// load and bind operation
//========================================
#define SERVICE_FACTORY AAL::InProcSingletonSvcsFact< AAL::AIAService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AIASERVICE_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
AIASERVICE_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)

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
// Name: UIDClientEvent
// Description: Constructor
// Interface: public
// Inputs: - Class name.
// Outputs: Pointer to factory.
// Comments:
//=============================================================================
class UIDriverEvent : public IUIDriverEvent,
                      public CAALEvent
{
public:
   UIDriverEvent( IBase *pObject, uidrvMessage * pmessage)
   : CAALEvent(pObject),
     m_pmessage(pmessage)
   {
      SetInterface( evtUIDriverClientEvent,
                    dynamic_cast<IUIDriverEvent*>(this));

   }

   btHANDLE                  DevHandle()  const { return m_pmessage->handle();   }
   uid_msgIDs_e              MessageID()  const { return m_pmessage->id();       }
   btVirtAddr                Payload()    const { return m_pmessage->payload();  }
   btWSSize                  PayloadLen() const { return m_pmessage->size();     }
   stTransactionID_t const & msgTranID()  const { return m_pmessage->tranID();   }
   btObjectType              Context()   const { return m_pmessage->context(); }

   // HM 20081213
   uid_errnum_e              ResultCode() const { return m_pmessage->result_code(); }
   void                      ResultCode(uid_errnum_e e) { return m_pmessage->result_code(e); }


   virtual ~UIDriverEvent() { if(m_pmessage != NULL) delete m_pmessage; }

protected:
   uidrvMessage *m_pmessage;

   UIDriverEvent(){};
 };


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
   if(Shuttingdown == m_state){
      initFailed( new CExceptionTransactionEvent( NULL,
                                                  rtid,
                                                  errCreationFailure,
                                                  reasInvalidState,
                                                  "AIAService::Init Shutting down"));
      return true;  // Ignore requests while we are shuttingdown
   }
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

      m_Semaphore.Reset(0);

      // Create the Message delivery thread
      m_pMDT = new OSLThread(AIAService::MessageDeliveryThread,
                             OSLThread::THREADPRIORITY_NORMAL,
                             this);

      // Make sure that the kernel pipe to the database is open.
      //  The Wait is posted in the AIAService:MessageDeliveryThread
      //  indicating that it has started
      SemWait();
      m_bIsOK = true;

      // The AIA does not report itself up to the requester through initComplete()
      //  since it is really the factory and transport for the AFUProxies which are
      //  the actual Services. Since it is initComplete() that normally registers the
      //  Service with the ServiceModule we must register directly.  No event is generated.
      getAALServiceModule()->AddToServiceList(this);
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
   //  Note that it is important that the optargs passed in through this function signature is used NOT the OptArgs()
   //  method. The latter would pass the AIA's OptArgs NOT necessarily the ones passed in the allocate call.
   AFUProxyGet(pclientBase, optArgs, rtid);

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
}


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
   ALIAFUProxy *pnewProxy = new ALIAFUProxy(getAALServiceModule(), getRuntime() );

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
   ASSERT( NULL!=pAFUProxy );
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

      // If we are shutting down we will be counting releases
      //  using this semaphore as a count-up
      if(Shuttingdown == m_state){
         SemPost();
      }

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


AAL::btBool AIAService::MapWSID(AAL::btWSSize Size, AAL::btWSID wsid, AAL::btVirtAddr *pRet)
{
   return m_uida.MapWSID(Size, wsid, pRet);
}

void AIAService::UnMapWSID(AAL::btVirtAddr ptr, AAL::btWSSize Size)
{
   m_uida.UnMapWSID(ptr, Size);
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

   //Get a pointer to this objects context
   AIAService *This = (AIAService*)pContext;

   This->m_bIsOK=true;

   // Wake up the ctor
   This->SemPost();

   // Run the message pump
   This->Process_Event();

   // Message pump exited, so AIAService shutting down. Signal that by setting flag.
   This->m_uida.IsOK(false);

   // cerr << "AIAService::~AIAService: shutting down UI Client file\n";
   AAL_DEBUG(LM_UAIA,"AIAService::MessageDeliveryThread: shutting down UI Client file\n");

   // Close the channel
   This->m_uida.Close();

   // cout << "AIAService::~AIAService: done\n";
   AAL_INFO(LM_UAIA, "AIAService::MessageDeliveryThread: done\n");

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

   uidrvMessage *pMessage = new uidrvMessage;
   AAL_INFO(LM_UAIA, "AIAService::Process_Event. in\n");

   while(m_uida.GetMessage(pMessage) != false) {
      AAL_DEBUG(LM_UAIA, "AIAService::Process_Event: GetMessage Returned\n");
      if (pMessage->result_code() != uid_errnumOK) {
         AAL_WARNING(LM_UAIA, "AIAService::Process_Event: pMessage->result_code() is not uid_errnumOK, but is " <<
                  pMessage->result_code() << std::endl);
      }

      if (rspid_UID_Shutdown == pMessage->id()) { // Are we done?
         AAL_INFO(LM_UAIA, "AIAService::Process_Event: Shutdown Seen\n");
         delete pMessage;
         return;
      }else {

         // Generate the event - No need to destroy message as it being passed to event and will be
         // destroyed there.  TODO - Object should be Proxy not the AIA
         AFUProxyCallback *pDisp = new AFUProxyCallback(static_cast<IAFUProxyClient *>(pMessage->context()),
                                                        new UIDriverEvent(this,pMessage));
         getRuntime()->schedDispatchable(pDisp);

         pMessage = new uidrvMessage;

      }
   } // while()

   // catastrophic failure.  try to clean up after ourself.
   delete pMessage;

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

   AAL_INFO(LM_AIA, __AAL_FUNC__ << ": Releasing.\n");
   {
      AutoLock(this);
      m_state = Shuttingdown;
   }

   //--------------------------------------------------------------
   // The shutdown process involves a thread ShutdownThread
   //   that waits for all resources currently using the AIA to
   //   release or until a timeout occurs.  At that point the
   //   message delivery engine is shutdown and the AIA is done.
   //   This operations is performed in a dispatchable.
   //--------------------------------------------------------------
   IDispatchable * pDisp = new (std::nothrow) ShutdownDisp( this,
                                                            timeout,
                                                            TransactionID(dynamic_cast<IBase*>(this),true) );
   ASSERT(NULL != pDisp);
   FireAndForget(pDisp);
   return true;
}

//=============================================================================
// Name: WaitForShutdown()
// Description: Waits for all Driver stack to shutdown and then competes
//                release.
// Inputs: rtid - TransactionID
//         waittime - amount of time to wait
// Interface: public
// Outputs: none.
//=============================================================================
void AIAService::WaitForShutdown(TransactionID const &rtid,
                                 btTime timeout)
{
   // Wait for any children
   SemWait();
   m_pMDT->Join();
   delete m_pMDT;
   m_pMDT = NULL;

   AAL_INFO(LM_AIA, __AAL_FUNC__ << ": Done Releasing.\n");

   // Since we are a singleton that never presented to a
   //  client we don't do a ServiceBase::Release just complete now.
   ServiceBase::ReleaseComplete();
}

//=============================================================================
// Name: serviceReleased()
// Description: During a shutdown where AFUProxies were not released we Release
//              them. But because the application is shutting down we don't let
//              the events go to the normal client. We override with TranID and
//              eat the event.  We also decrement a count so we know when we
//              are done.
// Inputs: rtid - TransactionID
// Interface: public
// Outputs: none.
//=============================================================================
void AIAService::serviceReleased(TransactionID const &rTranID )
{
   SemPost();
}

//=============================================================================
// Name: serviceReleaseFailed()
// Description: During a shutdown where AFUProxies were not released we Release
//              them. But because the application is shutting down we don't let
//              the events go to the normal client. We override with TranID and
//              eat the event.  We also decrement a count so we know when we
//              are done.
// Inputs: rtid - TransactionID
// Interface: public
// Outputs: none.
// Comments: Would not expect this to happen
//=============================================================================
void AIAService::serviceReleaseFailed(const IEvent &rEvent)
{
   SemPost();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             AIA DISPATCHABLES             ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
AIAService::ShutdownDisp::ShutdownDisp(AIAService *pAIA,
                                       btTime time,
                                       TransactionID const &tid)
: m_pAIA(pAIA),
  m_timeout(time),
  m_tid(tid)
{}

void AIAService::ShutdownDisp::operator() ()
{
   // Assume no children so we will not wait
   m_pAIA->m_Semaphore.Reset(1);

   ReleaseChildren(m_tid);

   ShutdownMDT * pShutdownTrans = new (std::nothrow) ShutdownMDT(m_tid,m_timeout);
   ASSERT(pShutdownTrans->IsOK());

   if(!pShutdownTrans->IsOK()){
      delete this;
      return;
   }

   m_pAIA->SendMessage(NULL, pShutdownTrans, NULL);

   m_pAIA->WaitForShutdown(m_tid, m_timeout);

   delete this;
}

void AIAService::ShutdownDisp::ReleaseChildren(TransactionID const &tid)
{
   AFUList_citr iter = m_pAIA->m_mAFUList.end();

   btUnsigned32bitInt size = static_cast<btUnsigned32bitInt>(m_pAIA->m_mAFUList.size());
   if ( 0 == size ) {
      return;
   }

   // Set the count up semaphore
   m_pAIA->m_Semaphore.Reset( - size);

   while ( m_pAIA->m_mAFUList.begin() != iter ) {

      // Get the IAALService from the IBase

      IAALService *pService = dynamic_ptr<IAALService>(iidService, (*iter));

      iter--;

      if ( NULL != pService ) {
         // Release the Service overriding the default delivery
         pService->Release(tid);
      }
   }
}

END_NAMESPACE(AAL)

