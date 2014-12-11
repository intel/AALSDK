// Copyright (c) 2014, Intel Corporation
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
#include "aalsdk/aas/XLRuntimeMessages.h"
#include "aalsdk/aas/ServiceHost.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/AALLoggerExtern.h"              // AAL Logger

#include "ServiceBroker.h"


#define SERVICE_FACTORY AAL::AAS::InProcSvcsFact< AAL::ServiceBroker >

#define RRMBROKER_VERSION_CURRENT  0
#define RRMBROKER_VERSION_REVISION 0
#define RRMBROKER_VERSION_AGE      0
#define RRMBROKER_VERSION          "0.0.0"

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
USING_NAMESPACE(XL)


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
void ServiceBroker::init(TransactionID const &rtid)
{
   // The Resource Manager is implemented as an AAL Service however rather
   //  than going through the XLRuntime and default broker to aqcuire it
   //  using a ServiceHost allows us to specifically plug in the built-in
   //  implementation. NOTE: This is  the way the default runtime services
   //  are bootstrapped.
   m_pRMSvcHost = new XL::RT::ServiceHost(AAL_BUILTIN_SVC_MOD_ENTRY_POINT(librrm),
                                          getRuntime(),
                                          getRuntimeServiceProvider());

   //Allocate the service.

   // Default constructor creates unique TID. This will be used later to re-acquire the clients tid
   //  to generate the response message

   TransactionID tid = TransactionID();
   m_Transactions[tid] = rtid;
   if(false == m_pRMSvcHost->allocService( dynamic_cast<AAL::IBase*>(this), NamedValueSet(), tid, AAL::XL::RT::IRuntime::NoRuntimeClientNotification ) ){
      // Remove pending transaction
      m_Transactions.erase(tid);
      QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent( getRuntimeClient(),
                                                               Client(),
                                                               this,
                                                               rtid,
                                                               errServiceNotFound,
                                                               reasUnknown,
                                                               "Could not allocate ResourceManager.  Possible bad argument or missing client interface.") );
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
void ServiceBroker::serviceAllocated( AAL::IBase          *pServiceBase,
                                      TransactionID const &rTranID)
{
   AutoLock(this);

   TransactionID origTid = m_Transactions[rTranID];
   m_Transactions.erase(rTranID);

   m_ResMgrBase = pServiceBase;
   m_ResMgr     = subclass_ptr<AAL::IResourceManager>(pServiceBase);
   if(NULL == m_ResMgr){

      QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent( getRuntimeClient(),
                                                               Client(),
                                                               this,
                                                               origTid,
                                                               errMethodNotImplemented,
                                                               reasNotImplemented,
                                                               "Service does not support IResourceManager") );
      return;
   }
   m_bIsOK = true;
   QueueAASEvent(new AAL::AAS::ObjectCreatedEvent( getRuntimeClient(),
                                                   Client(),
                                                   dynamic_cast<IBase*>(this),
                                                   origTid));
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
void ServiceBroker::serviceAllocateFailed(const IEvent        &rEvent)
{
   AAL::TransactionID TranID = AAL::dynamic_ref<ITransactionEvent>(iidTranEvent,rEvent).TranID();
   TransactionID origTid = m_Transactions[TranID];
   m_Transactions.erase(TranID);

   // If we were unable to load the ResourceManager then we cannot load.
   QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent( getRuntimeClient(),
                                                            Client(),
                                                            this,
                                                            origTid,
                                                            errServiceNotFound,
                                                            reasInvalidService,
                                                            strInvalidService)  );

}

//=============================================================================
// Name: serviceFreed
// Description: Service has been freed
// Interface: public
// Inputs: pServiceBase - Service that was freed
//         rTranID - Optional TransactionID
// Outputs: none.
// Comments:
//=============================================================================
void ServiceBroker::serviceFreed(TransactionID const &rTranID)
{
   AutoLock(this);
   // If we were unable to load the ResourceManager then we cannot load.
   SendMsg(new AAL::XL::RT::ServiceClientMessage(Client(),
                                    this,
                                    AAL::XL::RT::ServiceClientMessage::Freed,
                                    rTranID) );
}

//=============================================================================
// Name: allocService
// Description: Allocates a Service
// Interface: public
// Inputs:  pServiceClient - Pointer to the standard Service Client interface
// Comments:
//=============================================================================
void ServiceBroker::allocService(AAL::IBase                          *pServiceBase,
                                 const NamedValueSet                 &rManifest,
                                 TransactionID const                 &rTranID,
                                 AAL::XL::RT::IRuntime::eAllocatemode mode)
{
   // Process the manifest
   btcString            sName = NULL;
   NamedValueSet const *ConfigRecord;

   if ( ENamedValuesOK != rManifest.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord) ) {
      return;
   }

   if ( ENamedValuesOK != ConfigRecord->Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, &sName) ) {
      return;
   }
   // If this Service is not pure software then use Resource Manager
   if( !ConfigRecord->Has(AAL_FACTORY_CREATE_SOFTWARE_SERVICE)){

      if(NULL != m_ResMgr ) {

         // Need to save the pServiceClient to be able to generate the final event
         TransactionID tid;
         m_ServiceClientMap[tid].ServiceBase = pServiceBase;
         m_ServiceClientMap[tid].NoRuntimeEvent = mode;
         m_Transactions[tid] = rTranID;

         m_ResMgr->RequestResource(rManifest, tid );

      }else{
         // Throw error
      }
      return;
   }else{
      XL::RT::ServiceHost *SvcHost = NULL;
      if ( NULL == (SvcHost = findServiceHost(sName)) ) {
         // Instantiate the core facilities
         SvcHost = new XL::RT::ServiceHost(sName, getRuntime(), getRuntimeServiceProvider());
      }

      if ( !SvcHost->IsOK() ) {
         QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent( getRuntimeClient(),
                                                                 Client(),
                                                                 NULL,
                                                                 rTranID,
                                                                 errCreationFailure,
                                                                 reasInternalError,
                                                                 "Failed to load Service"));
      }

      // Allocate the service
      if ( !SvcHost->allocService(pServiceBase, rManifest, rTranID, mode) ) {
         QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                 Client(),
                                                                 NULL,
                                                                 rTranID,
                                                                 errCreationFailure,
                                                                 reasInternalError,
                                                                 "Failed to construct Service"));
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
XL::RT::ServiceHost *ServiceBroker::findServiceHost(std::string const &sName)
{
   Servicemap_itr itr = m_ServiceMap.find(sName);
   if ( itr == m_ServiceMap.end() ) {
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
// Name: Release
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
btBool ServiceBroker::Release(TransactionID const &rTranID, btTime timeout)
{
   struct shutdown_thread_parms *pparms =
                                    new struct shutdown_thread_parms(this,
                                                                     rTranID,
                                                                     timeout);
   //--------------------------------------------
   // Create the Shutdown thread object
   //  The Shutdown thread is self destructive so
   //  no need to keep pointer nor do a Join()
   //  as the ~OSLThread will clean up
   //  resources from unjoined threads
   //--------------------------------------------

   // Important to Lock here and in thread to ensure that the assignment
   //  is complete before thread runs.
   Lock();
   m_pShutdownThread = new OSLThread(ServiceBroker::ShutdownThread,
                                     OSLThread::THREADPRIORITY_NORMAL,
                                     pparms);
   Unlock();
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

   // Destroy the thread and parms
   delete pparms;
   This->ServiceBase::Release(pparms->m_timeout);
}

struct shutdown_handler_thread_parms
{
shutdown_handler_thread_parms(ServiceBroker                 *pfact,
                              ServiceBroker::Servicemap_itr  itr,
                              CSemaphore                    &srvcCount,
                              btTime                         timeout) :
   m_this(pfact),
   m_itr(itr),
   m_timeout(timeout),
   m_srvcCount(srvcCount)
{}

   ServiceBroker                 *m_this;
   ServiceBroker::Servicemap_itr  m_itr;
   btTime                         m_timeout;
   CSemaphore                    &m_srvcCount;
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
                       new shutdown_handler_thread_parms(this, itr, srvcCount, timeout));

         DEBUG_CERR("ServiceBroker::DoShutdown - returned from IServiceModule->Shutdown()\n");
      }
   }

   srvcCount.Wait(timeout);
   Lock();

   //------------------------------------------
   // Send an event to the system event handler
   //------------------------------------------
   if ( m_servicecount ) {
      // Timed out - Shutdown did not succeed
      QueueAASEvent(new AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase*>(this),
                                                             exttranevtServiceShutdown,
                                                             rTranID,
                                                             errSystemTimeout,
                                                             reasSystemTimeout,
                                                             const_cast<btString>(strSystemTimeout)));
      Unlock();
   } else {
#if 0
      // Generate the event - Note that CObjectDestroyedTransactionEvent will work as well
      SendMessage(new ServiceClientMessage( Client(),
                                            this,
                                            ServiceClientMessage::Freed,
                                            rTranID)
                                          );
#endif

      QueueAASEvent(new AAL::AAS::CObjectDestroyedTransactionEvent(Client(), NULL, rTranID, NULL));

      // Clear the map now
      m_ServiceMap.clear();
      // Unlock before Release as that Destroys "this"
      Unlock();
      return true;
   }

   return false;
}  // ServiceBroker::DoShutdown

void ServiceBroker::ShutdownHandlerThread(OSLThread *pThread,
                                          void      *pContext)
{
   //Get a pointer to this objects context
   struct shutdown_handler_thread_parms *pparms =
            static_cast<struct shutdown_handler_thread_parms *>(pContext);
   ServiceBroker *This = static_cast<ServiceBroker *>(pparms->m_this);

   This->ShutdownHandler(pparms->m_itr, pparms->m_srvcCount);

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
   AAL::AAS::IServiceModule *pProvider = (*itr).second->getProvider();

   pProvider->Destroy();

   Lock();

   // Delete the service which unloads the plug-in (e.g.,so or dll)
   //DEBUG_CERR("ServiceBroker::ShutdownHandler: pLibrary = " << (void*)(( *itr ).second.pLibrary) << endl);

   delete (*itr).second;
   m_servicecount--;
   cnt.Post(1);

   Unlock();
}

 // Quiet Release. Used when Service is unloaded.
 btBool ServiceBroker::Release(btTime timeout)
 {
    return ServiceBase::Release(timeout);
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
     btcString            sName = NULL;

     // Get the original Tid and ServiceBase from the cache
     TransactionID origTid = m_Transactions[tid];
     m_Transactions.erase(tid);

     AAL::IBase *pClientBase = m_ServiceClientMap[tid].ServiceBase;
     AAL::btBool NoRuntimeEvent = m_ServiceClientMap[tid].NoRuntimeEvent;
     m_ServiceClientMap.erase(tid);

     NamedValueSet const *ConfigRecord;
     if ( ENamedValuesOK != nvsInstancerecord.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord) ) {
        return;
     }

     if ( ENamedValuesOK != ConfigRecord->Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, &sName) ) {
        return;
     }
     XL::RT::ServiceHost *SvcHost = NULL;
     if ( NULL == (SvcHost = findServiceHost(sName)) ) {
        // Instantiate the core facilities
        SvcHost = new XL::RT::ServiceHost(sName, getRuntime(), getRuntimeServiceProvider());
     }

     if ( !SvcHost->IsOK() ) {
        QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                dynamic_ptr<IServiceClient>(iidServiceClient,pClientBase),
                                                                NULL,
                                                                origTid,
                                                                errCreationFailure,
                                                                reasInternalError,
                                                                "Failed to load Service"));
     }

     // Allocate the service
     if ( !SvcHost->allocService(pClientBase, nvsInstancerecord, origTid) ) {
        QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                dynamic_ptr<IServiceClient>(iidServiceClient,pClientBase),
                                                                NULL,
                                                                origTid,
                                                                errCreationFailure,
                                                                reasInternalError,
                                                                "Failed to construct Service"));
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

    AAL::IBase *pClientBase = m_ServiceClientMap[tid].ServiceBase;
    m_ServiceClientMap.erase(tid);
    QueueAASEvent(new AAL::AAS::ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                            dynamic_ptr<IServiceClient>(iidServiceClient, pClientBase),
                                                            NULL,
                                                            origTid,
                                                            errCreationFailure,
                                                            reasResourcesNotAvailable,
                                                            strNoResourceDescr));
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
