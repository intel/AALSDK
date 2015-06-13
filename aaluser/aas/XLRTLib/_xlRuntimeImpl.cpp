// Copyright (c) 2014-2015, Intel Corporation
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
//        FILE: _xlRuntimeImpl.cpp
//     CREATED: Mar 7, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: Implementation of XL _runtime class
// HISTORY:
// COMMENTS: This is the internal implementation of the runtime
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// XL Runtime definitions
#include "aalsdk/AALTypes.h"
#include "aalsdk/aas/ServiceHost.h"
#include "aalsdk/Dispatchables.h"

#include "_xlRuntimeImpl.h"
#include "_xlMessageDelivery.h"
#include "_xlServiceBroker.h"

#include "aalsdk/osal/Sleep.h"
#include "aalsdk/osal/Env.h"

#include "aalsdk/INTCDefs.h"
#include "aalsdk/CAALEvent.h"

using namespace std;

BEGIN_NAMESPACE(AAL)


BEGIN_C_DECLS
// Singleton Runtime implementation
static _runtime *pruntime = NULL;

//=============================================================================
// Name: _getnewRuntimeInstance
// Description: Factory for the singleton Runtime Implementation
// Interface: private
// Inputs: pRuntimeProxy - Pointer to the Runtime Container (Proxy)
//         pClient - Pointer to the client for this instance.
// Outputs: pointer to Runtim implementation.
// Comments: The Runtime implementation keeps a map of Proxy to Clients
//           so that multiple Proxy's can point to the same Runtime and
//           messages are routed appropriately.
//=============================================================================
_runtime *_getnewRuntimeInstance( Runtime *pRuntimeProxy,
                                  IRuntimeClient *pClient)
{


   // If There is no client then there isn't much
   //  to do but silently fail.
   if(NULL == pClient){
      ASSERT(NULL != pClient);
      AAL_ERR(LM_AAS, "Missing IRuntimeClient on Runtime construction");
      return NULL;
   }

   // If missing the Proxy fail
   if(NULL == pRuntimeProxy) {
      // Dispatch the event ourselves, because MDS is no more.
      OSLThreadGroup oneShot;
      RuntimeCallback *pRuntimeStopped = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                                             pClient,
                                                             new CExceptionTransactionEvent( NULL,
                                                                                             extranevtRuntimeCreateorProxy,
                                                                                             TransactionID(),
                                                                                             errCreationFailure,
                                                                                             reasMissingParameter,
                                                                                             "Failed to instantiate Runtime"));

       // Fire the final event
      oneShot.Add(pRuntimeStopped);
      oneShot.Drain();  // Wait for it to be dispatched
      return NULL;
   }

   // If this is the first create then instantiate
   if(NULL == pruntime){
      pruntime = new _runtime(pRuntimeProxy, pClient);
   }

   // Connect this client and proxy to the runtime
   pruntime->addProxy(pRuntimeProxy, pClient);

   if(!pruntime->IsOK()){
      // Dispatch the event ourselves, because MDS is no more.
      OSLThreadGroup oneShot;
      RuntimeCallback *pRuntimeStopped = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                                             pClient,
                                                             new CExceptionTransactionEvent( NULL,
                                                                                             extranevtRuntimeCreateorProxy,
                                                                                             TransactionID(),
                                                                                             errCreationFailure,
                                                                                             reasCauseUnknown,
                                                                                             "Failed to instantiate Runtime"));

       // Fire the final event
      oneShot.Add(pRuntimeStopped);
      oneShot.Drain();  // Wait for it to be dispatched
      return NULL;
   }

   return pruntime;
}
END_C_DECLS

//=============================================================================
// Name: _releasRuntimeInstance
// Description: Releases a Runtime instance
// Interface: private
// Inputs: pRuntimeProxy - Pointer to the Runtime Container (Proxy)
//         pRuntime - Pointer to the Runtime
// Outputs: pointer to Runtim implementation.
// Comments: The Runtime implementation keeps a map of Proxy to Clients
//           so that multiple Proxy's can point to the same Runtime and
//           messages are routed appropriately.
//=============================================================================
void _runtime::releaseRuntimeInstance( Runtime *pRuntimeProxy)
{

   if( NULL == pruntime ){
      AAL_ERR(LM_AAS, "releaseRuntimeInstance() called with no Runtime present");
      return;
   }

   // If missing the Proxy fail
   if(NULL == pRuntimeProxy) {
      // Dispatch the event ourselves, because MDS is no more.
      OSLThreadGroup oneShot;
      RuntimeCallback *pRuntimeRelease = new RuntimeCallback(RuntimeCallback::Event,
                                                             m_pClient,
                                                             new CExceptionTransactionEvent( NULL,
                                                                                             extranevtRuntimeDestroyorRelease,
                                                                                             TransactionID(),
                                                                                             errReleaseFailure,
                                                                                             reasMissingParameter,
                                                                                             "Failed to release Runtime"));

       // Fire the final event
      oneShot.Add(pRuntimeRelease);
      oneShot.Drain();  // Wait for it to be dispatched
      return;
   }

   // If its not the owner then just removes this proxy
   if(pRuntimeProxy != m_pOwner){
      removeProxy(pRuntimeProxy);
      return;
   }

   if ( IsOK() && (m_state !=Stopped) ) {
      // Stop and clean up properly  TODO
   }

   delete this;
}

//=============================================================================
// Name: _runtime
// Description: Constructor
// Interface: public
// Outputs: none.
// Comments:
//=============================================================================
_runtime::_runtime(Runtime* pRuntimeProxy, IRuntimeClient*pClient) :
   CAASBase(),
   m_status(false),
   m_pOwner(pRuntimeProxy),
   m_pOwnerClient(pClient),
   m_pClient(this),              // Must be our own client for internal services
   m_pMDSSvcHost(NULL),
   m_pBrokerSvcHost(NULL),
   m_pMDS(NULL),
   m_pMDSbase(NULL),
   m_pBroker(NULL),
   m_pBrokerbase(NULL),
   m_state(Stopped)
{
   m_sem.Create(0);

   // Register interfaces
   // Add the public interfaces
   if ( SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) != EObjOK ) {
      return;
   }

   // Instantiate the core facilities. To deal with chicken and egg problem, to bootstrap
   //  default Services the Runtime performs some functionality typically reserved for Broker

   m_status = true;  // Assume all will go fine.

   // Message Delivery Service

   // The ServiceHost loads the Service Implementation, typically contained in a Service Library (.so/.dll)
   //  The default Service implementations are allocated directly in the Runtime Service Library.
   //  This variant assumes the Service is implemented within this executable.
   m_pMDSSvcHost = new ServiceHost(AAL_SVC_MOD_ENTRY_POINT(localMDS), this, this);

   // Instantiate the Service where the Runtime (this) implements the Client.
   m_pMDSSvcHost->allocService( this, NamedValueSet(), TransactionID(MDS));

   // Block until the serviceAllocated (or failed) callback
   m_sem.Wait(); // for the local Message Delivery Service
   if(false == m_status){
      // Dispatch the event ourselves, because MDS did not start.
      OSLThreadGroup oneShot;
      RuntimeCallback *pRuntimeFailed = new RuntimeCallback( RuntimeCallback::CreateorGetProxyFailed,
                                                             m_pOwnerClient,
                                                             new CExceptionTransactionEvent( m_pOwner,
                                                                                             extranevtRuntimeCreateorProxy,
                                                                                             TransactionID(),
                                                                                             errCreationFailure,
                                                                                             reasSubModuleFailed,
                                                                                             "Failed to load Message Delivery Service"));

       // Fire the final event
      oneShot.Add(pRuntimeFailed);
      oneShot.Drain();  // Wait for it to be dispatched
      return;
   }

   // Service Broker
   m_pBrokerSvcHost = new ServiceHost(AAL_SVC_MOD_ENTRY_POINT(localServiceBroker), this, this);
   m_pBrokerSvcHost->allocService( this, NamedValueSet(), TransactionID(Broker));
   m_sem.Wait(); // for the local Broker

   if(false == m_status){
       // Dispatch the event ourselves, because MDS is no more.
       OSLThreadGroup oneShot;
       RuntimeCallback *pRuntimeFailed = new RuntimeCallback( RuntimeCallback::CreateorGetProxyFailed,
                                                              m_pOwnerClient,
                                                              new CExceptionTransactionEvent( m_pOwner,
                                                                                              extranevtRuntimeCreateorProxy,
                                                                                              TransactionID(),
                                                                                              errCreationFailure,
                                                                                              reasSubModuleFailed,
                                                                                              "Failed to load Broker Service"));

        // Fire the final event
       oneShot.Add(pRuntimeFailed);
       oneShot.Drain();  // Wait for it to be dispatched
       return;
    }
}

//=============================================================================
// Name: IsOK
// Description: Is the object functional
// Interface: public
// returns: true if functional.
// Comments:
//=============================================================================
btBool _runtime::IsOK()
{
   return m_status;
}


//=============================================================================
// Name: stop
// Description: Stop the runtime
// Interface: public
// Comments:
//=============================================================================
void _runtime::stop(Runtime* pProxy)
{
   Lock();
   if(NULL == pProxy){
      // Runtime Failed to start because it already is  TODO broadcast
      SendMsg(new RuntimeCallback(RuntimeCallback::Event,
                                  m_pOwnerClient,
                                  new CExceptionTransactionEvent(m_pOwner,
                                                                 exttranevtSystemStop,
                                                                 TransactionID(),
                                                                 errBadParameter,
                                                                 reasMissingParameter,
                                                                 "NULL Proxy")),
                                  NULL);
   }

   if(m_pOwner != pProxy ){
      // Runtime Failed to stop. Can only stop original
      SendMsg(new RuntimeCallback(RuntimeCallback::StopFailed,
                                  m_pOwnerClient,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStop,
                                                                 TransactionID(),
                                                                 errSysSystemPermission,
                                                                 reasNotOwner,
                                                                 "Not using original Runtime")),
                                  NULL);

   }

   // If the runtime is OK but is NOT stopped AND
   if ( IsOK() && (m_state != Stopped) ) {
      m_status = false;
      Unlock();

      // Prepare our sem. We will wait for notification from serviceReleased() before continuing.
      m_sem.Reset(0);

      // Release the Service Broker.
      dynamic_ptr<IAALService>(iidService, m_pBrokerbase)->Release(TransactionID(Broker));
   } else {
      // Dispatch the event ourselves, because MDS is no more.
      OSLThreadGroup oneShot;
      RuntimeCallback *pRuntimeStopped = new RuntimeCallback(RuntimeCallback::Stopped,
                                                             m_pOwnerClient,
                                                             pProxy);

       // Fire the final event
      oneShot.Add(pRuntimeStopped);
      oneShot.Drain();  // Wait for it to be dispatched

      Unlock();
   }
}

//=============================================================================
// Name: start
// Description: Start the runtime.
// Interface: public
// Inputs: pProxy - Pointer to Runtime's owner IBase interface
//         rconfigParms - Configuration parameters
// Outputs: none.
// Comments:
//=============================================================================
btBool _runtime::start(Runtime             *pProxy,
                       const NamedValueSet &rConfigParms)
{
   if(m_pOwner != pProxy){
      // Runtime Failed to start because it already is  TODO broadcast
      SendMsg(new RuntimeCallback(RuntimeCallback::StartFailed,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStart,
                                                                 TransactionID(),
                                                                 errSysSystemStarted,
                                                                 reasNotOwner,
                                                                 "Not using original Runtime")),
                                  NULL);


   }
   if( Started == m_state  ){
      // Runtime Failed to start because it already is
      SendMsg(new RuntimeCallback(RuntimeCallback::StartFailed,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStart,
                                                                 TransactionID(),
                                                                 errSysSystemStarted,
                                                                 reasSystemAlreadyStarted,
                                                                 strSystemAlreadyStarted)),
                                  NULL);
      return false;
   }

   // Process the configuration parameters
   if ( !ProcessConfigParms(rConfigParms) ) {
      return false;
   }


   if ( IsOK() ) {
      m_state = Started;
      SendMsg(new RuntimeCallback(RuntimeCallback::Started,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms), NULL);
      return true;
   } else {
      // Runtime Failed to start
      SendMsg(new RuntimeCallback(RuntimeCallback::StartFailed,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStart,
                                                                 TransactionID(),
                                                                 errSystemTimeout,
                                                                 reasSystemTimeout,
                                                                 "XL Runtime Failed to start - Rest of event is bogus")),
                                  NULL);
      return false;
   }
}

//=============================================================================
// Name: addProxy
// Description: Adds a Client to the Runtime instance by recording the Proxy
//              and client.
// Interface: public
// Inputs: pRuntimeProxy - Pointer to Proxy Runtime
//         pClient - Client for the proxy
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::addProxy( Runtime *pRuntimeProxy,
                         IRuntimeClient *pClient)
{
   AutoLock(this);
   if( (NULL == pRuntimeProxy) || (NULL == pClient) ){
      AAL_ERR(LM_AAS, "addProxy: NULL Proxy or Client");
      return;
   }

   // Save in map
   m_mClientMap[pRuntimeProxy] = pClient;

}

//=============================================================================
// Name: removeProxy
// Description: Removes a Client from the Runtime instance.
// Interface: public
// Inputs: pRuntimeProxy - Pointer to Proxy Runtime
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::removeProxy( Runtime *pRuntimeProxy)
{
   AutoLock(this);

   if(NULL == pRuntimeProxy){
      AAL_ERR(LM_AAS, "removeProxy: NULL Proxy.");
      return;
   }

   // Erase it
   ClientMap_itr     cmItr = m_mClientMap.find(pRuntimeProxy);
   if( m_mClientMap.end() == cmItr ){
      AAL_ERR(LM_AAS, "removeProxy: Proxy not found.");
      return;
   }
   m_mClientMap.erase(cmItr);

}

//=============================================================================
// Name: ProcessConfigParms
// Description: Process config parms for Runtime configuration record
// Interface: public
// Inputs: rConfigParms - Config parms
// Outputs: none.
// Comments:
//=============================================================================
Environment * Environment::sm_EnvObj = NULL;

btBool _runtime::ProcessConfigParms(const NamedValueSet &rConfigParms)
{
   NamedValueSet const *pConfigRecord;
   btcString            sName  = NULL;
   std::string               strSname;
  // Environment          env;

   //
   // First check environment
   if( false == (Environment::GetObj()->Get("XLRUNTIME_CONFIG_BROKER_SERVICE", strSname)) ){

      if ( ENamedValuesOK != rConfigParms.Get(XLRUNTIME_CONFIG_RECORD, &pConfigRecord) ) {
         // Check to see if default services are running
         return true;
      }

      if ( ENamedValuesOK == pConfigRecord->Get(XLRUNTIME_CONFIG_BROKER_SERVICE, &sName) ) {
         if ( NULL == m_pBrokerbase ) {
            // Runtime Failed to start
            SendMsg(new RuntimeCallback(RuntimeCallback::StartFailed,
                                        m_pOwnerClient,
                                        m_pOwner,
                                        rConfigParms,
                                        new CExceptionTransactionEvent(m_pOwner,
                                                                       exttranevtServiceShutdown,
                                                                       TransactionID(),
                                                                       errSystemTimeout,
                                                                       reasSystemTimeout,
                                                                       "XL Runtime Failed to start - No Broker - Rest of event is bogus")),
                                        NULL);
            return false;
         }
      }
   }

   if(NULL != sName){
      // Create the Service Request
      NamedValueSet ConfigRecord;
      NamedValueSet optArgs;

      // Using back door because thats all we know.
      ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, sName);
      optArgs.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, ConfigRecord);

      // Allocate the service.
      allocService(this, optArgs, TransactionID(Broker));
      m_sem.Wait();
   }
   return true;
}

//=============================================================================
// Name: allocService
// Description: Request to have a Service Allocated internally
// Interface: public
// Inputs: pClient - Pointer to client's callback interface
//         rManifest - Manifest describing Service
//         rTranID - Optional TransactionID
//         ThisThread - btBool indicating if proc should run in this thread
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::allocService(IBase                  *pClient,
                            NamedValueSet const    &rManifest,
                            TransactionID const    &rTranID)
{
   AutoLock(this);
   if ( IsOK() ) {
      // Used for internal allocations, this object is the Runtime and the RuntimeClient
      //  (first 2 arguments)
      m_pBroker->allocService(this, this, pClient, rManifest, rTranID);
   }
}

//=============================================================================
// Name: allocService
// Description: Request to have a Service Allocated
// Interface: public
// Inputs: pClient - Pointer to client's callback interface
//         rManifest - Manifest describing Service
//         rTranID - Optional TransactionID
//         ThisThread - btBool indicating if proc should run in this thread
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::allocService(  Runtime                *pProxy,
                              IBase                  *pServiceClient,
                              NamedValueSet const    &rManifest,
                              TransactionID const    &rTranID)
{
   AutoLock(this);

   // Make sure the Proxy is valid
   ClientMap_itr cmItr = m_mClientMap.find(pProxy);
   if(cmItr == m_mClientMap.end()){
      SendMsg(new RuntimeCallback(RuntimeCallback::AllocateFailed,
                                  m_pOwnerClient,
                                  new CExceptionTransactionEvent(NULL,
                                                                 extranevtServiceAllocateFailed,
                                                                 rTranID,
                                                                 errBadParameter,
                                                                 reasInvalidParameter,
                                                                 "Runtime Proxy invalid")),
                                  NULL);
      return;

   }

   // Get the Runtime Client associated with the Runtime Proxy used to issue this request.
   //   Runtime messages will be sent to this Client.
   IRuntimeClient *pRuntimeClient = cmItr->second;

   if ( IsOK() ) {
      // Send the Broker the allocation request
      m_pBroker->allocService(pProxy, pRuntimeClient, pServiceClient, rManifest, rTranID);
   }
}

//=============================================================================
// Name: schedDispatchable
// Description: Schedule an IDispatchable
// Interface: public
// Inputs: pdispatchable - pointer to IDispatchable
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::schedDispatchable(IDispatchable *pdispatchable)
{
   // TODO 2nd parm deprecated
   SendMsg(pdispatchable, NULL);
}


//
// IServiceClient Interface
//-------------------------
//=============================================================================
// Name: serviceAllocated
// Description: Called when a Service has been allocated through
//              allocateService())
// Interface: public
// Inputs: pClient - Pointer to client's callback interface
//         rManifest - Manifest describing Service
//         rTranID - Optional TransactionID
//         ThisThread - btBool indicating if proc should run in this thread
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::serviceAllocated(IBase               *pServiceBase,
                                  TransactionID const &rTranID )
{
   AutoLock(this);

   switch ( rTranID.ID() ) {
      case MDS : {
         m_pMDSbase = pServiceBase;
         m_pMDS     = subclass_ptr<IEventDeliveryService>(pServiceBase);
         m_sem.Post(1);
      } break;

      case Broker : {
         // Replace the Broker
         m_pBrokerbase = pServiceBase;
         m_pBroker     = subclass_ptr<IServiceBroker>(pServiceBase);
         m_sem.Post(1);
      } break;

      default :
         ASSERT(false);
      break;
   }
}

//=============================================================================
// Name: serviceAllocatedFailed
// Description: Allocation of a service failed
// Interface: public
// Inputs: rEvent - Exception Event
//         rTranID - Optional TransactionID
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::serviceAllocateFailed(const IEvent        &rEvent)
{
   m_status = false;
   m_sem.Post(1);
}

//=============================================================================
// Name: serviceReleased
// Description: Service has been Released
// Interface: public
// Inputs: pServiceBase - Service that was freed
//         rTranID - Optional TransactionID
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::serviceReleased(TransactionID const &rTranID)
{
   AutoLock(this);

   switch ( rTranID.ID() ) {
      case Broker : {
         // Don't delete here. Taken care of by ServiceBase::Release().
         m_pBroker     = NULL;
         m_pBrokerbase = NULL;

         m_pMDS->StopEventDelivery();
         m_state = Stopped;

         // Dispatch the event ourselves, because MDS is no more.
         OSLThreadGroup oneShot;
         RuntimeCallback *pRuntimeStopped = new RuntimeCallback(RuntimeCallback::Stopped,
                                                                m_pOwnerClient,
                                                                m_pOwner);

          // Fire the final event
         oneShot.Add(pRuntimeStopped);
         oneShot.Drain();  // Wait for it to be dispatched
      } break;

      default :
         ASSERT(false);
      break;
   }
}

//=============================================================================
// Name: serviceReleasedfailed
// Description: Service Release failed
// Interface: public
// Inputs: rEvent - Event detailing failure
// Outputs: none.
// Comments: Not much to do but forward
//=============================================================================
void _runtime::serviceReleaseFailed(const IEvent &rEvent)
{
   AutoLock(this);

   m_pBroker     = NULL;
   m_pBrokerbase = NULL;

   m_pMDS->StopEventDelivery();
   m_state = Stopped;

   // Copy the exception event as the original will be destroyed when we return
   IExceptionTransactionEvent *pExevent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);

   CExceptionTransactionEvent *pcopyEvent = new CExceptionTransactionEvent(this,
                                                                           pExevent->TranID(),
                                                                           pExevent->ExceptionNumber(),
                                                                           pExevent->Reason(),
                                                                           pExevent->Description());

   // Dispatch the event ourselves, because MDS is no more.
   OSLThreadGroup oneShot(1,1);  // Make sure we use a single thread to serialize the events
   RuntimeCallback *pRuntimeException = new RuntimeCallback(RuntimeCallback::Event,
                                                            m_pOwnerClient,
                                                            pcopyEvent);


   RuntimeCallback *pRuntimeStopped = new RuntimeCallback(RuntimeCallback::Stopped,
                                                          m_pOwnerClient,
                                                          m_pOwner);
   // Fire the final events
   oneShot.Add(pRuntimeException);
   oneShot.Add(pRuntimeStopped);

   oneShot.Drain();  // Wait for it to be dispatched
}


//=============================================================================
// Name: messageHandler
// Description: Unsolicited Event or Message handler
// Interface: public
// Inputs: rEvent - The event
// Outputs: none.
// Comments:
//=============================================================================
// Message Handler
//   Input: rEvent - Event contains message/event.  Typically used for
//          exceptions or events for which no standard callback is defined.
void _runtime::serviceEvent(const IEvent &rEvent)
{
   // TODO
   ASSERT(false);
}



// IXLRuntimeServices
//=============================================================================
// Name: getMessageDeliveryService
// Description: Get the base interface to teh MDS
// Interface: public
// Comments:
//=============================================================================
IBase *_runtime::getMessageDeliveryService()
{
   AutoLock(this);
   return m_pMDSbase;
}

//=============================================================================
// Name: messageHandler
// Description: Unsolicited Event or Message handler
// Interface: public
// Inputs: rEvent - The event
// Outputs: none.
// Comments:
//=============================================================================
void _runtime::setMessageDeliveryService(IBase *pMDSbase)
{
   AutoLock(this);
   m_pMDSbase = pMDSbase;
}

//=============================================================================
// Name: SendMsg
// Description:Send a message
// Interface: public
// Inputs: pobject - Dispatchable object to send
//         parm - Parameter
// Outputs: none.
// Comments:
//=============================================================================
btBool _runtime::SendMsg(IDispatchable *pobject, btObjectType parm)
{
   AutoLock(this);
   if ( NULL == m_pMDSbase ) {
      return false;
   }
   subclass_ref<IEventDeliveryService>(m_pMDSbase).QueueEvent(parm, pobject);
   return true;         // TODO cleanup IEventdeliveryService
}

//=============================================================================
// Name: getRuntimeClient
// Description: return the Runtime's Client's Interface
// Interface: public
// Outputs: Pointer to client interface.
// Comments:
//=============================================================================
IRuntimeClient *_runtime::getRuntimeClient()
{
   AutoLock(this);
   return m_pClient;
}



//=============================================================================
// Name: ~_runtime
// Description: Destructor
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
_runtime::~_runtime()
{
   AutoLock(this);

   // Check for proxies
   if(!m_mClientMap.empty()){
      OSLThreadGroup oneShot;
      ClientMap_itr cmIntr = m_mClientMap.begin();

      // Remove all Proxies, notfying them of the destruction if needed
      while( cmIntr != m_mClientMap.end()){
         // The owner (creator) of the runtime does not need to be notified
         if(cmIntr->first != m_pOwner){
            // Notify the Proxy owner that the proxy is dead.
             RuntimeCallback *pRuntimeRelease = new RuntimeCallback(RuntimeCallback::Event,
                                                                    cmIntr->second,
                                                                    new CExceptionTransactionEvent( cmIntr->first,            // The Proxy is in the event
                                                                                                    extranevtProxyStopped,
                                                                                                    TransactionID(),
                                                                                                    errProxyInvalid,
                                                                                                    reasParentReleased,
                                                                                                    "Parent Runtime destroyed while proxy still outstanding!"));

             oneShot.Add(pRuntimeRelease);
          }
          ++cmIntr;
      }

      oneShot.Drain();  // Wait for them to be dispatched

      // Empty the map. We cannot destroy proxy objects.
      m_mClientMap.clear();
   }

   if ( m_pMDSSvcHost ) {
      m_pMDS->StopEventDelivery();
      delete m_pMDSSvcHost;
      m_pMDSSvcHost = NULL;
   }
   if ( m_pBrokerSvcHost ) {
      delete m_pBrokerSvcHost;
      m_pBrokerSvcHost = NULL;
   }
}


END_NAMESPACE(AAL)

