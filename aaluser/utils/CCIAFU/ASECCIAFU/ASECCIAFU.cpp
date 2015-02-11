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
/// @file ASECCIAFU.cpp
/// @brief Implementation of ASE CCI AFU Service.
/// @ingroup ASECCIAFU
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
/// 07/31/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/service/ICCIClient.h>
#include <aalsdk/ase/ase_common.h>

#include "ASECCIAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ASECCIAFU
/// @{

CriticalSection ASECCIAFU::sm_ASEMtx;

void ASECCIAFU::init(TransactionID const &TranID)
{
   QueueAASEvent( new(std::nothrow) ObjectCreatedEvent(getRuntimeClient(),
                                                       Client(),
                                                       dynamic_cast<IBase *>(this),
                                                       TranID) );
}

btBool ASECCIAFU::Release(TransactionID const &TranID, btTime timeout)
{
   return ServiceBase::Release(TranID, timeout);
}

btBool ASECCIAFU::Release(btTime timeout)
{
   return ServiceBase::Release(timeout);
}


void ASECCIAFU::WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID)
{
   buffer_t                  buf;
   std::pair<map_iter, bool> res;
   btcString                 descr = NULL;

   ::memset(&buf, 0, sizeof(buffer_t));

   buf.memsize = (uint32_t)Length;

   ASECCIAFU::sm_ASEMtx.Lock();
   allocate_buffer(&buf);
   ASECCIAFU::sm_ASEMtx.Unlock();

   if ( ( ASE_BUFFER_VALID != buf.valid )   ||
        ( MAP_FAILED == (void *)buf.vbase ) ||
        ( 0 == buf.fake_paddr ) ) {
      descr = "allocate_buffer()";
      goto _SEND_ERR;
   }

   Lock();
   // Map the virtual address to its internal representation.
   res = m_WkspcMap.insert(std::make_pair((btVirtAddr)buf.vbase, buf));
   Unlock();

   if ( !res.second ) {
      descr = "map.insert()";
      goto _SEND_ERR;
   }

   SendMsg( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
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
   SendMsg( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                               pExcept));
}

void ASECCIAFU::WorkspaceFree(btVirtAddr           Address,
                              TransactionID const &TranID)
{
   btcString descr = NULL;

   Lock();

   // Find the internal structure.
   map_iter iter = m_WkspcMap.find(Address);
   buffer_t buf;

   if ( m_WkspcMap.end() == iter ) {
      // No mapping for Address exists.
      Unlock();
      descr = "no such address";
      goto _SEND_ERR;
   }

   buf = (*iter).second;

   ASECCIAFU::sm_ASEMtx.Lock();
   deallocate_buffer(&buf);
   ASECCIAFU::sm_ASEMtx.Unlock();

   // Remove the mapping.
   m_WkspcMap.erase(iter);

   Unlock();

   SendMsg( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                      TranID) );
   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                  TranID,
                                                                  errAFUWorkSpace,
                                                                  reasAFUNoMemory,
                                                                  descr);
   SendMsg( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                           pExcept));
}

btBool ASECCIAFU::CSRRead(btCSROffset CSR,
                          btCSRValue *pValue)
{
   if ( __UINTPTR_T_CONST(0x344) == CSR ) {
      *pValue = m_Last3c4;
   } else if ( __UINTPTR_T_CONST(0x34c) == CSR ) {
      *pValue = m_Last3cc;
   } else {
      ASECCIAFU::sm_ASEMtx.Lock();
      *pValue = (btCSRValue) csr_read(CSR);
      ASECCIAFU::sm_ASEMtx.Unlock();
   }

   return true;
}

btBool ASECCIAFU::CSRWrite(btCSROffset CSR,
                           btCSRValue  Value)
{
   if ( __UINTPTR_T_CONST(0x3c4) == CSR ) {
      m_Last3c4 = Value;
   } else if ( __UINTPTR_T_CONST(0x3cc) == CSR ) {
      m_Last3cc = Value;
   } else {
      ASECCIAFU::sm_ASEMtx.Lock();
      csr_write(CSR, (bt32bitCSR)Value);
      ASECCIAFU::sm_ASEMtx.Unlock();
   }

   return true;
}

btBool ASECCIAFU::CSRWrite64(btCSROffset CSR,
                             bt64bitCSR  Value)
{
   if ( CSRWrite(CSR + 4, Value >> 32) ) {
      return CSRWrite(CSR, Value & 0xffffffff);
   }
   return false;
}

/// @} group ASECCIAFU

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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::ASECCIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

ASECCIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
ASECCIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

