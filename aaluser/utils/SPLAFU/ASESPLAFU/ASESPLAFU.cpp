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
/// @file ASESPLAFU.cpp
/// @brief Implementation of SPL AFU Software Service.
/// @ingroup ASESPLAFU
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
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/service/ISPLClient.h>

#include "ASESPLAFU.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ASESPLAFU
/// @{

CriticalSection ASESPLAFU::sm_ASEMtx;

void ASESPLAFU::init(TransactionID const &TranID)
{
  // RRS: *FIXME* Test allocate_dsm here
  m_dsm = (struct buffer_t *) malloc (sizeof(struct buffer_t));
  m_dsm->memsize = 64*1024;
  allocate_buffer(m_dsm);

  // Issue SPL reset, Find SPL ID, and AFU ID
  spl_driver_reset(m_dsm);
  spl_driver_dsm_setup(m_dsm);
  spl_driver_afu_setup(m_dsm);

  QueueAASEvent( new(std::nothrow) AAL::AAS::ObjectCreatedEvent(getRuntimeClient(),
								Client(),
								dynamic_cast<IBase *>(this),
								TranID) );
}

btBool ASESPLAFU::Release(TransactionID const &TranID, btTime timeout)
{
  ASESPLAFU::sm_ASEMtx.Lock();
  // deallocate_buffer (m_spl_pt);
  // deallocate_buffer (m_spl_cxt);
  deallocate_buffer (m_dsm);
  ASESPLAFU::sm_ASEMtx.Unlock();
  return ServiceBase::Release(TranID, timeout);
}

btBool ASESPLAFU::Release(btTime timeout)
{
  ASESPLAFU::sm_ASEMtx.Lock();
  // deallocate_buffer (m_spl_pt);
  // deallocate_buffer (m_spl_cxt);
  deallocate_buffer (m_dsm);
  ASESPLAFU::sm_ASEMtx.Unlock();
  return ServiceBase::Release(timeout);
}

void ASESPLAFU::WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID)
{
   btcString                 descr = NULL;
   buffer_t                  buf;
   std::pair<map_iter, bool> res;

   ::memset(&buf, 0, sizeof(buffer_t));

   buf.memsize = (uint32_t)Length;

   ASESPLAFU::sm_ASEMtx.Lock();
   allocate_buffer(&buf);
   m_AFUCntxt_vbase = (uint64_t*)buf.vbase;
   ASESPLAFU::sm_ASEMtx.Unlock();

   printf("APP-C: Building SPL page table and context\n");
   setup_spl_cxt_pte (m_dsm, &buf);

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

   SendMsg( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                          TranID,
                                                          (btVirtAddr)buf.vbase,
                                                          (btPhysAddr)buf.fake_paddr,
                                                          (btWSSize)buf.memsize) );
   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                            TranID,
                                                                            errAFUWorkSpace,
                                                                            reasAFUNoMemory,
                                                                            descr);
   SendMsg( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                               pExcept) );
}

void ASESPLAFU::WorkspaceFree(btVirtAddr           Address,
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

   ASESPLAFU::sm_ASEMtx.Lock();
   deallocate_buffer(&buf);
   // deallocate_buffer(m_spl_pt);
   // deallocate_buffer(m_spl_cxt);
   ASESPLAFU::sm_ASEMtx.Unlock();

   // Remove the mapping.
   m_WkspcMap.erase(iter);

   Unlock();

   SendMsg( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                      TranID) );
   return;

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                            TranID,
                                                                            errAFUWorkSpace,
                                                                            reasAFUNoMemory,
                                                                            descr);
   SendMsg( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                           pExcept) );
}

btBool ASESPLAFU::CSRRead(btCSROffset CSR,
                          btCSRValue *pValue)
{
   if ( __UINTPTR_T_CONST(0x344) == CSR ) {
      *pValue = m_Last3c4;
   } else if ( __UINTPTR_T_CONST(0x34c) == CSR ) {
      *pValue = m_Last3cc;
   } else {
      ASESPLAFU::sm_ASEMtx.Lock();
      *pValue = (btCSRValue) csr_read(CSR);
      ASESPLAFU::sm_ASEMtx.Unlock();
   }

   return true;
}

btBool ASESPLAFU::CSRWrite(btCSROffset CSR,
                           btCSRValue  Value)
{
   if ( __UINTPTR_T_CONST(0x3c4) == CSR ) {
      m_Last3c4 = Value;
   } else if ( __UINTPTR_T_CONST(0x3cc) == CSR ) {
      m_Last3cc = Value;
   } else {
      ASESPLAFU::sm_ASEMtx.Lock();
      csr_write(CSR, (bt32bitCSR)Value);
      ASESPLAFU::sm_ASEMtx.Unlock();
   }

   return true;
}

btBool ASESPLAFU::CSRWrite64(btCSROffset CSR,
                             bt64bitCSR  Value)
{
   if ( CSRWrite(CSR + 4, Value >> 32) ) {
      return CSRWrite(CSR, Value & 0xffffffff);
   }
   return false;
}

void ASESPLAFU::StartTransactionContext(TransactionID const &TranID,
                                        btVirtAddr           Address,
                                        btTime               Pollrate)
{
// SPL Driver reset
  spl_driver_reset (m_dsm);
  
  // SPL driver SPL setup
  spl_driver_dsm_setup(m_dsm);

  // SPL driver AFU setup
  spl_driver_afu_setup(m_dsm);

  // TODO populate these AFU DSM values
  btVirtAddr AFUDSMVirt = (btVirtAddr)m_dsm->vbase; // RRS: NULL
  btWSSize   AFUDSMSize = (btWSSize)m_dsm->memsize;
  
  if ( NULL == Address ) {
    // The user wants access to the AFU DSM before starting the transaction.
    // We don't actually start the transaction when Address is NULL.
    SendMsg( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
							   TranID,
							   AFUDSMVirt,
							   AFUDSMSize) );
    return;
  }
  
  // TODO start the SPL transaction here, just like the SPL driver does.
  spl_driver_start(m_AFUCntxt_vbase);




  SendMsg( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
							 TranID,
							 AFUDSMVirt,
							 AFUDSMSize) );
}

void ASESPLAFU::StopTransactionContext(TransactionID const &TranID)
{

   // TODO stop the SPL transaction here, just like the SPL driver does.
  spl_driver_stop();

  // Reset the SPL 
  spl_driver_reset(m_dsm);



   SendMsg( new(std::nothrow) SPLClientTransactionStopped(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                          TranID) );
}

void ASESPLAFU::SetContextWorkspace(TransactionID const &TranID,
                                    btVirtAddr           Address,
                                    btTime               Pollrate)
{
   ASSERT(NULL != Address);


   // TODO start the SPL transaction here, just like the SPL driver does.
  ASESPLAFU::sm_ASEMtx.Lock();
  csr_write (SPL_CH_CTRL_OFF, 0x0);
  ASESPLAFU::sm_ASEMtx.Unlock();



   SendMsg( new(std::nothrow) SPLClientContextWorkspaceSet(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                           TranID) );
}

/// @} group ASESPLAFU

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


#define SERVICE_FACTORY AAL::AAS::InProcSvcsFact< ASESPLAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

ASESPLAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
ASESPLAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

