// Copyright (c) 2012-2015, Intel Corporation
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
/// @file AALService.cpp
/// @brief ServiceBase implementation.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
/// COMMENTS: This module implements the Base  classes used by AAL Services.
///           Use of these classes insure that the Service properly plugs
///           into the AAL Service framework and provides the Service with
///           convenient access to other system core facilities.
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/21/2013     TSW      Moving C++ inlined definitions to .cpp file
/// 03/12/2014     JG       Added support for client callback interface
///                         Fixed a bug in copy constructor where copy was
///                          incomplete.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/aas/AALService.h"
#include "aalsdk/AALLogger.h"
#include "aalsdk/Dispatchables.h"

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: ServiceBase
// Description: Constructor.
// Interface: public
// Inputs: AALServiceModule *container - Pointer to the Service Module that
//                                       provdes access to core facilities and
//                                       maintains a list of all Services
//                                       constructed by this module.
//         IAALTransport    *ptransport - Optional transport (remote only)
//         IAALMarshaller   *marshaller - Optional Marshaller (remote only)
//         IAALUnMarshaller *unmarshaller - Optional Unmarshaller (remote only)
// Comments:
//=============================================================================
ServiceBase::ServiceBase(AALServiceModule *container,
                         IRuntime         *pAALRuntime,
                         IAALTransport    *ptransport,
                         IAALMarshaller   *marshaller,
                         IAALUnMarshaller *unmarshaller) :
   m_RuntimeClient(NULL),
   m_Runtime(pAALRuntime->getRuntimeProxy(this)),  // Use my own Proxy
   m_pclient(NULL),
   m_pclientbase(NULL),
   m_pcontainer(container),
   m_ptransport(ptransport),
   m_pmarshaller(marshaller),
   m_punmarshaller(unmarshaller),
   m_runMDT(false),
   m_pMDT(NULL)
{
   AutoLock(this);

   if ( EObjOK != SetInterface(iidServiceBase, dynamic_cast<IServiceBase *>(this)) ) {
      m_bIsOK = false;
      return;
   }

   if ( EObjOK != SetSubClassInterface(iidService, dynamic_cast<IAALService *>(this)) ) {
      m_bIsOK = false;
      return;
   }
}

ServiceBase::~ServiceBase()
{
   btBool DoJoin = false;

   {
      AutoLock(this);

      // Cleanup objects we were handed
      //   TODO May want to consider moving to upper layer that created them
      //   but the factory is currently temporal.
      if ( HasTransport() ) {
         delete m_ptransport;
         m_ptransport = NULL;
      }

      if ( HasMarshaller() ) {
         delete m_pmarshaller;
         m_pmarshaller = NULL;
      }

      if ( HasUnMarshaller() ) {
         delete m_punmarshaller;
         m_punmarshaller = NULL;
      }

      m_runMDT = false;
      DoJoin = ( NULL != m_pMDT );
   }

   if ( DoJoin ) {
      // Don't join while locked.
      m_pMDT->Join();
   }

   {
      AutoLock(this);

      if ( m_bIsOK ) {
         // Not been released yet.
         Released();
      }
   }
}

btBool ServiceBase::Release(TransactionID const &rTranID, btTime timeout)
{
   AutoLock(this);

   Released();

   // Send the Released Event.  The callback will execute ServiceBase::Release(btTime timeout)
   //  just before dispatching the callback thus insuring that the final cleanup is executed
   //  before notification is received.
   return getRuntime()->schedDispatchable( new ServiceClientCallback(ServiceClientCallback::Released,
                                                                     Client(),
                                                                     this,
                                                                     rTranID) );
}

btBool ServiceBase::Release(btTime timeout)
{
   {
      AutoLock(this);

      Released();

      // Release the Proxy
      getRuntime()->releaseRuntimeProxy();
      m_Runtime = NULL;
   }

   // We must constrain the scope of the AutoLock above to prevent dereferencing
   // this after deleting it below.

   delete this;
   return true;
}

IBase * ServiceBase::_init(IBase               *pclient,
                           TransactionID const &rtid,
                           NamedValueSet const &optArgs,
                           CAALEvent           *pcmpltEvent)
{
   if ( NULL == pclient ) {
      return NULL;
   }

   AutoLock(this);

   m_pclientbase = pclient;

   m_pclient = dynamic_ptr<IServiceClient>(iidServiceClient, pclient);
   if ( NULL == m_pclient ) {
      return NULL;
   }

   m_optArgs = optArgs;

   // This is used to generate the creation event.
   m_RuntimeClient = getRuntime()->getRuntimeClient();

   // Check that mandatory initialization has occurred

   // If no completion event then this is the direct superclass
   //  of the most derived class in the Service class hierarchy.
   if ( NULL == pcmpltEvent ) {
      init(rtid);
   } else {
      // Queue the completion to enable next layer down (the class derived from this)
      //   to initialize
      getRuntime()->schedDispatchable(pcmpltEvent);
   }

   return this;
}

btBool ServiceBase::sendmsg()
{
   AutoLock(this);

   if ( !HasTransport() ) {
      return false;
   }

   btWSSize  len;
   btBool    ret  = true;
   btcString pBuf = marshall().pmsgp(&len);

   if ( sender().putmsg(pBuf, len) != len ) {
      ret = false;
   }
   // Empty the marshaller buffer for next message
   marshall().Empty();
   return ret;
}

btBool ServiceBase::getmsg()
{
   AutoLock(this);

   if ( !HasTransport() ) {
      return false;
   }

   btWSSize  len = 0;
   btcString msg = recvr().getmsg(&len);
      
   if ( (btWSSize)-1 == len ) {
      AAL_ERR(LM_AAS, "recv error\n");
      return false;
   }

   if ( 0 == len ) {
      return false; // EOF seen, no data
   }

   unmarshall().importmsg(msg, len);
   return true;
}

btBool ServiceBase::startMDS()
{
   AutoLock(this);

   ASSERT(NULL == m_pMDT);
   if ( NULL != m_pMDT ) {
      // prevent multiple start's so that we don't leak threads.
      return false;
   }
   ASSERT(!m_runMDT);

   m_runMDT = true;
   m_pMDT   = new(std::nothrow) OSLThread(ServiceBase::_MessageDeliveryThread,
                                          OSLThread::THREADPRIORITY_ABOVE_NORMAL,
                                          this);
   if ( NULL == m_pMDT ) {
      m_runMDT = false;
      return false;
   }

   m_runMDT = m_pMDT->IsOK();

   return m_runMDT;
}

void ServiceBase::_MessageDeliveryThread(OSLThread *pThread, void *pContext)
{
   // Call the non-static member
   reinterpret_cast<ServiceBase *>(pContext)->MessageDeliveryThread();
}

void ServiceBase::MessageDeliveryThread()
{
// TODO error handling
   while ( m_runMDT ) {

      if ( false == getmsg() ) {
         exit(1);
      }

      processmsg();
   }
}

IAALMarshaller &     ServiceBase::marshall() { AutoLock(this); return *m_pmarshaller;   }
IAALUnMarshaller & ServiceBase::unmarshall() { AutoLock(this); return *m_punmarshaller; }
IAALTransport &         ServiceBase::recvr() { AutoLock(this); return *m_ptransport;    }
IAALTransport &        ServiceBase::sender() { AutoLock(this); return *m_ptransport;    }

btBool   ServiceBase::HasMarshaller()  const { AutoLock(this); return NULL != m_pmarshaller;   }
btBool ServiceBase::HasUnMarshaller()  const { AutoLock(this); return NULL != m_punmarshaller; }
btBool    ServiceBase::HasTransport()  const { AutoLock(this); return NULL != m_ptransport;    }

NamedValueSet const &        ServiceBase::OptArgs() const { AutoLock(this); return m_optArgs;       }
IServiceClient *              ServiceBase::Client() const { AutoLock(this); return m_pclient;       }
IBase *                   ServiceBase::ClientBase() const { AutoLock(this); return m_pclientbase;   }
IRuntime *                ServiceBase::getRuntime() const { AutoLock(this); return m_Runtime;       }
IRuntimeClient *    ServiceBase::getRuntimeClient() const { AutoLock(this); return m_RuntimeClient; }
AALServiceModule * ServiceBase::pAALServiceModule() const { AutoLock(this); return m_pcontainer;    }

void ServiceBase::allocService(IBase                  *pClient,
                               NamedValueSet const    &rManifest,
                               TransactionID const    &rTranID)
{
   getRuntime()->allocService(pClient, rManifest, rTranID);
}

void ServiceBase::Released()
{
   AutoLock(this);
   // Mark as not OK before deleting self or it will recurse Releasing
   m_bIsOK = false;
   m_pcontainer->ServiceReleased(this);
}

void ServiceBase::initComplete(TransactionID const &rtid)
{
   init(rtid);
}

#if DEPRECATED
void ServiceBase::messageHandler(const IEvent &rEvent)
{
   // Forward the event to the static event handler
   ASSERT(false);
}
#endif // DEPRECATED

ServiceBase::ServiceBase(const ServiceBase & ) {/*empty*/}
ServiceBase & ServiceBase::operator = (const ServiceBase & ) { return *this; }


//=============================================================================
// Name: ServiceProxyBase
// Description: Constructor.
// Interface: public
// Inputs: AALServiceModule *container - Pointer to the Service Module that
//                                       provdes access to core facilities and
//                                       maintains a list of all Services
//                                       constructed by this module.
//         IAALTransport    *ptransport - Optional transport (remote only)
//         IAALMarshaller   *marshaller - Optional Marshaller (remote only)
//         IAALUnMarshaller *unmarshaller - Optional Unmarshaller (remote only)
// Comments:
//=============================================================================
ServiceProxyBase::ServiceProxyBase(AALServiceModule *container,
                                   IRuntime         *pAALRUNTIME,
                                   IAALTransport    *ptransport,
                                   IAALMarshaller   *marshaller,
                                   IAALUnMarshaller *unmarshaller) :
   ServiceBase(container,
               pAALRUNTIME,
               ptransport,
               marshaller,
               unmarshaller),
   m_pcmpltEvent(NULL)
 {}

IBase * ServiceProxyBase::_init(IBase               *pclient,
                                TransactionID const &rtid,
                                NamedValueSet const &optArgs,
                                CAALEvent           *pcmpltEvent)
{
   // Check to see if this is the direct super class of the most derived class
   if ( NULL != pclient ) {

      // No then save the completion event and post it when our initialization
      //  completes
      m_pcmpltEvent = pcmpltEvent;
      return this;
   }

   ServiceBase::_init(pclient,
                      rtid,
                      optArgs,
                      new InitComplete<ServiceProxyBase>(this, &ServiceProxyBase::Doinit, rtid));
   return this;
}


void ServiceProxyBase::Doinit(TransactionID const &rtid)
{
   if ( !HasTransport() ) {
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                      Client(),
                                                                      dynamic_cast<IBase *>(this),
                                                                      rtid,
                                                                      errCreationFailure,
                                                                      reasNoDevice,
                                                                      "No transport provided to proxy class"));
      return;
   }

   // If there is a transport use it to connect
   m_ptransport->connectremote(OptArgs());

   // Create the remote side object
   marshall().Empty();  // Just to be sure
   marshall().Add(AAL_SERVICE_PROXY_INTERFACE_METHOD,       eNew);
   marshall().Add(AAL_SERVICE_PROXY_INTERFACE,              this);
   marshall().Add(AAL_SERVICE_PROXY_INTERFACE_NEW_OPTARGS, &m_optArgs);

   if ( !sendmsg() ) {
      m_ptransport->disconnect();
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                      Client(),
                                                                      dynamic_cast<IBase *>(this),
                                                                      rtid,
                                                                      errCreationFailure,
                                                                      reasInternalError,
                                                                      "Failed to send NEW message to server"));

      return;
   }

   //===========================
   //
   //  INITIALIZATION COMPLETE
   //
   //===========================

   // If this superclass' _init() was called with a completion Event to queue
   //   then there is more initialzation to go (i.e., this is not the direct parent of the
   //   most derived class) otherwise the only thing left to do is to call the subclass'
   //   init() method
   //
   if ( NULL != m_pcmpltEvent ) {
      getRuntime()->schedDispatchable(m_pcmpltEvent);
   } else {
      // Last superclass before most derived so call init()
      init(rtid);
   }
}



ServiceStubBase::ServiceStubBase(AALServiceModule *container,
                                 IRuntime         *pAALRUNTIME,
                                 IAALTransport    *ptransport,
                                 IAALMarshaller   *marshaller,
                                 IAALUnMarshaller *unmarshaller) :
   ServiceBase(container,
               pAALRUNTIME,
               ptransport,
               marshaller,
               unmarshaller),
   m_pcmpltEvent(NULL)
{}

IBase * ServiceStubBase::_init(IBase               *pclient,
                               TransactionID const &rtid,
                               NamedValueSet const &optArgs,
                               CAALEvent           *pcmpltEvent)
{
   // Check to see if this is the direct super class of the most derived class
   if ( NULL != pclient ) {
      // No then save the completion event and post it when our initialization
      //  completes
      m_pcmpltEvent = pcmpltEvent;
      return this;
   }

   ServiceBase::_init(pclient,
                      rtid,
                      optArgs,
                      new InitComplete<ServiceStubBase>(this, &ServiceStubBase::Doinit, rtid));
   return this;
}

void ServiceStubBase::Doinit(TransactionID const &rtid)
{
   // If there
   if ( !HasTransport() ) {
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                      Client(),
                                                                      dynamic_cast<IBase *>(this),
                                                                      rtid,
                                                                      errCreationFailure,
                                                                      reasNoDevice,
                                                                      "No transport provided to stub class"));
      return;
   }

   // If there is a transport use it to connect
   m_ptransport->waitforconnect(m_optArgs);

   //===========================
   //
   //  INITIALIZATION COMPLETE
   //
   //===========================

   // If this superclass' _init() was called with a completion Event to queue
   //   then there is more initialzation to go (i.e., this is not the direct parent of the
   //   most derived class) otherwise the only thing left to do is to call the subclass'
   //   init() method
   //
   if ( NULL != m_pcmpltEvent ) {
      getRuntime()->schedDispatchable(m_pcmpltEvent);
   } else {
      // Last superclass before most derived so call init()
      init(rtid);
   }
}

END_NAMESPACE(AAL)

