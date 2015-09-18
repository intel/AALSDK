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
/// @file ASEALIAFU.cpp
/// @brief Implementation of ASE ALI AFU Service.
/// @ingroup ASEALIAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// This sample demonstrates how to create an AFU Service that uses a host-based AFU engine.
///  This design also applies to AFU Services that use hardware via a
///  Physical Interface Protocol (PIP) module.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/service/ICCIClient.h>
#include <aalsdk/ase/ase_common.h>

#include "ASEALIAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ASEALIAFU
/// @{

CriticalSection ASEALIAFU::sm_ASEMtx;

btBool ASEALIAFU::init( IBase *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &TranID)
{
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, ServiceClientBase());
   ASSERT( NULL != pClient );
   if ( NULL == pClient ) {
      /// ObjectCreatedExceptionEvent Constructor.
      getRuntime()->schedDispatchable( new ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                       ServiceClient(),
                                                                       this,
                                                                       TranID,
                                                                       errBadParameter,
                                                                       reasMissingInterface,
                                                                       "Client did not publish ICCIClient Interface") );
      return false;
   }

  session_init();
  getRuntime()->schedDispatchable( new(std::nothrow) ObjectCreatedEvent(getRuntimeClient(),
                                                                        ServiceClient(),
                                                                        dynamic_cast<IBase *>(this),
                                                                        TranID) );
  return true;
}

btBool ASEALIAFU::Release(TransactionID const &TranID, btTime timeout)
{
  session_deinit();
  return ServiceBase::Release(TranID, timeout);
}

btBool ASEALIAFU::Release(btTime timeout)
{
  session_deinit();
  return ServiceBase::Release(timeout);
}


void ASEALIAFU::WorkspaceAllocate(btWSSize             Length,
                                    TransactionID const &TranID)
{
  buffer_t                  buf;
  std::pair<map_iter, bool> res;
  btcString                 descr = NULL;

  ::memset(&buf, 0, sizeof(buffer_t));

  buf.memsize = (uint32_t)Length;

  {
     AutoLock(&ASEALIAFU::sm_ASEMtx);
     allocate_buffer(&buf);
  }

  if ( ( ASE_BUFFER_VALID != buf.valid )   ||
       ( MAP_FAILED == (void *)buf.vbase ) ||
       ( 0 == buf.fake_paddr ) ) {
    descr = "allocate_buffer()";
    goto _SEND_ERR;
  }

  {
     AutoLock(this);
     // Map the virtual address to its internal representation.
     res = m_WkspcMap.insert(std::make_pair((btVirtAddr)buf.vbase, buf));
  }

  if ( !res.second ) {
    descr = "map.insert()";
    goto _SEND_ERR;
  }

  getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, ServiceClientBase()),
                                                                                 TranID,
                                                                                 (btVirtAddr)buf.vbase,
                                                                                 (btPhysAddr)buf.fake_paddr,
                                                                                 (btWSSize)buf.memsize) );
  return;

 _SEND_ERR:
  IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                 TranID,
                                                                 errAFUWorkSpace,
                                                                 reasAFUNoMemory,
                                                                 descr);
  getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ServiceClientBase()),
                                                                                      pExcept));
}

void ASEALIAFU::WorkspaceFree(btVirtAddr           Address,
                                TransactionID const &TranID)
{
   btcString descr = NULL;
   map_iter  iter;
   buffer_t  buf;

   {
      AutoLock(this);

      // Find the internal structure.
      iter = m_WkspcMap.find(Address);

      if ( m_WkspcMap.end() == iter ) {
         // No mapping for Address exists.
         descr = "no such address";
         goto _SEND_ERR;
      }

      buf = (*iter).second;

      {
         AutoLock(&ASEALIAFU::sm_ASEMtx);
         deallocate_buffer(&buf);
      }

      // Remove the mapping.
      m_WkspcMap.erase(iter);
   }

  getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, ServiceClientBase()),
                                                                             TranID) );
  return;

 _SEND_ERR:
  IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                 TranID,
                                                                 errAFUWorkSpace,
                                                                 reasAFUNoMemory,
                                                                 descr);
  getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ServiceClientBase()),
                                                                                  pExcept));
}

btBool ASEALIAFU::CSRRead(btCSROffset CSR,
                            btCSRValue *pValue)
{
   if ( __UINTPTR_T_CONST(0x344) == CSR ) {
      *pValue = m_Last3c4;
   } else if ( __UINTPTR_T_CONST(0x34c) == CSR ) {
      *pValue = m_Last3cc;
   } else {
      AutoLock(&ASEALIAFU::sm_ASEMtx);
      *pValue = (btCSRValue) csr_read(CSR);
   }
   return true;
}

btBool ASEALIAFU::CSRWrite(btCSROffset CSR,
                             btCSRValue  Value)
{
   if ( __UINTPTR_T_CONST(0x3c4) == CSR ) {
      m_Last3c4 = Value;
   } else if ( __UINTPTR_T_CONST(0x3cc) == CSR ) {
      m_Last3cc = Value;
   } else {
      AutoLock(&ASEALIAFU::sm_ASEMtx);
      csr_write(CSR, (bt32bitCSR)Value);
   }
   return true;
}

btBool ASEALIAFU::CSRWrite64(btCSROffset CSR,
                               bt64bitCSR  Value)
{
  if ( CSRWrite(CSR + 4, Value >> 32) ) {
    return CSRWrite(CSR, Value & 0xffffffff);
  }
  return false;
}

/// @} group ASEALIAFU

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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::ASEALIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

ASEALIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
/* No commands other than default, at the moment. */
ASEALIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

