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
//        FILE: _runtimeimpl.h
//     CREATED: Mar 7, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for the AAL runtime implementation
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 06/25/2015     JG       Removed XL from name
//****************************************************************************///
#ifndef __RUNTIMEIMPL_H__
#define __RUNTIMEIMPL_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/aas/_xlRuntimeServices.h>
#include <aalsdk/CUnCopyable.h>
#include <aalsdk/AALBase.h>

#include <aalsdk/osal/OSSemaphore.h>

#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/aas/ServiceHost.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/aas/IServiceBroker.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/eds/AASEventDeliveryService.h>
#include <aalsdk/aas/AALRuntimeModule.h>

#include <map>


BEGIN_NAMESPACE(AAL)
class _runtime;

//=============================================================================
// Name: _getnewRuntimeInstance
// Description: AAL Runtime system factory.
// Comments:
//=============================================================================
BEGIN_C_DECLS
_runtime *_getnewRuntimeInstance( Runtime *pRuntimeProxy,
                                  IRuntimeClient *pClient);
END_C_DECLS


//=============================================================================
// Name: _runtime
// Description: Class implements the internal AAL Runtime system.
// Comments:
//=============================================================================
class _runtime : public  CAASBase,
                 public  IRuntime,
                 private CUnCopyable,
                 private IServiceClient,        // For internal Services
                 private IRuntimeClient,        // _runtime uses a Runtime Proxy
                 public  IAALRUNTIMEServices
{
public:
   typedef std::map<Runtime *, IRuntimeClient *> ClientMap;
   typedef ClientMap::iterator                 ClientMap_itr;

   _runtime(Runtime* pRuntimeProxy, IRuntimeClient*pClient);
   void releaseRuntimeInstance( Runtime *pRuntimeProxy);


   // Start: Start the runtime
   // Input: pProxy - Pointer to Proxy for the IRuntimeClient callback.
   //           rconfigParms - Reference to configuration parameters.
   btBool start( Runtime *pProxy,
                 const NamedValueSet &rconfigParms );
   // Stop: Stop the runtime
   void stop(Runtime* pProxy);

   //  allocService: This variant takes a proxy pointer so that the RuntimeClient
   //                responses go to teh correct place.
   //    Input: pProxy - Pointer to Runtime Proxy.
   //           pClient - Pointer to an IBase containing and IServiceClient interface.
   //           rManifest - Reference to manifest containing Service
   //                       description and any configuration parameters.
   void allocService( Runtime                 *pProxy,
                      IBase                   *pClient,
                      NamedValueSet const     &rManifest = NamedValueSet(),
                      TransactionID const     &rTranID   = TransactionID());

   // IRuntime - Needed so that internal services can use the Runtime
   //            without involving Application's Runtime Proxy.  Can't use
   //            a Proxy since the Runtime is not instantiated yet.
   //            Some of the functions are not supported.
   btBool start(const NamedValueSet &rconfigParms) {return false;};
   void   stop() {return;};
   IRuntime *getRuntimeProxy(IRuntimeClient *pClient){return NULL;};

   btBool releaseRuntimeProxy(IRuntime *pRuntime){return false;};
   btBool releaseRuntimeProxy(){return false;};

   IRuntimeClient *getRuntimeClient();

   btBool IsOK();

   //  allocService: Allocates a Service to the client
   //    Input: pClient - Pointer to an IBase containing and IServiceClient interface.
   //           rManifest - Reference to manifest containing Service
   //                       description and any configuration parameters.
   void allocService( IBase                   *pClient,
                      NamedValueSet const     &rManifest = NamedValueSet(),
                      TransactionID const     &rTranID   = TransactionID());

   void schedDispatchable(IDispatchable *pdispatchable);
   // </IRuntime>


   // Returns a proxy pointer to the singleton Runtime
   void addProxy(Runtime *pRuntimeProxy,
                 IRuntimeClient *pClient);

   // Returns a proxy pointer to the singleton Runtime
   void removeProxy(Runtime *pRuntimeProxy);

   // IAALRUNTIMEServices
   IBase      *getMessageDeliveryService();
   void        setMessageDeliveryService(IBase *pMDSbase);
   btBool      SendMsg(IDispatchable *pobject, btObjectType parm);

protected:
   btBool InstallDefaults();
   //
   // IServiceClient Interface
   //-------------------------
   void  serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &rTranID = TransactionID());
   void  serviceReleased(TransactionID const &rTranID = TransactionID());
   void  serviceReleaseFailed(const IEvent &rEvent);
   void  serviceAllocateFailed(const IEvent &rEvent);
   // Message Handler
   //   Input: rEvent - Event contains message/event.  Typically used for
   //          exceptions or events for which no standard callback is defined.
   void serviceEvent(const IEvent &rEvent);

   // IRuntimeClient
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};

    void runtimeStarted(IRuntime            *pRuntime,
                        const NamedValueSet &rConfigParms){};

    void runtimeStopped(IRuntime *pRuntime){};

    void runtimeStartFailed(const IEvent &rEvent){};

    void runtimeStopFailed(const IEvent &rEvent){};

    void runtimeAllocateServiceFailed( IEvent const &rEvent){};

    void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                         TransactionID const &rTranID){};

    void runtimeEvent(const IEvent &rEvent){};
    // </IRuntimeClient>
   // Internal interfaces
   btBool ProcessConfigParms(const NamedValueSet &rConfigParms);

private:
   _runtime();
   ~_runtime();

   enum Services{
      MDS = 1,
      Broker
   };

   enum State{
      Stopped = 1,
      Started
   };

   btBool                         m_status;
   enum State                     m_state;
   IRuntimeClient                *m_pClient;       // Client used by internal services

   // Clients of the Runtime
   Runtime                       *m_pOwner;        // Creator Runtime Proxy
   IRuntimeClient                *m_pOwnerClient;  // Creator's Client

   ClientMap                      m_mClientMap;    // Map of Runtime Proxys


   // Core Facilities: Implemented as built-in plug-in Services
   //  each will have an Entry Point (module commands handler), IServiceModule (Factory)
   //  and a Service interface.

   // Default MDS Host container
   ServiceHost                    *m_pMDSSvcHost;

   //Default Service Broker Host Container
   ServiceHost                    *m_pBrokerSvcHost;

   // Active core services
   IEventDeliveryService          *m_pMDS;
   IBase                          *m_pMDSbase;

   IServiceBroker                 *m_pBroker;
   IBase                          *m_pBrokerbase;

   CSemaphore                      m_sem;
};

END_NAMESPACE(AAL)


#endif // __AALRUNTIMEIMPL_H__

