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
/// @file CCIAFU.cpp
/// @brief Implementation of CCI AFU Service.
/// @ingroup CCIAFU
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
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/18/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/service/HWCCIAFUService.h>
#include <aalsdk/service/ASECCIAFUService.h>
#include <aalsdk/service/SWSimCCIAFUService.h>
#include <aalsdk/Dispatchables.h>

#include "CCIAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup CCIAFU
/// @{

btBool CCIAFU::init( IBase *pclientBase,
                     NamedValueSet const &optArgs,
                     TransactionID const &rtid)
{
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase());
   ASSERT( NULL != pClient );
   if(NULL == pClient){
      /// ObjectCreatedExceptionEvent Constructor.
      initFailed(new CExceptionTransactionEvent( this,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Client did not publish ICCIClient Interface"));
      return false;
   }

   // Default target is hardware AFU.
   NamedValueSet manifest(std::string(HWCCIAFU_MANIFEST));

   // Determine which AFU implementation has been requested (HW, ASE, or SW Sim).
   if ( m_optArgs.Has(CCIAFU_NVS_KEY_TARGET) ) {

      m_optArgs.Get(CCIAFU_NVS_KEY_TARGET, &m_TargetAFU);
      ASSERT(NULL != m_TargetAFU);

      if ( 0 == std::string(m_TargetAFU).compare(CCIAFU_NVS_VAL_TARGET_ASE) ) {
         manifest = NamedValueSet(std::string(ASECCIAFU_MANIFEST));
      } else if ( 0 == std::string(m_TargetAFU).compare(CCIAFU_NVS_VAL_TARGET_SWSIM) ) {
         manifest = NamedValueSet(std::string(SWSIMCCIAFU_MANIFEST));
      }
   }

   // TODO Use wrap/unwrap utils.
   m_TranIDFrominit = rtid;
   allocService(dynamic_ptr<IBase>(iidBase, this), manifest, TransactionID());
   return true;
}

btBool CCIAFU::Release(TransactionID const &TranID, btTime timeout)
{
   if ( NULL != m_pDelegate ) {
      dynamic_cast<IAALService *>(m_pDelegate)->Release(TransactionID(), timeout);
      m_TranIDFromRelease  = TranID;
      m_TimeoutFromRelease = timeout;
      return true;
   }

   return ServiceBase::Release(TranID, timeout);
}

btBool CCIAFU::Release(btTime timeout)
{
   if ( NULL != m_pDelegate ) {
      dynamic_cast<IAALService *>(m_pDelegate)->Release(timeout);
      m_pDelegate = NULL;
      return true;
   }

   return ServiceBase::Release(timeout);
}


void CCIAFU::serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &TranID)
{
   m_pDelegate = dynamic_ptr<ICCIAFU>(iidCCIAFU, pServiceBase);
   ASSERT(NULL != m_pDelegate);

   if ( NULL != m_TargetAFU ) {
      if ( 0 == std::string(m_TargetAFU).compare(CCIAFU_NVS_VAL_TARGET_FPGA) ) {
         SetInterface(iidHWCCIAFU, dynamic_ptr<ICCIAFU>(iidHWCCIAFU, pServiceBase));
      } else if ( 0 == std::string(m_TargetAFU).compare(CCIAFU_NVS_VAL_TARGET_ASE) ) {
         SetInterface(iidASECCIAFU, dynamic_ptr<ICCIAFU>(iidASECCIAFU, pServiceBase));
      } else if ( 0 == std::string(m_TargetAFU).compare(CCIAFU_NVS_VAL_TARGET_SWSIM) ) {
         SetInterface(iidSWSIMCCIAFU, dynamic_ptr<ICCIAFU>(iidSWSIMCCIAFU, pServiceBase));
      } else {
         ASSERT(false); // unsupported target AFU
      }
   } else { // default to hardware
      SetInterface(iidHWCCIAFU, dynamic_ptr<ICCIAFU>(iidHWCCIAFU, pServiceBase));
   }

   initComplete(m_TranIDFrominit);
}

void CCIAFU::serviceAllocateFailed(const IEvent        &Event)
{
   // Reflect the error to the outer client.
// TODO extract the Exception info and put in this event
   initFailed( new(std::nothrow) CExceptionTransactionEvent( NULL,
                                                             m_TranIDFrominit,
                                                             errInternal,
                                                             reasCauseUnknown,
                                                             "Allocate Failed"));
}

void CCIAFU::serviceReleased(TransactionID const &TranID)
{
   m_pDelegate = NULL;
   ServiceBase::Release(m_TranIDFromRelease, m_TimeoutFromRelease);
}

void CCIAFU::serviceReleaseFailed(const IEvent        &Event)
{
   // Reflect the error to the outer client.
// TODO extract the Exception info and put in this event
   initFailed( new(std::nothrow) CExceptionTransactionEvent( NULL,
                                                             m_TranIDFrominit,
                                                             errInternal,
                                                             reasCauseUnknown,
                                                             "Release Failed"));
}

void CCIAFU::serviceEvent(const IEvent &Event)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) ServiceClientCallback(ServiceClientCallback::Event,
                                                                            getServiceClient(),
                                                                            getRuntimeClient(),
                                                                            dynamic_cast<IBase *>(this),
                                                                            &Event) );
}


// We delegate our ICCIAFU interface to the chosen implementation strategy.
void CCIAFU::WorkspaceAllocate(btWSSize             Length,
                               TransactionID const &TranID)
{
   m_pDelegate->WorkspaceAllocate(Length, TranID);
}

void CCIAFU::WorkspaceFree(btVirtAddr           Address,
                           TransactionID const &TranID)
{
   m_pDelegate->WorkspaceFree(Address, TranID);
}

btBool CCIAFU::CSRRead(btCSROffset CSR,
                       btCSRValue *pValue)
{
   return m_pDelegate->CSRRead(CSR, pValue);
}

btBool CCIAFU::CSRWrite(btCSROffset CSR,
                        btCSRValue  Value)
{
   return m_pDelegate->CSRWrite(CSR, Value);
}

btBool CCIAFU::CSRWrite64(btCSROffset CSR,
                          bt64bitCSR  Value)
{
   return m_pDelegate->CSRWrite64(CSR, Value);
}

void CCIAFU::OnWorkspaceAllocated(TransactionID const &TranID,
                                  btVirtAddr           WkspcVirt,
                                  btPhysAddr           WkspcPhys,
                                  btWSSize             WkspcSize)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                                                  TranID,
                                                                                  WkspcVirt,
                                                                                  WkspcPhys,
                                                                                  WkspcSize) );
}

void CCIAFU::OnWorkspaceAllocateFailed(const IEvent &Event)
{
   // Reflect the message to the outer client.
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase());
   ASSERT(NULL != pClient);
   if ( NULL != pClient ) {
      pClient->OnWorkspaceAllocateFailed(Event);
   }
}

void CCIAFU::OnWorkspaceFreed(TransactionID const &TranID)
{
   // Reflect the message to the outer client.
   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                      TranID) );
}

void CCIAFU::OnWorkspaceFreeFailed(const IEvent &Event)
{
   // Reflect the message to the outer client.
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase());
   ASSERT(NULL != pClient);
   if ( NULL != pClient ) {
      pClient->OnWorkspaceFreeFailed(Event);
   }
}

/// @}

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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::CCIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

CCIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
CCIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

