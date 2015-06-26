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
/// @file SPLAFU.cpp
/// @brief Implementation of SPL AFU Service.
/// @ingroup SPLAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// This sample demonstrates how to create an AFU Service that uses a host-based AFU engine.
///  This design also applies to AFU Services that use hardware via a
///  Physical Interface Protocol (PIP) module.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/18/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALBase.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/Dispatchables.h>
#include <aalsdk/service/HWSPLAFUService.h>
#include <aalsdk/service/ASESPLAFUService.h>
#include <aalsdk/service/SWSimSPLAFUService.h>

#include "SPLAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup SPLAFU
/// @{

void SPLAFU::init(TransactionID const &TranID)
{
   ISPLClient *pClient = dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase());
   ASSERT( NULL != pClient );
   if(NULL == pClient){
      /// ObjectCreatedExceptionEvent Constructor.
      getRuntime()->schedDispatchable(new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                      Client(),
                                                                      this,
                                                                      TranID,
                                                                      errBadParameter,
                                                                      reasMissingInterface,
                                                                      "Client did not publish ISPLClient Interface"));
      return;
   }

   // Default target is hardware AFU.
   NamedValueSet manifest(std::string(HWSPLAFU_MANIFEST));

   // Determine which AFU implementation has been requested (HW vs ASE).
   if ( m_optArgs.Has(SPLAFU_NVS_KEY_TARGET) ) {

      m_optArgs.Get(SPLAFU_NVS_KEY_TARGET, &m_TargetAFU);
      ASSERT(NULL != m_TargetAFU);

      if ( 0 == std::string(m_TargetAFU).compare(SPLAFU_NVS_VAL_TARGET_ASE) ) {
         manifest = NamedValueSet(std::string(ASESPLAFU_MANIFEST));
      } else if ( 0 == std::string(m_TargetAFU).compare(SPLAFU_NVS_VAL_TARGET_SWSIM) ) {
         manifest = NamedValueSet(std::string(SWSIMSPLAFU_MANIFEST));
      }
   }

   // TODO Use wrap/unwrap utils.
   m_TranIDFrominit = TranID;
   allocService(dynamic_ptr<IBase>(iidBase, this), manifest, TransactionID());
}

btBool SPLAFU::Release(TransactionID const &TranID, btTime timeout)
{
   if ( NULL != m_pDelegate ) {
      dynamic_cast<IAALService *>(m_pDelegate)->Release(TransactionID(), timeout);
      m_TranIDFromRelease  = TranID;
      m_TimeoutFromRelease = timeout;
      return true;
   }

   return ServiceBase::Release(TranID, timeout);
}

btBool SPLAFU::Release(btTime timeout)
{
   if ( NULL != m_pDelegate ) {
      dynamic_cast<IAALService *>(m_pDelegate)->Release(timeout);
      m_pDelegate = NULL;
   }

   return ServiceBase::Release(timeout);
}


void SPLAFU::serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &TranID)
{
   m_pDelegate = dynamic_ptr<ISPLAFU>(iidSPLAFU, pServiceBase);
   ASSERT(NULL != m_pDelegate);

   if ( NULL != m_TargetAFU ) {
      if ( 0 == std::string(m_TargetAFU).compare(SPLAFU_NVS_VAL_TARGET_FPGA) ) {
         SetInterface(iidHWSPLAFU, dynamic_ptr<ISPLAFU>(iidHWSPLAFU, pServiceBase));
      } else if ( 0 == std::string(m_TargetAFU).compare(SPLAFU_NVS_VAL_TARGET_ASE) ) {
         SetInterface(iidASESPLAFU, dynamic_ptr<ISPLAFU>(iidASESPLAFU, pServiceBase));
      } else if ( 0 == std::string(m_TargetAFU).compare(SPLAFU_NVS_VAL_TARGET_SWSIM) ) {
         SetInterface(iidSWSIMSPLAFU, dynamic_ptr<ISPLAFU>(iidSWSIMSPLAFU, pServiceBase));
      } else {
         ASSERT(false); // unsupported target AFU
      }
   } else { // default to hardware
      SetInterface(iidHWSPLAFU, dynamic_ptr<ISPLAFU>(iidHWSPLAFU, pServiceBase));
   }

   getRuntime()->schedDispatchable( new(std::nothrow) ObjectCreatedEvent(getRuntimeClient(),
                                                                         Client(),
                                                                         dynamic_cast<IBase *>(this),
                                                                         m_TranIDFrominit) );
}

void SPLAFU::serviceAllocateFailed(const IEvent &Event)
{
   // Reflect the error to the outer client.
   // TODO extract the Exception info and put in this event
   getRuntime()->schedDispatchable( new (std::nothrow) ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                                   Client(),
                                                                                   dynamic_cast<IBase *>(this),
                                                                                   m_TranIDFrominit,
                                                                                   errInternal,
                                                                                   reasCauseUnknown,
                                                                                   "Release Failed") );
}

void SPLAFU::serviceReleased(TransactionID const &TranID)
{
   m_pDelegate = NULL;
   ServiceBase::Release(m_TranIDFromRelease, m_TimeoutFromRelease);
}

void SPLAFU::serviceReleaseFailed(const IEvent &Event)
{
   // Reflect the error to the outer client.
   // TODO extract the Exception info and put in this event
   getRuntime()->schedDispatchable( new (std::nothrow) ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                                   Client(),
                                                                                   dynamic_cast<IBase *>(this),
                                                                                   m_TranIDFrominit,
                                                                                   errInternal,
                                                                                   reasCauseUnknown,
                                                                                   "Release Failed") );
}

void SPLAFU::serviceEvent(const IEvent &Event)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) ServiceClientCallback(ServiceClientCallback::Event,
                                                                            Client(),
                                                                            dynamic_cast<IBase *>(this),
                                                                            &Event) );
}


void SPLAFU::WorkspaceAllocate(btWSSize             Length,
                               TransactionID const &TranID)
{
   m_pDelegate->WorkspaceAllocate(Length, TranID);
}

void SPLAFU::WorkspaceFree(btVirtAddr           Address,
                           TransactionID const &TranID)
{
   m_pDelegate->WorkspaceFree(Address, TranID);
}

btBool SPLAFU::CSRRead(btCSROffset CSR,
                       btCSRValue *pValue)
{
   return m_pDelegate->CSRRead(CSR, pValue);
}

btBool SPLAFU::CSRWrite(btCSROffset CSR,
                        btCSRValue  Value)
{
   return m_pDelegate->CSRWrite(CSR, Value);
}

btBool SPLAFU::CSRWrite64(btCSROffset CSR,
                          bt64bitCSR  Value)
{
   return m_pDelegate->CSRWrite64(CSR, Value);
}

void SPLAFU::StartTransactionContext(TransactionID const &TranID,
                                     btVirtAddr           Address,
                                     btTime               Pollrate)
{
   m_pDelegate->StartTransactionContext(TranID, Address, Pollrate);
}

void SPLAFU::StopTransactionContext(TransactionID const &TranID)
{
   m_pDelegate->StopTransactionContext(TranID);
}


void SPLAFU::SetContextWorkspace(TransactionID const &TranID,
                                 btVirtAddr           Address,
                                 btTime               Pollrate)
{
   m_pDelegate->SetContextWorkspace(TranID, Address, Pollrate);
}


void SPLAFU::OnWorkspaceAllocated(TransactionID const &TranID,
                                  btVirtAddr           WkspcVirt,
                                  btPhysAddr           WkspcPhys,
                                  btWSSize             WkspcSize)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                  TranID,
                                                                                  WkspcVirt,
                                                                                  WkspcPhys,
                                                                                  WkspcSize) );
}

void SPLAFU::OnWorkspaceAllocateFailed(const IEvent &Event)
{
   // Reflect the message to the outer client.
   ISPLClient *pClient = dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase());
   ASSERT(NULL != pClient);
   if ( NULL != pClient ) {
      pClient->OnWorkspaceAllocateFailed(Event);
   }
}

void SPLAFU::OnWorkspaceFreed(TransactionID const &TranID)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                              TranID) );
}

void SPLAFU::OnWorkspaceFreeFailed(const IEvent &Event)
{
   // Reflect the message to the outer client.
   ISPLClient *pClient = dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase());
   ASSERT(NULL != pClient);
   if ( NULL != pClient ) {
      pClient->OnWorkspaceFreeFailed(Event);
   }
}

void SPLAFU::OnTransactionStarted(TransactionID const &TranID,
                                  btVirtAddr           AFUDSM,
                                  btWSSize             AFUDSMSize)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                  TranID,
                                                                                  AFUDSM,
                                                                                  AFUDSMSize) );
}

void SPLAFU::OnContextWorkspaceSet(TransactionID const &TranID)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientContextWorkspaceSet(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                   TranID) );
}

void SPLAFU::OnTransactionFailed(const IEvent &Event)
{
   // Reflect the message to the outer client.
   ISPLClient *pClient = dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase());
   ASSERT(NULL != pClient);
   if ( NULL != pClient ) {
      pClient->OnTransactionFailed(Event);
   }
}

void SPLAFU::OnTransactionComplete(TransactionID const &TranID)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionComplete(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                   TranID) );
}

void SPLAFU::OnTransactionStopped(TransactionID const &TranID)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStopped(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                  TranID) );
}

/// @} group SPLAFU

END_NAMESPACE(AAL)


#if defined( __AAL_WINDOWS__ )

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
         break;
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::SPLAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

SPLAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
SPLAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

