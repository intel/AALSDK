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
//        FILE: _xlruntimeimpl.h
//     CREATED: Mar 7, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for the XL runtime implementation
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __XLRUNTIMEIMPL_H__
#define __XLRUNTIMEIMPL_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/xlRuntime.h>
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
#include <aalsdk/aas/XLRuntimeModule.h>


BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: _getnewXLRuntimeInstance
// Description: XL runtime system factory.
// Comments:
//=============================================================================
BEGIN_C_DECLS
IRuntime *_getnewXLRuntimeInstance();
END_C_DECLS


//=============================================================================
// Name: _xlruntime
// Description: Class implements the internal XL runtime system.
// Comments:
//=============================================================================
class _xlruntime : public  CAASBase,
                   private CUnCopyable,
                   private IServiceClient,
                   public  IXLRuntimeServices,
                   public  IRuntime
{
public:
   _xlruntime();
   ~_xlruntime();

   // Start: Start the runtime
   //    Input: pclient - Pointer to an IRuntimeClient callback.
   //           rconfigParms - Reference to configuration parameters.
   btBool start(IBase               *pClient,
                const NamedValueSet &rconfigParms);

   // Stop: Stop the runtime
   void stop();

 

   btBool IsOK();

   // IXLRuntimeServices

   //  allocService: Allocates a Service to the client
   //    Input: pClient - Pointer to an IBase containing and IServiceClient interface.
   //           rManifest - Reference to manifest containing Service
   //                       description and any configuration parameters.
   void allocService( IBase                   *pClient,
                      NamedValueSet const     &rManifest = NamedValueSet(),
                      TransactionID const     &rTranID   = TransactionID(),
                      IRuntime::eAllocatemode  mode = NotifyAll);

   void schedDispatchable(IDispatchable *pdispatchable);

   // IXLRuntimeServices
   IBase      *getMessageDeliveryService();
   void        setMessageDeliveryService(IBase *pMDSbase);
   btBool      SendMsg(IDispatchable *pobject, btObjectType parm);
   IRuntimeClient *getRuntimeClient();

protected:
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

   // Internal interfaces
   btBool ProcessConfigParms(const NamedValueSet &rConfigParms);

private:
   enum Services{
      MDS = 1,
      Broker
   };

   enum State{
      Stopped = 1,
      Started
   };

   btBool                          m_status;
   IRuntimeClient                 *m_pclient;
   enum State                      m_state;

   // Core Facilities: Implemented as built-in plug-in Services
   //  each will have an Entry Point (module commands handler), IServiceModule (Factory)
   //  and a Service interface.

   // Default MDS Host container
   ServiceHost                     *m_pMDSSvcHost;
   //Default Service Broker Host Container
   ServiceHost                     *m_pBrokerSvcHost;

   // Active core services
   IEventDeliveryService           *m_pMDS;
   IBase                           *m_pMDSbase;

   IServiceBroker                  *m_pBroker;
   IBase                           *m_pBrokerbase;

   CSemaphore                       m_sem;
};

END_NAMESPACE(AAL)


#endif // __XLRUNTIMEIMPL_H__

