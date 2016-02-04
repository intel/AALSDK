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
//        FILE: runtime.cpp
//     CREATED: Mar 3, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: Implementation of AAL Runtime classes
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/21/2015     TSW      Initial version.
//****************************************************************************///
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// AAL Runtime definitions
#include "aalsdk/AALTypes.h"
#include "aalsdk/Runtime.h"
//#include "_RuntimeImpl.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/aas/Dispatchables.h"

/// @addtogroup AAL Runtime
/// @{

BEGIN_NAMESPACE(AAL)


ServiceAllocated::ServiceAllocated(IServiceClient      *pSvcClient,
                                   IRuntimeClient      *pRTClient,
                                   IBase               *pServiceBase,
                                   TransactionID const &rTranID) :
   m_pSvcClient(pSvcClient),
   m_pRTClient(pRTClient),
   m_pServiceBase(pServiceBase),
   m_TranID(rTranID)
{
   ASSERT(NULL != m_pServiceBase);
}

void ServiceAllocated::operator() ()
{
   IServiceClient *pSvcClient = NULL;
   IRuntimeClient *pRTClient  = NULL;

   // Process the TransactionID.
   if ( NULL != m_TranID.Ibase() ) {
      pSvcClient = dynamic_ptr<IServiceClient>(iidServiceClient, m_TranID.Ibase());
      pRTClient  = dynamic_ptr<IRuntimeClient>(iidRuntimeClient, m_TranID.Ibase());

      if ( NULL != pSvcClient ) {
         pSvcClient->serviceAllocated(m_pServiceBase, m_TranID);
      }

      if ( NULL != pRTClient ) {
         pRTClient->runtimeAllocateServiceSucceeded(m_pServiceBase, m_TranID);
      }
   }

   if ( ( NULL == m_TranID.Ibase() ) || !m_TranID.Filter() ) {
      pSvcClient = m_pSvcClient;
      pRTClient  = m_pRTClient;

      if ( NULL != pSvcClient ) {
         pSvcClient->serviceAllocated(m_pServiceBase, m_TranID);
      }

      if ( NULL != pRTClient ) {
         pRTClient->runtimeAllocateServiceSucceeded(m_pServiceBase, m_TranID);
      }
   }

   delete this;
}


ServiceAllocateFailed::ServiceAllocateFailed(IServiceClient *pSvcClient,
                                             IRuntimeClient *pRTClient,
                                             const IEvent   *pEvent) :
   m_pSvcClient(pSvcClient),
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pSvcClient || NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

ServiceAllocateFailed::~ServiceAllocateFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void ServiceAllocateFailed::operator() ()
{
   if ( NULL != m_pSvcClient ) {
      m_pSvcClient->serviceAllocateFailed(*m_pEvent);
   }

   if ( NULL != m_pRTClient ) {
      m_pRTClient->runtimeAllocateServiceFailed(*m_pEvent);
   }

   delete this;
}


DestroyServiceObject::DestroyServiceObject(ISvcsFact *pSvcsFact,
                                           IBase     *pService) :
   m_pSvcsFact(pSvcsFact),
   m_pService(pService)
{
   ASSERT(NULL != m_pSvcsFact);
   ASSERT(NULL != m_pService);
}

void DestroyServiceObject::operator() ()
{
   if ( NULL != m_pSvcsFact ) {
      m_pSvcsFact->DestroyServiceObject(m_pService);
   }
   delete this;
}

ServiceReleased::ServiceReleased(IServiceClient      *pSvcClient,
                                 IBase               *pServiceBase,
                                 TransactionID const &rTranID) :
   m_pSvcClient(pSvcClient),
   m_pServiceBase(pServiceBase),
   m_TranID(rTranID)
{
   ASSERT(NULL != m_pSvcClient);
   ASSERT(NULL != m_pServiceBase);
}

void ServiceReleased::operator() ()
{
   IServiceBase *pSvcBase = dynamic_ptr<IServiceBase>(iidServiceBase, m_pServiceBase);
   ASSERT(NULL != pSvcBase);
   if ( NULL != pSvcBase ) {
      pSvcBase->ReleaseComplete();
   }

   IServiceClient *pSvcClient = NULL;

   // Process the TransactionID.
   if ( NULL != m_TranID.Ibase() ) {
      pSvcClient = dynamic_ptr<IServiceClient>(iidServiceClient, m_TranID.Ibase());

      if ( NULL != pSvcClient ) {
         pSvcClient->serviceReleased(m_TranID);
      }
   }

   if ( ( NULL == m_TranID.Ibase() ) || !m_TranID.Filter() ) {
      pSvcClient = m_pSvcClient;

      if ( NULL != pSvcClient ) {
         pSvcClient->serviceReleased(m_TranID);
      }
   }

   delete this;
}

ServiceReleaseFailed::ServiceReleaseFailed(IServiceClient *pSvcClient,
                                           const IEvent   *pEvent) :
   m_pSvcClient(pSvcClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pSvcClient);
   ASSERT(NULL != m_pEvent);
}

ServiceReleaseFailed::~ServiceReleaseFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void ServiceReleaseFailed::operator() ()
{
   if ( NULL != m_pSvcClient ) {
      m_pSvcClient->serviceReleaseFailed(*m_pEvent);
   }

   delete this;
}

ServiceEvent::ServiceEvent(IServiceClient *pSvcClient,
                           const IEvent   *pEvent) :
   m_pSvcClient(pSvcClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pSvcClient);
   ASSERT(NULL != m_pEvent);
}

ServiceEvent::~ServiceEvent()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void ServiceEvent::operator() ()
{
   if ( NULL != m_pSvcClient ) {
      m_pSvcClient->serviceEvent(*m_pEvent);
   }

   delete this;
}

////////////////////////////////////////////////////////////////////////////////

RuntimeCreateOrGetProxyFailed::RuntimeCreateOrGetProxyFailed(IRuntimeClient *pRTClient,
                                                             const IEvent   *pEvent) :
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

RuntimeCreateOrGetProxyFailed::~RuntimeCreateOrGetProxyFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void RuntimeCreateOrGetProxyFailed::operator() ()
{
   m_pRTClient->runtimeCreateOrGetProxyFailed(*m_pEvent);
   delete this;
}

RuntimeStarted::RuntimeStarted(IRuntimeClient      *pRTClient,
                               IRuntime            *pRT,
                               const NamedValueSet &rConfigParms) :
   m_pRTClient(pRTClient),
   m_pRT(pRT),
   m_rConfigParms(rConfigParms)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pRT);
}

void RuntimeStarted::operator() ()
{
   m_pRTClient->runtimeStarted(m_pRT, m_rConfigParms);
   delete this;
}

RuntimeStartFailed::RuntimeStartFailed(IRuntimeClient *pRTClient,
                                       const IEvent   *pEvent) :
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

RuntimeStartFailed::~RuntimeStartFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void RuntimeStartFailed::operator() ()
{
   m_pRTClient->runtimeStartFailed(*m_pEvent);
   delete this;
}

RuntimeStopped::RuntimeStopped(IRuntimeClient *pRTClient,
                               IRuntime       *pRT) :
   m_pRTClient(pRTClient),
   m_pRT(pRT)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pRT);
}

void RuntimeStopped::operator() ()
{
   m_pRTClient->runtimeStopped(m_pRT);
   delete this;
}

RuntimeStopFailed::RuntimeStopFailed(IRuntimeClient *pRTClient,
                                     const IEvent   *pEvent) :
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

RuntimeStopFailed::~RuntimeStopFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void RuntimeStopFailed::operator() ()
{
   m_pRTClient->runtimeStopFailed(*m_pEvent);
   delete this;
}

RuntimeAllocateServiceSucceeded::RuntimeAllocateServiceSucceeded(IRuntimeClient      *pRTClient,
                                                                 IBase               *pServiceBase,
                                                                 TransactionID const &rTranID) :
   m_pRTClient(pRTClient),
   m_pServiceBase(pServiceBase),
   m_rTranID(rTranID)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pServiceBase);
}

void RuntimeAllocateServiceSucceeded::operator() ()
{
   m_pRTClient->runtimeAllocateServiceSucceeded(m_pServiceBase, m_rTranID);
   delete this;
}

RuntimeAllocateServiceFailed::RuntimeAllocateServiceFailed(IRuntimeClient *pRTClient,
                                                           const IEvent   *pEvent) :
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

RuntimeAllocateServiceFailed::~RuntimeAllocateServiceFailed()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void RuntimeAllocateServiceFailed::operator() ()
{
   m_pRTClient->runtimeAllocateServiceFailed(*m_pEvent);
   delete this;
}

RuntimeEvent::RuntimeEvent(IRuntimeClient *pRTClient,
                           const IEvent   *pEvent) :
   m_pRTClient(pRTClient),
   m_pEvent(pEvent)
{
   ASSERT(NULL != m_pRTClient);
   ASSERT(NULL != m_pEvent);
}

RuntimeEvent::~RuntimeEvent()
{
   if ( NULL != m_pEvent ) {
      delete m_pEvent;
   }
}

void RuntimeEvent::operator() ()
{
   m_pRTClient->runtimeEvent(*m_pEvent);
   delete this;
}

ServiceRevoke::ServiceRevoke(IServiceRevoke *pRevoke)
: m_pRevoke(pRevoke)
{
   ASSERT(NULL != m_pRevoke);
}

void ServiceRevoke::operator ()()
{
   m_pRevoke->serviceRevoke();
   delete this;
}

END_NAMESPACE(AAL)

/// @}

