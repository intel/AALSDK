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
//        FILE: _ServiceBroker.cpp
//     CREATED: Mar 14, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Implements the AAL default Service Broker.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 06/25/2015     JG       Removed XL from name
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALDefs.h"
#include "aalsdk/INTCDefs.h"

#include "aalsdk/osal/OSServiceModule.h"
#include "aalsdk/aas/AALInProcServiceFactory.h"  // Defines InProc Service Factory
#include "aalsdk/Dispatchables.h"
#include "aalsdk/aas/ServiceHost.h"
#include "aalsdk/AALLoggerExtern.h"              // AAL Logger
#include "_ServiceBroker.h"
#include "aalsdk/aas/AALRuntimeModule.h"


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::_ServiceBroker >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, localServiceBroker, AALRUNTIME_API, AALRUNTIME_VERSION, AALRUNTIME_VERSION_CURRENT, AALRUNTIME_VERSION_REVISION, AALRUNTIME_VERSION_AGE)
   // Only default service commands for now.
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)


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
btBool _ServiceBroker::init( IBase *pclientBase,
                             NamedValueSet const &optArgs,
                             TransactionID const &rtid)
{
   return initComplete(rtid);
}

//=============================================================================
// Name: allocService
// Description: Allocates a Service
// Interface: public
// Inputs:  pServiceClient - Pointer to the standard Service Client interface
// Comments:
//=============================================================================
void _ServiceBroker::allocService(IRuntime               *pProxy,
                                  IRuntimeClient         *pRuntimeClient,
                                  IBase                  *pServiceClientBase,
                                  const NamedValueSet     &rManifest,
                                  TransactionID const     &rTranID)
{
   // Process the manifest
   btcString             sName  = NULL;
   INamedValueSet const *ConfigRecord = NULL;

   IServiceClient      *pServiceClient = dynamic_ptr<IServiceClient>(iidServiceClient, pServiceClientBase);
   if ( NULL == pServiceClient ) { // TODO replace all ObjectCreatedExceptionEvents with RuntimeCallbacks
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(pRuntimeClient,
                                                                      pServiceClient,
                                                                      NULL,
                                                                      rTranID,
                                                                      errAllocationFailure,
                                                                      reasMissingInterface,
                                                                      strMissingInterface));
   }

   if ( ENamedValuesOK != rManifest.Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord) ) {
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(pRuntimeClient,
                                                                      pServiceClient,
                                                                      NULL,
                                                                      rTranID,
                                                                      errAllocationFailure,
                                                                      reasBadConfiguration,
                                                                      "Missing Config Record"));
      return;
   }

   if ( ENamedValuesOK != ConfigRecord->Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, &sName) ) {
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(pRuntimeClient,
                                                                      pServiceClient,
                                                                      NULL,
                                                                      rTranID,
                                                                      errAllocationFailure,
                                                                      reasBadConfiguration,
                                                                      "Missing Config RecordService Name"));
      return;
   }

   ServiceHost *SvcHost = NULL;
   if ( NULL == (SvcHost = findServiceHost(sName)) ) {
      // Load the Service Library and set the Runtime Proxy and Runtime Service Providers
      SvcHost = new ServiceHost(sName);
   }

   if ( (NULL == SvcHost) || !SvcHost->IsOK() ) {
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(pRuntimeClient,
                                                                      pServiceClient,
                                                                      NULL,
                                                                      rTranID,
                                                                      errCreationFailure,
                                                                      reasInternalError,
                                                                      "Failed to load Service"));
      return;
   }

   // Allocate the service

   // Save the ServiceHost.  Do it now before the Service generates the serviceAllocated.
   //  If it fails remove it
   m_ServiceMap[std::string(sName)] = SvcHost;

   if ( !SvcHost->InstantiateService(pProxy, pServiceClientBase, rManifest, rTranID) ) {
      m_ServiceMap.erase(std::string(sName));
      delete SvcHost;
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(pRuntimeClient,
                                                                      pServiceClient,
                                                                      NULL,
                                                                      rTranID,
                                                                      errCreationFailure,
                                                                      reasInternalError,
                                                                      "Failed to construct Service"));
      return;
   }

}


//=============================================================================
// Name: findServiceHost
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
ServiceHost *_ServiceBroker::findServiceHost(std::string const &sName)
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
   shutdown_thread_parms(_ServiceBroker    *pfact,
                         TransactionID const &rTranID,
                         btTime               timeout) :
      m_this(pfact),
      m_rTranID(rTranID),
      m_timeout(timeout)
   {}

   _ServiceBroker *m_this;
   TransactionID     m_rTranID;
   btTime            m_timeout;
};
//=============================================================================
// Name: Release
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
btBool _ServiceBroker::Release(TransactionID const &rTranID, btTime timeout)
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
   {
      AutoLock(this);
      m_pShutdownThread = new OSLThread(_ServiceBroker::ShutdownThread,
                                        OSLThread::THREADPRIORITY_NORMAL,
                                        pparms);
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
void _ServiceBroker::ShutdownThread(OSLThread *pThread,
                                      void      *pContext)
{
   //Get a pointer to this objects context
   struct shutdown_thread_parms *pparms = static_cast<struct shutdown_thread_parms *>(pContext);
   _ServiceBroker             *This   = static_cast<_ServiceBroker *>(pparms->m_this);

   This->DoShutdown(pparms->m_rTranID, pparms->m_timeout);

   // Destroy the thread and parms
   delete pparms;

}

struct shutdown_handler_thread_parms
{
shutdown_handler_thread_parms(_ServiceBroker *pfact,
                              ServiceHost      *pSvcHost,
                              CSemaphore       &srvcCount,
                              btTime            timeout) :
   m_this(pfact),
   m_pSvcHost(pSvcHost),
   m_timeout(timeout),
   m_srvcCount(srvcCount)
{}

   _ServiceBroker *m_this;
   ServiceHost      *m_pSvcHost;
   btTime            m_timeout;
   CSemaphore       &m_srvcCount;
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
btBool _ServiceBroker::DoShutdown(TransactionID const &rTranID,
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
   srvcCount.Create( - static_cast<btInt>(size) );

   //-------------------------------------------------
   // Iterate through the services shutting each down
   // after issuing a shutdown on each wait for the
   // services to complete
   //-------------------------------------------------
   for ( itr = m_ServiceMap.begin() ; size > 0 ; size--, itr++ ) {

      // If the IServiceModule is present
      if ( NULL != (*itr).second->getProvider() ) {

         // Shutdown done in parallel so each gets same max-time
         //   assume 0 time start so no timeout adjust performed

         // Technically should join on these threads
         new OSLThread(_ServiceBroker::ShutdownHandlerThread,
                       OSLThread::THREADPRIORITY_NORMAL,
                       new shutdown_handler_thread_parms(this, (*itr).second, srvcCount, timeout));

      }
   }

   srvcCount.Wait(timeout);

   {
      AutoLock(this);
      //------------------------------------------
      // Send an event to the system event handler
      //------------------------------------------
      if ( m_servicecount > 0 ) {
         // Timed out - Shutdown did not succeed
         getRuntime()->schedDispatchable(new CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                        exttranevtServiceShutdown,
                                                                        rTranID,
                                                                        errSystemTimeout,
                                                                        reasSystemTimeout,
                                                                        const_cast<btString>(strSystemTimeout)));
      } else {
         // Generate the callback and finish the cleanup (performed in the Dispatchable)
         getRuntime()->schedDispatchable(new ServiceClientCallback(ServiceClientCallback::Released,
                                                                   ServiceClient(),
                                                                   this,
                                                                   rTranID));

         // Clear the map now
         m_ServiceMap.clear();
         return true;
      }
   }

   return false;
}  // _ServiceBroker::DoShutdown

void _ServiceBroker::ShutdownHandlerThread(OSLThread *pThread,
                                             void      *pContext)
{
   //Get a pointer to this objects context
   struct shutdown_handler_thread_parms *pparms =
            static_cast<struct shutdown_handler_thread_parms *>(pContext);
   _ServiceBroker *This = static_cast<_ServiceBroker *>(pparms->m_this);

   This->ShutdownHandler(pparms->m_pSvcHost, pparms->m_srvcCount);

   // Destroy the thread and parms
   delete pparms;
}

//=============================================================================
// Name: ShutdownHandler
// Description: Services shutdown complete events
//=============================================================================
void _ServiceBroker::ShutdownHandler(ServiceHost *pSvcHost, CSemaphore &cnt)
{
   // get second ptr
   IServiceModule *pProvider = pSvcHost->getProvider();

   pProvider->Destroy();

   {
      AutoLock(this);
      // Delete the service which unloads the plug-in (e.g.,so or dll)
      // DEBUG_CERR("_ServiceBroker::ShutdownHandler: pLibrary = " << (void *)( pProvider ) << endl);

      delete pSvcHost;
      m_servicecount--;
      cnt.Post(1);
   }
}

 //=============================================================================
 // Name: ~_ServiceBroker
 // Description: Destructor
 // Interface: public
 // Comments:
 //=============================================================================
 _ServiceBroker::~_ServiceBroker()
 {

 }


END_NAMESPACE(AAL)

