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
// PURPOSE: Implementation of XL _xlruntime class
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

#include "_xlRuntimeImpl.h"
#include "_xlMessageDelivery.h"
#include "_xlServiceBroker.h"

#include "aalsdk/osal/Sleep.h"
#include "aalsdk/osal/Env.h"

#include "aalsdk/INTCDefs.h"

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(XL)
      BEGIN_NAMESPACE(RT)


BEGIN_C_DECLS
IRuntime *_getnewXLRuntimeInstance()
{
   _xlruntime *pruntime = new _xlruntime;
   return pruntime->IsOK() == true ? pruntime : NULL;
}
END_C_DECLS


//=============================================================================
// Name: _xlruntime
// Description: Constructor
// Interface: public
// Outputs: none.
// Comments:
//=============================================================================
_xlruntime::_xlruntime() :
   CAASBase(),
   m_status(false),
   m_pclient(NULL),
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


   // Instantiate the core facilities

   // MDS
   m_pMDSSvcHost = new ServiceHost(AAL_SVC_MOD_ENTRY_POINT(localMDS), this, this);
   m_pMDSSvcHost->allocService( this, NamedValueSet(), TransactionID(MDS), true);
   m_sem.Wait(); // for the local Message Delivery Service

   // Service Broker
   m_pBrokerSvcHost = new ServiceHost(AAL_SVC_MOD_ENTRY_POINT(localServiceBroker),this,  this);
   m_pBrokerSvcHost->allocService( this, NamedValueSet(), TransactionID(Broker), true);
   m_sem.Wait(); // for the local Broker

   m_status = true;
}

//=============================================================================
// Name: IsOK
// Description: Is the object functional
// Interface: public
// returns: true if functional.
// Comments:
//=============================================================================
btBool _xlruntime::IsOK()
{
   return m_status;
}


//=============================================================================
// Name: stop
// Description: Stop the runtime
// Interface: public
// Comments:
//=============================================================================
void _xlruntime::stop()
{
   Lock();

   if ( IsOK() && (m_state !=Stopped) ) {
      m_status = false;
      Unlock();

      // Prepare our sem. We will wait for notification from serviceFreed() before continuing.
      m_sem.Reset(0);

      // Release the Service Broker.
      dynamic_ptr<IAALService>(iidService, m_pBrokerbase)->Release(TransactionID(Broker));
   } else {
      // Dispatch the event ourselves, because MDS is no more.
      OSLThreadGroup oneShot;
      RuntimeMessage *pRuntimeStopped = new RuntimeMessage(m_pclient,
                                            this,
                                            RuntimeMessage::Stopped);

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
// Inputs: pclient - Pointer to Runtime's client IBase interface
//         rconfigParms - Configuration parameters
// Outputs: none.
// Comments:
//=============================================================================
AAL::btBool _xlruntime::start( AAL::IBase      *pclient,
                               const NamedValueSet &rConfigParms)
{

   if( Started == m_state  ){
      // Runtime Failed to start because it already is
      SendMsg(new RuntimeMessage(m_pclient,
                                 this,
                                 rConfigParms,
                                 RuntimeMessage::StartFailed,
                                 new AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase*>(this),
                                                                          exttranevtSystemStart,
                                                                          TransactionID(),
                                                                          errSysSystemStarted,
                                                                          reasSystemAlreadyStarted,
                                                                          strSystemAlreadyStarted)),
                                 NULL);
      return false;
   }

   // Extract the IRuntimeClient Interface
   m_pclient = dynamic_ptr<IRuntimeClient>(iidRuntimeClient,pclient);
   if(NULL == m_pclient){
      // Have to return failure because no
      //  Interface to report through
      return false;
   }

   if ( !ProcessConfigParms(rConfigParms) ) {
      return false;
   }


   if ( IsOK() ) {
      m_state = Started;
      SendMsg(new RuntimeMessage(m_pclient,
                                 this,
                                 rConfigParms,
                                 RuntimeMessage::Started), NULL);
      return true;
   } else {
      // Runtime Failed to start
      SendMsg(new RuntimeMessage(m_pclient,
                                 this,
                                 rConfigParms,
                                 RuntimeMessage::StartFailed,
                                 new AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase*>(this),
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
// Name: ProcessConfigParms
// Description: Process config parms for Runtime configuration record
// Interface: public
// Inputs: rConfigParms - Config parms
// Outputs: none.
// Comments:
//=============================================================================
btBool _xlruntime::ProcessConfigParms(const NamedValueSet &rConfigParms)
{
   NamedValueSet const *pConfigRecord;
   btcString            sName  = NULL;
   AAL::Environment     env;

   //
   // First check environment
   if( NULL == (sName = env.getVal(XLRUNTIME_CONFIG_BROKER_SERVICE)) ){

      if ( ENamedValuesOK != rConfigParms.Get(XLRUNTIME_CONFIG_RECORD, &pConfigRecord) ) {
         // Check to see if default services are running
         return true;
      }

      if ( ENamedValuesOK == pConfigRecord->Get(XLRUNTIME_CONFIG_BROKER_SERVICE, &sName) ) {
         if ( NULL == m_pBrokerbase ) {
            // Runtime Failed to start
            SendMsg(new RuntimeMessage(m_pclient,
                                       this,
                                       rConfigParms,
                                       RuntimeMessage::StartFailed,
                                       new AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase*>(this),
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

      // Allocate the service. NOTE: Suppress the RuntimeClientEvent
      allocService(this, optArgs, TransactionID(Broker), NoRuntimeClientNotification);
      m_sem.Wait();
   }
   return true;
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
void _xlruntime::allocService(AAL::IBase *pClient,
                              NamedValueSet const      &rManifest,
                              TransactionID const      &rTranID,
                              AAL::XL::RT::IRuntime::eAllocatemode mode)
{
   AutoLock(this);
   if ( IsOK() ) {
      m_pBroker->allocService(pClient, rManifest, rTranID, mode);
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
void _xlruntime::schedDispatchable(IDispatchable *pdispatchable)
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
void _xlruntime::serviceAllocated(AAL::IBase          *pServiceBase,
                                  TransactionID const &rTranID )
{
   AutoLock(this);

   switch ( rTranID.ID() ) {
      case MDS : {
         m_pMDSbase = pServiceBase;
         m_pMDS     = subclass_ptr<AAL::AAS::IEventDeliveryService>(pServiceBase);
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
void _xlruntime::serviceAllocateFailed(const IEvent        &rEvent)
{
   m_status = false;
   m_sem.Post(1);
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
void _xlruntime::serviceFreed(TransactionID const &rTranID)
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
         RuntimeMessage *pRuntimeStopped = new RuntimeMessage(m_pclient,
 															  this,
															  RuntimeMessage::Stopped);

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
void _xlruntime::serviceEvent(const IEvent &rEvent)
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
AAL::IBase *_xlruntime::getMessageDeliveryService()
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
void _xlruntime::setMessageDeliveryService(AAL::IBase *pMDSbase)
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
btBool _xlruntime::SendMsg(IDispatchable *pobject, btObjectType parm)
{
   AutoLock(this);
   if ( NULL == m_pMDSbase ) {
      return false;
   }
   subclass_ref<AAL::AAS::IEventDeliveryService>(m_pMDSbase).QueueEvent(parm, pobject);
   return true;         // TODO cleanup IEventdeliveryService
}

//=============================================================================
// Name: getRuntimeClient
// Description: return the Runtime's Client's Interface
// Interface: public
// Inputs: pobject - Dispatchable object to send
//         parm - Parameter
// Outputs: none.
// Comments:
//=============================================================================
IRuntimeClient *_xlruntime::getRuntimeClient()
{
   AutoLock(this);
   return m_pclient;
}



//=============================================================================
// Name: ~_xlruntime
// Description: Destructor
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
_xlruntime::~_xlruntime()
{
   AutoLock(this);
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


      END_NAMESPACE(RT)
   END_NAMESPACE(XL)
END_NAMESPACE(AAL)
