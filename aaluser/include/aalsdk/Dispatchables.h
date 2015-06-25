// Copyright (c) 2015, Intel Corporation
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
//        FILE: Dispatchables.h
//     CREATED: May 15, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE: This file implements convenient IDispatchble based classes.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 05/15/2015     JG       Initial Version
//****************************************************************************///
#ifndef __AALSDK_DISPATCHABLES_H__
#define __AALSDK_DISPATCHABLES_H__
#include <aalsdk/AALDefs.h>
#include <aalsdk/osal/IDispatchable.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/Runtime.h>

BEGIN_NAMESPACE(AAL)

//============================================================================
// AAL Service Client
//============================================================================
class ServiceClientCallback : public IDispatchable
{
public:
   enum MessageType{
      Allocated,
      AllocateFailed,
      Released,
      ReleaseFailed,
      Event
   };

   ServiceClientCallback(enum MessageType        type,
                         IServiceClient          *pClient,
                         IBase                   *pServiceBase,
                         TransactionID const     &rTranID = TransactionID() ) :
   m_type(type),
   m_pClient(pClient),
   m_pServiceBase(pServiceBase),
   m_TranID(rTranID),
   m_pEvent(NULL)
   {
      ASSERT(NULL != pClient);
      ASSERT(NULL != pServiceBase);
   }

   ServiceClientCallback(enum MessageType        type,
                         IServiceClient         *pClient,
                         IBase                  *pServiceBase,
                         const IEvent           *pEvent=NULL ) :
   m_type(type),
   m_pClient(pClient),
   m_pServiceBase(pServiceBase),
   m_TranID(),
   m_pEvent(pEvent)
   {
      ASSERT(NULL != pClient);
      ASSERT(NULL != pServiceBase);
   }


void operator() ()
{
   switch ( m_type ) {
      case Allocated : {
         m_pClient->serviceAllocated(m_pServiceBase, m_TranID);
      } break;
      case AllocateFailed : {
         m_pClient->serviceAllocateFailed(*m_pEvent);
      } break;
      case Released : {
         m_pClient->serviceReleased(m_TranID);
      } break;
      case ReleaseFailed : {
         m_pClient->serviceReleaseFailed(*m_pEvent);
      } break;
      case Event : {
         m_pClient->serviceEvent(*m_pEvent);
         // Delete the event object as it didn't render itself
         delete m_pEvent;
      } break;
      default:
         ASSERT(false);
      break;
   }
   delete this;
}

virtual ~ServiceClientCallback() {}

protected:
   IServiceClient          *m_pClient;
   IBase                   *m_pServiceBase;
   enum MessageType         m_type;
   TransactionID const      m_TranID;
   IEvent const            *m_pEvent;
};

//============================================================================
// AAL Runtime
//============================================================================
class RuntimeCallback : public IDispatchable
{
public:
   enum MessageType{
      CreateorGetProxyFailed,
      AllocateFailed,
      ServiceAllocated,
      Started,
      StartFailed,
      StopFailed,
      Stopped,
      Event
   };

   RuntimeCallback( enum MessageType       type,
                   IRuntimeClient         *po,
                   IRuntime               *prt,
                   const NamedValueSet    &rConfigParms,
                   const IEvent           *pEvent=NULL) :
   m_type(type),
   m_pobject(po),
   m_prt(prt),
   m_so(NULL),
   m_rConfigParms(rConfigParms),
   m_pEvent(pEvent)
{}

   RuntimeCallback(enum MessageType        type,
                   IRuntimeClient          *po,
                   IRuntime                *prt) :
   m_type(type),
   m_pobject(po),
   m_prt(prt),
   m_so(NULL),
   m_rConfigParms(NamedValueSet()),
   m_pEvent(NULL)
{}

   RuntimeCallback(enum MessageType        type,
                   IBase                   *so,
                   TransactionID const     &rtid) :
   m_type(type),
   m_pobject(NULL),
   m_prt(NULL),
   m_so(so),
   m_rConfigParms(NamedValueSet()),
   m_rTranID(rtid),
   m_pEvent(NULL)
{}

   RuntimeCallback(enum MessageType        type,
                   IRuntimeClient          *po,
                   const IEvent            *pEvent) :
   m_type(type),
   m_pobject(po),
   m_prt(NULL),
   m_rConfigParms(NamedValueSet()),
   m_pEvent(pEvent)
{}

void operator() ()
{
   switch ( m_type ) {
      case CreateorGetProxyFailed: {
         m_pobject->runtimeCreateOrGetProxyFailed(*m_pEvent);
      }break;
      case Started : {
         m_pobject->runtimeStarted(m_prt, m_rConfigParms);
      } break;
      case StartFailed : {
         m_pobject->runtimeStartFailed(*m_pEvent);
      } break;
      case ServiceAllocated : {
         m_pobject->runtimeAllocateServiceSucceeded(m_so, m_rTranID);
      } break;
      case AllocateFailed : {
         m_pobject->runtimeAllocateServiceFailed(*m_pEvent);
      } break;
      case Stopped : {
         m_pobject->runtimeStopped(m_prt);
      } break;
      case Event : {
         m_pobject->runtimeEvent(*m_pEvent);
         // Delete the event object as it didn't render itself
         delete m_pEvent;
      } break;
      default:
         ASSERT(false);
      break;
   }
   delete this;
}

virtual ~RuntimeCallback() {}

protected:
   IRuntimeClient          *m_pobject;
   IRuntime                *m_prt;
   IBase                   *m_so;
   const NamedValueSet     &m_rConfigParms;
   enum MessageType         m_type;
   TransactionID const      m_rTranID;
   IEvent const            *m_pEvent;
};

END_NAMESPACE(AAL)

#endif // __AALSDK_DISPATCHABLES_H__

