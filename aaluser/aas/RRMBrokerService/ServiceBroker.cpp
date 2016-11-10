// Copyright(c) 2014-2016, Intel Corporation
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
//        FILE: ServiceBroker.cpp
//     CREATED: April 4, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Implements an XL Sample Service Broker Service plug-in.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/osal/OSServiceModule.h"
#include "aalsdk/aas/AALInProcServiceFactory.h"  // Defines InProc Service Factory
#include "aalsdk/aas/Dispatchables.h"
#include "aalsdk/aas/ServiceHost.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/AALLoggerExtern.h"              // AAL Logger

#include "ServiceBroker.h"
#include "aalsdk/osal/Sleep.h"

#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::ServiceBroker >

#if defined ( __AAL_WINDOWS__ )                                               
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, librrmbroker, RRMBROKER_API, RRMBROKER_VERSION, RRMBROKER_VERSION_CURRENT, RRMBROKER_VERSION_REVISION, RRMBROKER_VERSION_AGE)
   // Only default commands for now.
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )                                               
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)

struct shutdown_thread_parms
{
   shutdown_thread_parms(ServiceBroker       *pfact,
                         TransactionID const &rTranID,
                         btTime               timeout) :
      m_this(pfact),
      m_rTranID(rTranID),
      m_timeout(timeout)
   {}

   ServiceBroker *m_this;
   TransactionID  m_rTranID;
   btTime         m_timeout;
};

//=============================================================================
// Name: init
// Description: Initialize the object
// Interface: public
// Inputs: rtid - reference to a transaction ID
// Comments:
//   This is called via the base class construction chain.  Since this class is
//   derived from ServiceBase it can assume that all of the base members have
//.  been initialized.
//=============================================================================
btBool ServiceBroker::init(IBase *pclientBase,
                           NamedValueSet const &optArgs,
                           TransactionID const &rtid)
{
   // The Resource Manager is implemented as an AAL Service however rather
   //  than going through the AALRUNTIME and default broker to aqcuire it
   //  using a ServiceHost allows us to specifically plug in the built-in
   //  implementation. NOTE: This is  the way the default runtime services
   //  are bootstrapped.
   m_pRMSvcHost = new ServiceHost(AAL_BUILTIN_SVC_MOD_ENTRY_POINT(librrm));

   //Allocate the service.

   // Default constructor creates unique TID. This will be used later to re-acquire the clients tid
   //  to generate the response message

   TransactionID tid = TransactionID();
   m_Transactions[tid] = rtid;
   if ( !m_pRMSvcHost->InstantiateService( getRuntime(),
                                           dynamic_cast<IBase*>(this), optArgs, tid ) ) {
      // Remove pending transaction
      m_Transactions.erase(tid);
      initFailed( new CExceptionTransactionEvent( NULL,
                                                  rtid,
                                                  errServiceNotFound,
                                                  reasUnknown,
                                                  "Could not allocate ResourceManager.  Possible bad argument or missing client interface.") );
      return false;
   }
   return true;

}

//=============================================================================
// Name: ~ServiceBroker
// Description: IDestructor
// Interface: public
// Comments:
//   Delete any Service Hosts. They won't be used anymore
//=============================================================================
ServiceBroker::~ServiceBroker()
{
   Servicemap_itr itr;

   btUnsigned32bitInt size = static_cast<btUnsigned32bitInt>(m_ServiceMap.size());
   if ( 0 == size ) {
      return;
   }

   for ( itr = m_ServiceMap.begin() ; size ; size--, itr++ ) {
      // If the IServiceModule is present
      delete ( *itr ).second;
   }
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
void ServiceBroker::serviceAllocated(IBase               *pServiceBase,
                                     TransactionID const &rTranID)
{
   AutoLock(this);

   TransactionID origTid = m_Transactions[rTranID];
   m_Transactions.erase(rTranID);

   m_ResMgrBase = pServiceBase;
   m_ResMgr     = dynamic_ptr<IResourceManager>(iidResMgr, pServiceBase);
   if ( NULL == m_ResMgr ) {
      initFailed( new CExceptionTransactionEvent( NULL,
                                                  origTid,
                                                  errMethodNotImplemented,
                                                  reasNotImplemented,
                                                  "Service does not support IResourceManager") );
      return;
   }

   m_bIsOK = true;
   initComplete(origTid);
   return;
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
void ServiceBroker::serviceAllocateFailed(const IEvent &rEvent)
{
   TransactionID TranID = dynamic_ref<ITransactionEvent>(iidTranEvent,rEvent).TranID();
   TransactionID origTid = m_Transactions[TranID];
   m_Transactions.erase(TranID);

   // Print an error message. If the reason for failure was "reasNoDevice",
   //  meaning that we couldn't establish a connection to the remote resource
   //  manager, the most common reason is that there's no driver loaded.

   IExceptionTransactionEvent *evt =
         dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);

   if ( NULL != evt ) {
      AAL_CRIT( LM_All, "Failed to allocate local resource manager: " <<
            evt->Description() << std::endl );
      if ( evt->Reason() == reasNoDevice ) {
         AAL_CRIT( LM_All, "Did you load the drivers?" << std::endl );
      }
      // If we were unable to load the ResourceManager then we cannot load.
      initFailed( new CExceptionTransactionEvent( this,
                                                  origTid,
                                                  evt->ExceptionNumber(),
                                                  evt->Reason(),
                                                  evt->Description()) );
   } else {
      // Not an ExceptionTransactionEvent -- do the best we can.
      initFailed( new CExceptionTransactionEvent( this,
                                       origTid,
                                       errServiceNotFound,
                                       reasInvalidService,
                                       strInvalidService) );
   }

} // ServiceBroker::serviceAllocateFailed()

//=============================================================================
// Name: serviceReleased
// Description: Service has been Released
// Interface: public
// Inputs: pServiceBase - Service that was released
//         rTranID - Optional TransactionID
// Outputs: none.
// Comments:
//=============================================================================
void ServiceBroker::serviceReleased(TransactionID const &rTranID)
{
   AutoLock(this);

   btTime timeout = -1;

   struct shutdown_thread_parms *pparms =
                                    new struct shutdown_thread_parms(this,
                                                                     rTranID,
                                                                     timeout);
   // FIXME: get original timeout

   //--------------------------------------------
   // Create the Shutdown thread object
   //  The Shutdown thread is self destructive so
   //  no need to keep pointer nor do a Join()
   //  as the ~OSLThread will clean up
   //  resources from unjoined threads
   //--------------------------------------------

   // Important to Lock here and in thread to ensure that the assignment
   //  is complete before thread runs.
   {
      AutoLock(this);
      m_pShutdownThread = new OSLThread(ServiceBroker::ShutdownThread,
                                        OSLThread::THREADPRIORITY_NORMAL,
                                        pparms);
   }

#if 0    // old SDK
   // Resource Manager is Released. Get the original TID from rTranID
   TransactionID tid = *(reinterpret_cast<TransactionID *>(rTranID.Context()));

   // Reasource Manager Proxy is gone. Generate the event
   getRuntime()->schedDispatchable(new ServiceReleased(getServiceClient(),
                                                       this,
                                                       tid));
#endif
}

//=============================================================================
// Name: serviceReleaseFailed
// Description: Service has been Released
// Interface: public
// Inputs: pServiceBase - Service that was released
//         rTranID - Optional TransactionID
// Outputs: none.
// Comments:
//=============================================================================
void ServiceBroker::serviceReleaseFailed(const IEvent &rEvent)
{
   AutoLock(this);

   // Copy the exception event as the original will be destroyed when we return
   IExceptionTransactionEvent *pExevent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);

   CExceptionTransactionEvent *pcopyEvent = new CExceptionTransactionEvent(this,
                                                                           pExevent->TranID(),
                                                                           pExevent->ExceptionNumber(),
                                                                           pExevent->Reason(),
                                                                           pExevent->Description());


   // Notify the client
   getRuntime()->schedDispatchable( new ServiceReleaseFailed(getServiceClient(),
                                                             pcopyEvent) );
}

//=============================================================================
// Name: allocService
// Description: Allocates a Service
// Interface: public
// Inputs:  pServiceClient - Pointer to the standard Service Client interface
// Comments:
//=============================================================================
void ServiceBroker::allocService(IRuntime               *pProxy,
                                 IRuntimeClient         *pRuntimeClient,
                                 IBase                  *pServiceClientBase,
                                 const NamedValueSet    &rManifest,
                                 TransactionID const    &rTranID)
{
   // Process the manifest
   btcString             sName        = NULL;
   INamedValueSet const *ConfigRecord = NULL;

   IServiceClient      *pServiceClient = dynamic_ptr<IServiceClient>(iidServiceClient, pServiceClientBase);
   if ( NULL == pServiceClient ) { // TODO replace all ObjectCreatedExceptionEvents with RuntimeCallbacks
      getRuntime()->schedDispatchable(new ServiceAllocateFailed(pServiceClient,
                                                                pRuntimeClient,
                                                                new CExceptionTransactionEvent(NULL,
                                                                                               rTranID,
                                                                                               errAllocationFailure,
                                                                                               reasMissingInterface,
                                                                                               strMissingInterface)));
      return;
   }

   if ( ENamedValuesOK != rManifest.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord) ) {
      getRuntime()->schedDispatchable(new ServiceAllocateFailed(pServiceClient,
                                                                pRuntimeClient,
                                                                new CExceptionTransactionEvent(NULL,
                                                                                               rTranID,
                                                                                               errAllocationFailure,
                                                                                               reasBadConfiguration,
                                                                                               "Missing Config Record")));
      return;
   }

   if ( ENamedValuesOK != ConfigRecord->Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, &sName) ) {
      getRuntime()->schedDispatchable(new ServiceAllocateFailed(pServiceClient,
                                                                pRuntimeClient,
                                                                new CExceptionTransactionEvent(NULL,
                                                                                               rTranID,
                                                                                               errAllocationFailure,
                                                                                               reasBadConfiguration,
                                                                                               "Missing Config RecordService Name")));
      return;
   }

   // Determine whether we need to consult Resource Manager
   btBool SWService = false;
   if ( ConfigRecord->Has(AAL_FACTORY_CREATE_SOFTWARE_SERVICE) ){
      ConfigRecord->Get(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, &SWService);
   }
   // If this Service is not pure software then use Resource Manager
   if(false == SWService){
      if ( NULL != m_ResMgr ) {

         // Need to save the Runtime Proxy and Client interfaces to be able to generate the final event
         TransactionID tid;
         m_ServiceClientMap[tid].ServiceBase = pServiceClientBase;
         m_ServiceClientMap[tid].pProxy = pProxy;
         m_ServiceClientMap[tid].pRuntimeClient = pRuntimeClient;
         m_Transactions[tid] = rTranID;

         m_ResMgr->RequestResource(rManifest, tid );

      } else {
         // TODO Throw error
      }
      return;
   } else {
      ServiceHost *SvcHost      = NULL;
      btBool       AllocSvcHost = false;

      if ( NULL == (SvcHost = findServiceHost(sName)) ) {
         // Instantiate the core facilities
         AllocSvcHost = true;
         SvcHost      = new ServiceHost(sName);
      }

      if ( !SvcHost->IsOK() ) {
         getRuntime()->schedDispatchable( new ServiceAllocateFailed(pServiceClient,
                                                                    pRuntimeClient,
                                                                    new CExceptionTransactionEvent(NULL,
                                                                                                   rTranID,
                                                                                                   errCreationFailure,
                                                                                                   reasInternalError,
                                                                                                   "Failed to load Service")));
      }

      // Allocate the service
      if ( !SvcHost->InstantiateService( getRuntime(), pServiceClientBase, rManifest, rTranID) ) {

         if ( AllocSvcHost ) {
            delete SvcHost;
         }

         getRuntime()->schedDispatchable( new ServiceAllocateFailed(pServiceClient,
                                                                    pRuntimeClient,
                                                                    new CExceptionTransactionEvent(NULL,
                                                                                                   rTranID,
                                                                                                   errCreationFailure,
                                                                                                   reasInternalError,
                                                                                                   "Failed to construct Service")));
      } else {
         // Save the ServiceHost
         m_ServiceMap[std::string(sName)] = SvcHost;
      }
   }
}


//=============================================================================
// Name: findServiceHost
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
ServiceHost * ServiceBroker::findServiceHost(std::string const &sName)
{
   Servicemap_itr itr = m_ServiceMap.find(sName);
   if ( m_ServiceMap.end() == itr ) {
      return NULL;
   }
   return itr->second;
}

//=============================================================================
//
// Service Broker Shutdown is a complex process that involves shutting down
// all of the plug-in services started through it.  Shutdown in an asynchronous
// function that requires synchronization of the completion events of all of
// services being shutdown before a final completion can be sent.
//
// The synchronization is performed by a worker thread so as not to block the
// calling thread to Shutdown(). The Shutdown() function creates the worker
// thread which executes ShutdownThread() passing the input args. via a
// structure called shutdown_thread_parms. The thread function forwards the
// call to the DoShutdown() method on the Service Factory which does the bulk
// of the work.
//
// DoShutdown() iterates through the list of registered services and invokes
// Shutdown method on their IServiceModule interface. It then waits on a
// count-up semaphore waiting for each service to report that they have
// shutdown or until the timeout expires. It then generates and event notifying
// AAL core that it has completed.
//------------------------------------------------------------------------------


//=============================================================================
// Name: Release
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
btBool ServiceBroker::Release(TransactionID const &rTranID, btTime timeout)
{
   // save contents of rTranID for proper release in DoShutdown()
   m_releaseTid = rTranID;

   // TODO: follow down release chain of tunneled TransactionIDs (it's lost somewhere...)

   if (m_ResMgrBase != NULL) {
      // copy the contents of the rTranID for later and store in a new TransactionID
      // for this release transaction
      TransactionID tid(static_cast<btApplicationContext>(new TransactionID(rTranID)));

      dynamic_ptr<IAALService>(iidService, m_ResMgrBase)->Release(tid, timeout);
   }

   return true;
}


//=============================================================================
// Name: ShutdownThread
// Description: Thread used to wait for the system to shutdown
// Interface: public
// Inputs: pThread - thread object
//         pContext - context - contains parameters for operation
// Outputs: none.
// Comments:
//=============================================================================
void ServiceBroker::ShutdownThread(OSLThread *pThread,
                                   void      *pContext)
{
   //Get a pointer to this objects context
   struct shutdown_thread_parms *pparms = static_cast<struct shutdown_thread_parms *>(pContext);
   ServiceBroker *This = static_cast<ServiceBroker *>(pparms->m_this);
   This->DoShutdown(pparms->m_rTranID, pparms->m_timeout);

   // Destroy parms and encapsulated context
   delete static_cast<TransactionID *>(pparms->m_rTranID.Context());
   delete pparms;
}

struct shutdown_handler_thread_parms
{
shutdown_handler_thread_parms(ServiceBroker                 *pfact,
                              ServiceBroker::Servicemap_itr  itr,
                              CSemaphore                    *srvcCount,
                              btTime                         timeout) :
   m_this(pfact),
   m_itr(itr),
   m_timeout(timeout),
   m_srvcCount(srvcCount)
{}

   ServiceBroker                 *m_this;
   ServiceBroker::Servicemap_itr  m_itr;
   btTime                         m_timeout;
   CSemaphore                    *m_srvcCount;
};

//=============================================================================
// Name:          DoShutdown
// Description:   This is the work horse of Shutdown
// Interface:     public
// Inputs:        rTranID,
//                timeout - Max time hint
// Outputs:
// Comments:      Timeout currently not accurate but used as a hint
//                Needs to call Shutdown on all of the loaded services in
//                m_SrvcPkgMap, with asynchronous returns. Need timeouts to
//                enable recovery in the case of one of them hanging.
//=============================================================================
btBool ServiceBroker::DoShutdown(TransactionID const &rTranID,
                                 btTime               timeout)
{
   CSemaphore     srvcCount;
   Servicemap_itr itr;
   btBool         ret = false;

   btUnsigned32bitInt size = m_servicecount = static_cast<btUnsigned32bitInt>(m_ServiceMap.size());
   if ( 0 == size ) {
      timeout = 0;
   }

   // Initialize the semaphore as a count up by initializing
   //  count to a negative number.
   //  The waiter will block until the semaphore
   //  counts up to zero.
   srvcCount.Create( - static_cast<btInt>(size));

   //-------------------------------------------------
   // Iterate through the services shutting each down
   // after issuing a shutdown on each wait for the
   // services to complete
   //-------------------------------------------------
   for ( itr = m_ServiceMap.begin() ; size ; size--, itr++ ) {

      // If the IServiceModule is present
      if ( NULL != ( *itr ).second->getProvider() ) {

         // Shutdown done in parallel so each gets same max-time
         //   assume 0 time start so no timeout adjust performed
         DEBUG_CERR("ServiceBroker::DoShutdown - calling IServiceModule->Shutdown()\n");

         // Technically should join on these threads
         new OSLThread(ServiceBroker::ShutdownHandlerThread,
                       OSLThread::THREADPRIORITY_NORMAL,
                       new shutdown_handler_thread_parms(this, itr, &srvcCount, timeout));

         DEBUG_CERR("ServiceBroker::DoShutdown - returned from IServiceModule->Shutdown()\n");
      }
   }

   srvcCount.Wait(timeout);

   {
      AutoLock(this);

      //------------------------------------------
      // Send an event to the system event handler
      //------------------------------------------
      if ( m_servicecount ) {
         // Timed out - Shutdown did not succeed
         getRuntime()->schedDispatchable( new CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                         exttranevtServiceShutdown,
                                                                         rTranID,
                                                                         errSystemTimeout,
                                                                         reasSystemTimeout,
                                                                         const_cast<btString>(strSystemTimeout)) );
         return false;
      }
   }

   ServiceBase::Release(m_releaseTid, timeout);
   return true;
}  // ServiceBroker::DoShutdown

void ServiceBroker::ShutdownHandlerThread(OSLThread *pThread,
                                          void      *pContext)
{
   //Get a pointer to this objects context
   struct shutdown_handler_thread_parms *pparms =
            static_cast<struct shutdown_handler_thread_parms *>(pContext);
   ServiceBroker *This = static_cast<ServiceBroker *>(pparms->m_this);

   This->ShutdownHandler(pparms->m_itr, *pparms->m_srvcCount);

   // Destroy the thread and parms
   delete pparms;
}

//=============================================================================
// Name: ShutdownHandler
// Description: Services shutdown complete events
//=============================================================================
void ServiceBroker::ShutdownHandler(Servicemap_itr itr, CSemaphore &cnt)
{
   // get second ptr
   IServiceModule *pProvider = (*itr).second->getProvider();

   pProvider->Destroy();

   {
      AutoLock(this);

      // Delete the service which unloads the plug-in (e.g.,so or dll)
      //DEBUG_CERR("ServiceBroker::ShutdownHandler: pLibrary = " << (void*)(( *itr ).second.pLibrary) << endl);
      (*itr).second->freeProvider();
      //delete (*itr).second;
      m_servicecount--;
      cnt.Post(1);
   }
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
 void ServiceBroker::serviceEvent(const IEvent &rEvent)
 {
    // TODO
    ASSERT(false);
 }
///////////////////////////////////////////////////////////////////////////////
////                                                                       ////
////                          RESOURCE MANAGER CLIENT                      ////
////                                                                       ////
///////////////////////////////////////////////////////////////////////////////


 //=============================================================================
 // Name: resourceAllocated
 // Description: called when the resource(s) have been allocated by Resource
 //              manager
 // Interface: private
 // Inputs: nvsInstancerecord - instance record
 //         tid - Transaction ID
 // Outputs: none.
 // Comments:
 //=============================================================================
 void ServiceBroker::resourceAllocated( NamedValueSet const &nvsInstancerecord,
                                        TransactionID const &tid )
 {
     // Process the manifest
     btcString sName = NULL;

     // Get the original Tid and ServiceBase from the cache
     TransactionID origTid = m_Transactions[tid];
     m_Transactions.erase(tid);

     // Get the Runtime Proxy and Client information
     IBase *pClientBase             = m_ServiceClientMap[tid].ServiceBase;
     IRuntime *pProxy               = m_ServiceClientMap[tid].pProxy;
     IRuntimeClient *pRuntimeClient = m_ServiceClientMap[tid].pRuntimeClient;
     m_ServiceClientMap.erase(tid);

     INamedValueSet const *ConfigRecord = NULL;
     if ( ENamedValuesOK != nvsInstancerecord.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord) ) {
        return;
     }

     if ( ENamedValuesOK != ConfigRecord->Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, &sName) ) {
        return;
     }

     ServiceHost *SvcHost = NULL;
     if ( NULL == (SvcHost = findServiceHost(sName)) ) {
        // Load the Service Library and set the Runtime Proxy and Runtime Service Providers
        SvcHost = new ServiceHost(sName);
     }

     if ( !SvcHost->IsOK() ) {
        getRuntime()->schedDispatchable( new ServiceAllocateFailed(dynamic_ptr<IServiceClient>(iidServiceClient, pClientBase),
                                                                   pRuntimeClient,
                                                                   new CExceptionTransactionEvent(NULL,
                                                                                                  origTid,
                                                                                                  errCreationFailure,
                                                                                                  reasInternalError,
                                                                                                  "Failed to load Service")));
     }

     // Allocate the service
     if ( !SvcHost->InstantiateService(pProxy, pClientBase, nvsInstancerecord, origTid) ) {
        getRuntime()->schedDispatchable( new ServiceAllocateFailed(dynamic_ptr<IServiceClient>(iidServiceClient, pClientBase),
                                                                   pRuntimeClient,
                                                                   new CExceptionTransactionEvent(NULL,
                                                                                                  origTid,
                                                                                                  errCreationFailure,
                                                                                                  reasInternalError,
                                                                                                  "Failed to construct Service")));
     } else {
        // Save the ServiceHost
        m_ServiceMap[std::string(sName)] = SvcHost;
     }
}

 //=============================================================================
 // Name: resourceRequestFailed
 // Description: Unsolicited Event or Message handler
 // Interface: public
 // Inputs: rEvent - The event
 // Outputs: none.
 // Comments:
 //=============================================================================
 void ServiceBroker::resourceRequestFailed( NamedValueSet const &nvsManifest,
                                            const IEvent &rEvent )
 {
    // Get the clients TID for the final event using the tid assigned to the RequestResource()
    IExceptionTransactionEvent *theEvent =
                             dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);

    TransactionID tid = theEvent->TranID();
    TransactionID origTid = m_Transactions[tid];
    m_Transactions.erase(tid);

    IBase *pClientBase = m_ServiceClientMap[tid].ServiceBase;
    IRuntimeClient *pRuntimeClient = m_ServiceClientMap[tid].pRuntimeClient;
    m_ServiceClientMap.erase(tid);
    getRuntime()->schedDispatchable( new ServiceAllocateFailed(dynamic_ptr<IServiceClient>(iidServiceClient, pClientBase),
                                                               pRuntimeClient,
                                                               new CExceptionTransactionEvent(NULL,
                                                                                              origTid,
                                                                                              errCreationFailure,
                                                                                              reasResourcesNotAvailable,
                                                                                              strNoResourceDescr)));
 }

 //=============================================================================
 // Name: resourceManagerException
 // Description: Unsolicited Event or Message handler
 // Interface: public
 // Inputs: rEvent - The event
 // Outputs: none.
 // Comments:
 //=============================================================================
 void ServiceBroker::resourceManagerException( const IEvent &rEvent )
 {
 }


END_NAMESPACE(AAL)


