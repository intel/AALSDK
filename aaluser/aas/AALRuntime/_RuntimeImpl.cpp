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
//        FILE: _RuntimeImpl.cpp
//     CREATED: Mar 7, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: Implementation of AAL _runtime class
// HISTORY:
// COMMENTS: This is the internal implementation of the runtime
// WHEN:          WHO:     WHAT:
// 06/25/2015     JG       Removed XL from name
// 07/01/2015     JG       Redesigned RUntime Proxy structure.
//                            MDS is no longer a Service.
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// Runtime definitions
#include "aalsdk/AALTypes.h"
#include "aalsdk/aas/ServiceHost.h"
#include "aalsdk/Dispatchables.h"

#include "_RuntimeImpl.h"
#include "_MessageDelivery.h"
#include "_ServiceBroker.h"

#include "aalsdk/osal/Sleep.h"
#include "aalsdk/osal/Env.h"

#include "aalsdk/INTCDefs.h"
#include "aalsdk/CAALEvent.h"

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

// Singleton Runtime implementation
static _runtime       *pTheRuntime = NULL;
static CriticalSection TheRuntimeMtx;

//=============================================================================
// Name: _getnewRuntimeInstance
// Description: Factory for the singleton Runtime Implementation
// Interface: private
// Inputs: pRuntimeProxy - Pointer to the Runtime Container (Proxy)
//         pClient - Pointer to the client for this instance.
// Outputs: pointer to Runtime implementation.
// Comments: The Runtime implementation keeps a map of Proxy to Clients
//           so that multiple Proxy's can point to the same Runtime and
//           messages are routed appropriately.
//=============================================================================
_runtime * _getnewRuntimeInstance(Runtime        *pRuntimeProxy,
                                  IRuntimeClient *pClient,
                                  btBool          bFirstTime)
{
   // If There is no client then there isn't much
   //  to do but silently fail.
   ASSERT(NULL != pClient);
   if ( NULL == pClient ) {
      AAL_ERR(LM_AAS, "Missing IRuntimeClient on Runtime construction");
      return NULL;
   }

   IDispatchable *pDisp = NULL;

   // If missing the Proxy fail
   ASSERT(NULL != pRuntimeProxy);
   if ( NULL == pRuntimeProxy ) {
      // Fire the final event, waiting for it to dispatch.
      // Dispatch the event ourselves, because MDS not available.

      pDisp = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                       pClient,
                                       new CExceptionTransactionEvent(NULL,
                                                                      extranevtRuntimeCreateorProxy,
                                                                      TransactionID(),
                                                                      errCreationFailure,
                                                                      reasMissingParameter,
                                                                      "Failed to instantiate Runtime"));
      goto _DISP;
   }

   {
      AutoLock(&TheRuntimeMtx);

      // If there is already a runtime then can't be First Time through
      if ( ( NULL != pTheRuntime ) && bFirstTime ) {
         // Tried to instantiate a new Runtime after one was already created.

         pDisp = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                     pClient,
                                     new CExceptionTransactionEvent(NULL,
                                                                    extranevtRuntimeCreateorProxy,
                                                                    TransactionID(),
                                                                    errCreationFailure,
                                                                    reasSingletoneExists,
                                                                    "Failed to instantiate Runtime. Cannot instantiate multiple Runtimes. Use getRuntimeProxy()!"));
         goto _DISP;
      }

      // If there is already a runtime then can't be First Time through
      if ( ( NULL == pTheRuntime ) && !bFirstTime ) {
         // Tried to instantiate a new Runtime after one was already created.

         pDisp = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                     pClient,
                                     new CExceptionTransactionEvent(NULL,
                                                                    extranevtRuntimeCreateorProxy,
                                                                    TransactionID(),
                                                                    errCreationFailure,
                                                                    reasParameterValueInvalid,
                                                                    "Failed to instantiate Runtime. No Runtime instance with FirstTime set to true!"));
         goto _DISP;
      }

      if ( NULL == pTheRuntime ) {
         pTheRuntime = new _runtime(pRuntimeProxy, pClient);
      }

      // Connect this client and proxy to the runtime
      pTheRuntime->addProxy(pRuntimeProxy, pClient);

      if ( !pTheRuntime->IsOK() ) {
         // Dispatch the event ourselves, because MDS is no more.

         pDisp = new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                     pClient,
                                     new CExceptionTransactionEvent(NULL,
                                                                    extranevtRuntimeCreateorProxy,
                                                                    TransactionID(),
                                                                    errCreationFailure,
                                                                    reasCauseUnknown,
                                                                    "Failed to instantiate Runtime"));
         goto _DISP;
      }

      return pTheRuntime;
   }

_DISP:
   ASSERT(NULL != pDisp);
   if ( NULL != pDisp ) {
      // Fire the final event, waiting for it to dispatch.
      FireAndWait(pDisp);
   }
   return NULL;
}

END_C_DECLS

//=============================================================================
// Name: _runtime
// Description: Constructor
// Interface: public
// Outputs: none.
// Comments:
//=============================================================================
_runtime::_runtime(Runtime *pRuntimeProxy, IRuntimeClient *pClient) :
   CAASBase(),
   m_state(Stopped),
   m_pClient(this),              // Must be our own client for internal services
   m_pOwner(pRuntimeProxy),
   m_pOwnerClient(pClient),
   m_pProxy(NULL),
   m_pBrokerSvcHost(NULL),
   m_pBroker(NULL),
   m_pBrokerbase(NULL),
   m_pDefaultBrokerbase(NULL)
{
   m_sem.Create(0);

   // Register interfaces
   // Add the public interfaces
   if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ) {
      m_bIsOK = false;
      return;
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
   IDispatchable *pDisp = NULL;

   {
      AutoLock(this);

      if ( m_pOwner != pProxy ) {
         // Dispatch the event ourselves, because MDS Does not exist.
         pDisp = new RuntimeCallback(RuntimeCallback::StartFailed,
                                     m_pOwnerClient,
                                     pProxy,
                                     rConfigParms,
                                     new CExceptionTransactionEvent(pProxy,
                                                                    exttranevtSystemStart,
                                                                    TransactionID(),
                                                                    errSysSystemStarted,
                                                                    reasNotOwner,
                                                                    "Not using original Runtime"));
         goto _DISP;
      }

      if ( Started == m_state  ) {
         // Dispatch the event ourselves, because MDS Does not exist.
         pDisp = new RuntimeCallback(RuntimeCallback::StartFailed,
                                     m_pOwnerClient,
                                     pProxy,
                                     rConfigParms,
                                     new CExceptionTransactionEvent(pProxy,
                                                                    exttranevtSystemStart,
                                                                    TransactionID(),
                                                                    errSysSystemStarted,
                                                                    reasSystemAlreadyStarted,
                                                                    strSystemAlreadyStarted));
         goto _DISP;
      }

      // The Runtime needs a Runtime Proxy because it loads Services "much" like any application
      //  so it requires a RuntimeClient of its own.
      m_pProxy = m_pOwner->getRuntimeProxy(this);

      if ( NULL == m_pProxy ) {
         // Fire the event and wait for it to be dispatched.
         pDisp = new RuntimeCallback(RuntimeCallback::StartFailed,
                                     m_pOwnerClient,
                                     pProxy,
                                     rConfigParms,
                                     new CExceptionTransactionEvent(pProxy,
                                                                    exttranevtSystemStart,
                                                                    TransactionID(),
                                                                    errSysSystemStarted,
                                                                    reasInitError,
                                                                    "Unable to create Runtime Proxy"));
         m_bIsOK = false;
         goto _DISP;
      }

      // Set the status and continue
      if ( !m_pProxy->IsOK() ) {
         m_bIsOK = false;
      }

   }

   // InstallDefaults() will wait for a notification. Don't wait while locked..
   if ( !InstallDefaults() ) {
      // Fire the event and wait for it to be dispatched.
      pDisp = new RuntimeCallback(RuntimeCallback::StartFailed,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStart,
                                                                 TransactionID(),
                                                                 errSysSystemStarted,
                                                                 reasInitError,
                                                                 "Unable to load default Services"));
      goto _DISP;
   }

   // Process the configuration parameters will wait for a notification. Don't wait while locked..
   if ( !ProcessConfigParms(rConfigParms) ) {
      pDisp = new RuntimeCallback(RuntimeCallback::StartFailed,
                                  m_pOwnerClient,
                                  pProxy,
                                  rConfigParms,
                                  new CExceptionTransactionEvent(pProxy,
                                                                 exttranevtSystemStart,
                                                                 TransactionID(),
                                                                 errSysSystemStarted,
                                                                 reasInitError,
                                                                 "Unable to process Runtime configuration parameters"));
      goto _DISP;
   }

   if ( IsOK() ) {

      m_state = Started;

      schedDispatchable(new RuntimeCallback(RuntimeCallback::Started,
                                            m_pOwnerClient,
                                            pProxy,
                                            rConfigParms));

      return true;
   } else {
      // Runtime Failed to start

      schedDispatchable(new RuntimeCallback(RuntimeCallback::StartFailed,
                                            m_pOwnerClient,
                                            pProxy,
                                            rConfigParms,
                                            new CExceptionTransactionEvent(pProxy,
                                                                           exttranevtSystemStart,
                                                                           TransactionID(),
                                                                           errSysSystemStarted,
                                                                           reasInitError,
                                                                           "AAL Runtime Failed to start")));
      return false;
   }

_DISP:
   ASSERT(NULL != pDisp);
   if ( NULL != pDisp ) {
      // Fire the event and wait for it to be dispatched.
      FireAndWait(pDisp);
   }
   return false;
}

//=============================================================================
// Name: stop
// Description: Stop the runtime
// Interface: public
// Comments:
//=============================================================================
void _runtime::stop(Runtime *pProxy)
{
   {
      AutoLock(this);

      if ( NULL == pProxy ) {
         // Bad Parameter   TODO Check Return code
         schedDispatchable(new RuntimeCallback(RuntimeCallback::Event,
                                               m_pOwnerClient,
                                               new CExceptionTransactionEvent(m_pOwner,
                                                                              exttranevtSystemStop,
                                                                              TransactionID(),
                                                                              errBadParameter,
                                                                              reasMissingParameter,
                                                                              "NULL Proxy")));
         return;
      }

      if ( m_pOwner != pProxy ) {
         // Runtime Failed to stop. Can only stop original
         schedDispatchable(new RuntimeCallback(RuntimeCallback::StopFailed,
                                               m_pOwnerClient,
                                               new CExceptionTransactionEvent(pProxy,
                                                                              exttranevtSystemStop,
                                                                              TransactionID(),
                                                                              errSysSystemPermission,
                                                                              reasNotOwner,
                                                                              "Not using original Runtime")));
         return;
      }

      // If the runtime is OK but is NOT stopped
      if ( IsOK() && ( Stopped != m_state ) ) {

         // Prepare our sem. We will wait for notification from serviceReleased() before continuing.
         // (Don't wait while locked.)
         m_sem.Reset(0);

         // Release the Service Broker.
         goto _REL;
      }

      // Dispatch the event ourselves, because we may not have working MDS.
      goto _DISP;
   }

_REL:
   dynamic_ptr<IAALService>(iidService, m_pBrokerbase)->Release(TransactionID(Broker));
   return;

_DISP:
   // Fire the final event and wait for it to be dispatched.
   FireAndWait( new RuntimeCallback(RuntimeCallback::Stopped,
                                    m_pOwnerClient,
                                    pProxy) );
   return;
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
void _runtime::allocService(Runtime             *pProxy,
                            IBase               *pServiceClient,
                            NamedValueSet const &rManifest,
                            TransactionID const &rTranID)
{
   IDispatchable *pDisp = NULL;

   AutoLock(this);

   // Make sure the Proxy is valid
   ClientMap_itr cmItr = m_mClientMap.find(pProxy);

   if ( m_mClientMap.end() == cmItr ) {
      pDisp = new RuntimeCallback(RuntimeCallback::AllocateFailed,
                                  m_pOwnerClient,
                                  new CExceptionTransactionEvent(NULL,
                                                                 extranevtServiceAllocateFailed,
                                                                 rTranID,
                                                                 errBadParameter,
                                                                 reasInvalidParameter,
                                                                 "Runtime Proxy invalid"));
      goto _SCHED;
   }

   if ( !IsOK() ) {
      pDisp = new RuntimeCallback(RuntimeCallback::AllocateFailed,
                                  m_pOwnerClient,
                                  new CExceptionTransactionEvent(NULL,
                                                                 extranevtServiceAllocateFailed,
                                                                 rTranID,
                                                                 errInternal,
                                                                 reasCauseUnknown,
                                                                 "Runtime not OK"));
      goto _SCHED;
   }

   // cmItr->second is the Runtime Client associated with the Runtime Proxy used to issue this request.

   // Send the Broker the allocation request
   m_pBroker->allocService(pProxy, cmItr->second, pServiceClient, rManifest, rTranID);
   return;

_SCHED:
   ASSERT(NULL != pDisp);
   if ( NULL != pDisp ) {
      schedDispatchable(pDisp);
   }
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
void _runtime::allocService(IBase               *pClient,
                            NamedValueSet const &rManifest,
                            TransactionID const &rTranID)
{
   AutoLock(this);
   if ( IsOK() ) {
      // Used for internal allocations, this object is the Runtime and the RuntimeClient
      //  (first 2 arguments)
      m_pBroker->allocService(m_pProxy, this, pClient, rManifest, rTranID);
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
btBool _runtime::schedDispatchable(IDispatchable *pDispatchable)
{
   return m_MDS.scheduleMessage(pDispatchable);
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
// Name: IsOK
// Description: Is the object functional
// Interface: public
// returns: true if functional.
// Comments:
//=============================================================================
btBool _runtime::IsOK()
{
   return CAASBase::IsOK();
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
void _runtime::addProxy(Runtime        *pRuntimeProxy,
                        IRuntimeClient *pClient)
{
   if ( ( NULL == pRuntimeProxy ) || ( NULL == pClient ) ) {
      AAL_ERR(LM_AAS, "addProxy: NULL Proxy or Client");
      return;
   }

   AutoLock(this);

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
void _runtime::removeProxy(Runtime *pRuntimeProxy)
{
   ASSERT(NULL != pRuntimeProxy);
   if ( NULL == pRuntimeProxy ) {
      return;
   }

   AutoLock(this);

   // Erase it
   ClientMap_itr cmItr = m_mClientMap.find(pRuntimeProxy);

   ASSERT(m_mClientMap.end() != cmItr);
   if ( m_mClientMap.end() != cmItr ) {
      m_mClientMap.erase(cmItr);
   }
}

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
void _runtime::releaseRuntimeInstance(Runtime *pRuntimeProxy)
{
   IDispatchable *pDisp = NULL;

   {
      AutoLock(&TheRuntimeMtx);

      if ( NULL == pTheRuntime ) {
         AAL_ERR(LM_AAS, "releaseRuntimeInstance() called with no Runtime present");
         return;
      }

      // If missing the Proxy fail
      if ( NULL == pRuntimeProxy ) {
         // Dispatch the event ourselves, because MDS is no more.
         pDisp = new RuntimeCallback(RuntimeCallback::Event,
                                     m_pClient,
                                     new CExceptionTransactionEvent(NULL,
                                                                    extranevtRuntimeDestroyorRelease,
                                                                    TransactionID(),
                                                                    errReleaseFailure,
                                                                    reasMissingParameter,
                                                                    "Failed to release Runtime"));
         goto _DISP;
      }

      // If it's not the owner, then just removes this proxy.
      if ( pRuntimeProxy != m_pOwner ) {
         removeProxy(pRuntimeProxy);
         return;
      }

      if ( IsOK() && ( Stopped != m_state ) ) {
         // Stop and clean up properly  TODO
      }

      delete this;
      pTheRuntime = NULL;
   }

   return;

_DISP:

   ASSERT(NULL != pDisp);
   if ( NULL != pDisp ) {
      // Fire the final event and wait for it to be dispatched.
      FireAndWait(pDisp);
   }
}

_runtime::_runtime() {/*empty*/}

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

#if ENABLE_DEBUG
   std::cerr << "Num Proxies: " << m_mClientMap.size() << std::endl;
#endif // ENABLE_DEBUG

#if 0
   // Check for proxies
   if(!m_mClientMap.empty()){
      OSLThreadGroup oneShot;
      ClientMap_itr cmIntr = m_mClientMap.begin();

      // Remove all Proxies, notfying them of the destruction if needed
      while( cmIntr != m_mClientMap.end()){
         // The owner (creator) of the runtime does not need to be notified
         if(cmIntr->first != m_pOwner){
            // Notify the Proxy owner that the proxy is dead.  NOTE that this is presented as an Event.
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
#endif

   m_MDS.StopMessageDelivery();

   if ( NULL != m_pBrokerSvcHost ) {
      delete m_pBrokerSvcHost;
      m_pBrokerSvcHost = NULL;
   }
}

//=============================================================================
// Name: InstallDefaults
// Description: Install Default Services
// Interface: public
// Outputs: none.
// Comments:
//=============================================================================
btBool _runtime::InstallDefaults()
{
   // Message Delivery Service

   // Service Broker. The m_Proxy is _runtime's Proxy which has a pointer to _runtime's IRuntimeClient
   m_pBrokerSvcHost = new ServiceHost(AAL_SVC_MOD_ENTRY_POINT(localServiceBroker));
   m_pBrokerSvcHost->InstantiateService(m_pProxy, dynamic_cast<IBase *>(this), NamedValueSet(), TransactionID(Broker));

   m_sem.Wait(); // for the local Broker

   if ( IsOK() ) {
      return true;
   }

   // Dispatch the event ourselves, because MDS is no more.

   // Fire the final event and wait for it to be dispatched.
   FireAndWait( new RuntimeCallback(RuntimeCallback::CreateorGetProxyFailed,
                                    m_pOwnerClient,
                                    new CExceptionTransactionEvent(m_pOwner,
                                                                   extranevtRuntimeCreateorProxy,
                                                                   TransactionID(),
                                                                   errCreationFailure,
                                                                   reasSubModuleFailed,
                                                                   "Failed to load Broker Service")) );
   return false;
}

//=============================================================================
// Name: ProcessConfigParms
// Description: Process config parms for Runtime configuration record
// Interface: public
// Inputs: rConfigParms - Config parms
// Outputs: none.
// Comments:
//=============================================================================
btBool _runtime::ProcessConfigParms(const NamedValueSet &rConfigParms)
{
   INamedValueSet const *pConfigRecord = NULL;
   btcString             sName         = NULL;
   std::string           strSname;

   // First check environment
   if ( Environment::GetObj()->Get("AALRUNTIME_CONFIG_BROKER_SERVICE", strSname) ) {
      sName = strSname.c_str();
   } else {
      // Not in environment so check config parameters

      if ( ENamedValuesOK != rConfigParms.Get(AALRUNTIME_CONFIG_RECORD, &pConfigRecord) ) {
         // Not in Environment and no Config Parms.
         return true;
      }

      // See if the Broker is being overridden
      if ( ENamedValuesOK == pConfigRecord->Get(AALRUNTIME_CONFIG_BROKER_SERVICE, &sName) ) {
         // No default Broker so we can't load ANY new Services
         if ( NULL == m_pBrokerbase ) {
            // Runtime Failed to start
            schedDispatchable(new RuntimeCallback(RuntimeCallback::StartFailed,
                                                  m_pOwnerClient,
                                                  m_pOwner,
                                                  rConfigParms,
                                                  new CExceptionTransactionEvent(m_pOwner,
                                                                                 exttranevtServiceShutdown,
                                                                                 TransactionID(),
                                                                                 errSysSystemStarted,
                                                                                 reasInitError,
                                                                                 "AAL Runtime Failed to start - No Broker")));
            return false;
         }
      }
   }

   if ( NULL != sName ) {
      // Create the Service Request
      NamedValueSet ConfigRecord;
      NamedValueSet optArgs;

      // Using back door because thats all we know.
      ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, sName);
      optArgs.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

      // Allocate the service.
      allocService(this, optArgs, TransactionID(Broker));
      m_sem.Wait();
   }

   return true;
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
                                TransactionID const &rTranID)
{
   AutoLock(this);

   switch ( rTranID.ID() ) {
      case Broker : {
         // If there is already a broker, Replace it.
         if ( NULL != m_pBrokerbase ) {
            // Save a copy of the pointer .
            //  We cannot Release the default Broker now since that would cause the new Broker and any other Service
            //  allocated using the default to be Released. Release it later at stop()
            m_pDefaultBrokerbase = m_pBrokerbase;
         }

         m_pBrokerbase = pServiceBase;
         m_pBroker     = subclass_ptr<IServiceBroker>(m_pBrokerbase);

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
void _runtime::serviceAllocateFailed(const IEvent &rEvent)
{
   AutoLock(this);
   m_bIsOK = false;
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
         // If the Default Broker was replaced then release it now.  Next time through we will clean up.
         if ( NULL != m_pDefaultBrokerbase ) {
            // Release the Service Broker.
            dynamic_ptr<IAALService>(iidService, m_pDefaultBrokerbase)->Release(TransactionID(Broker));
            m_pDefaultBrokerbase = NULL;
            return;
         }

         // Shutting down.

         // Don't delete here. Taken care of by ServiceBase::Release().
         m_pBroker     = NULL;
         m_pBrokerbase = NULL;

         m_MDS.StopMessageDelivery();

         m_state = Stopped;

         // Release our Proxy
         m_pProxy->releaseRuntimeProxy();

         // Dispatch the event ourselves, because MDS is no more.

         // Fire and final event and wait for it to be dispatched.
         FireAndWait( new RuntimeCallback(RuntimeCallback::Stopped,
                                          m_pOwnerClient,
                                          m_pOwner) );
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

   m_MDS.StopMessageDelivery();
   m_state = Stopped;

   // Copy the exception event as the original will be destroyed when we return
   IExceptionTransactionEvent *pExevent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);

   CExceptionTransactionEvent *pcopyEvent = new CExceptionTransactionEvent(this,
                                                                           pExevent->TranID(),
                                                                           pExevent->ExceptionNumber(),
                                                                           pExevent->Reason(),
                                                                           pExevent->Description());

   // Dispatch the event ourselves, because MDS is no more.

   // Make sure to serialize the events.
   FireAndWait(new RuntimeCallback(RuntimeCallback::Event,
                                   m_pOwnerClient,
                                   pcopyEvent),
               1,
               1);


   FireAndWait(new RuntimeCallback(RuntimeCallback::Stopped,
                                   m_pOwnerClient,
                                   m_pOwner),
               1,
               1);
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

END_NAMESPACE(AAL)

