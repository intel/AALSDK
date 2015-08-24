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
/// @file SWSimSPLAFU.cpp
/// @brief Implementation of SPL Software Simulated(VAFU) Service.
/// @ingroup SWSimSPLAFU
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
/// 08/01/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AAL.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aas/AALInProcServiceFactory.h>
#include <aalsdk/service/ISPLClient.h>
#include <aalsdk/kernel/vafu2defs.h>

#include "SWSimSPLAFU.h"

#ifdef INFO
# undef INFO
#endif // INFO
#define INFO(x) AAL_INFO(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) AAL_ERR(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)

BEGIN_NAMESPACE(AAL)

/// @addtogroup SWSimSPLAFU
/// @{

#define SPL_CCI_AFU_ID      0x11100101
#define SPL_VAFU2_AFU_ID    0x11100181

#define CSR_AFU_DSM_SCRATCH 0x02bf

#define CSR_SPL_DSM_BASE    0x1000
#define CSR_SPL2_CNTXT_BASE 0x1008
#define CSR_SPL2_CH_CTRL    0x1010

#define CSR_AFU_DSM_BASE    0x8a00
#define CSR_AFU_CNTXT_BASE  0x8a08

#ifndef LOG2_CL
# define LOG2_CL 6
#endif // LOG2_CL
#ifndef CL
# define CL(x) ((x) * 64)
#endif // CL

void SWSimSPLAFU::init(TransactionID const &TranID)
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

   m_NextPhys = __PHYS_ADDR_CONST(1) << LOG2_CL;

   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_SCRATCH, CSR(CSR_AFU_DSM_SCRATCH, 0, false)));
   m_CSRMap.insert(std::make_pair(CSR_SPL_DSM_BASE,    CSR(CSR_SPL_DSM_BASE,    0, false)));
   m_CSRMap.insert(std::make_pair(CSR_SPL2_CNTXT_BASE, CSR(CSR_SPL2_CNTXT_BASE, 0, false)));
   m_CSRMap.insert(std::make_pair(CSR_SPL2_CH_CTRL,    CSR(CSR_SPL2_CH_CTRL,    0, false)));
   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_BASE,    CSR(CSR_AFU_DSM_BASE,    0, false)));
   m_CSRMap.insert(std::make_pair(CSR_AFU_CNTXT_BASE,  CSR(CSR_AFU_CNTXT_BASE,  0, false)));

   btUnsigned64bitInt ExceptionNumber  = errCreationFailure;
   btUnsigned64bitInt Reason           = reasResourcesNotAvailable;
   btcString          Description      = strNoResourceDescr;

   // Allocate the SPL DSM (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   if ( !InternalWkspcAlloc(sizeof(struct SPL2_DSM), m_SPLDSM) ) {
      goto INIT_FAILED;
   }

   INFO("SPL DSM -> " << m_SPLDSM);

   // Allocate the SPL Context (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   if ( !InternalWkspcAlloc(sizeof(struct SPL2_CNTXT), m_SPLContext) ) {
      goto INIT_FAILED;
   }

   INFO("SPL Context -> " << m_SPLContext);

   // Allocate the AFU DSM (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   if ( !InternalWkspcAlloc(sizeof(struct VAFU2_DSM) , m_AFUDSM) ) {
      goto INIT_FAILED;
   }

   INFO("AFU DSM -> " << m_AFUDSM);

   // Issue SPL reset (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   Driver_SPLReset();

   // Set the SPL DSM (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   Driver_SetSPLDSM();

   // Set the AFU DSM (aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_device_init())
   Driver_SetAFUDSM();

   getRuntime()->schedDispatchable( new(std::nothrow) ObjectCreatedEvent(getRuntimeClient(),
                                                                         Client(),
                                                                         dynamic_cast<IBase *>(this),
                                                                         TranID) );
   return;

INIT_FAILED:
   getRuntime()->schedDispatchable( new(std::nothrow) ObjectCreatedExceptionEvent(getRuntimeClient(),
                                                                                  Client(),
                                                                                  dynamic_cast<IBase *>(this),
                                                                                  TranID,
                                                                                  ExceptionNumber,
                                                                                  Reason,
                                                                                  Description) );
   if ( NULL != m_SPLDSM.Virt() ) {
      InternalWkspcFree(m_SPLDSM.Virt(), m_SPLDSM);
      m_SPLDSM = WkspcAlloc();
   }

   if ( NULL != m_SPLContext.Virt() ) {
      InternalWkspcFree(m_SPLContext.Virt(), m_SPLContext);
      m_SPLContext = WkspcAlloc();
   }

   if ( NULL != m_AFUDSM.Virt() ) {
      InternalWkspcFree(m_AFUDSM.Virt(), m_AFUDSM);
      m_AFUDSM = WkspcAlloc();
   }
}

btBool SWSimSPLAFU::Release(TransactionID const &TranID, btTime timeout)
{
   if ( NULL != m_SPLDSM.Virt() ) {
      InternalWkspcFree(m_SPLDSM.Virt(), m_SPLDSM);
      m_SPLDSM = WkspcAlloc();
   }

   if ( NULL != m_SPLContext.Virt() ) {
      InternalWkspcFree(m_SPLContext.Virt(), m_SPLContext);
      m_SPLContext = WkspcAlloc();
   }

   if ( NULL != m_AFUDSM.Virt() ) {
      InternalWkspcFree(m_AFUDSM.Virt(), m_AFUDSM);
      m_AFUDSM = WkspcAlloc();
   }
   return ServiceBase::Release(TranID, timeout);
}

btBool SWSimSPLAFU::Release(btTime timeout)
{
   if ( NULL != m_SPLDSM.Virt() ) {
      InternalWkspcFree(m_SPLDSM.Virt(), m_SPLDSM);
      m_SPLDSM = WkspcAlloc();
   }

   if ( NULL != m_SPLContext.Virt() ) {
      InternalWkspcFree(m_SPLContext.Virt(), m_SPLContext);
      m_SPLContext = WkspcAlloc();
   }

   if ( NULL != m_AFUDSM.Virt() ) {
      InternalWkspcFree(m_AFUDSM.Virt(), m_AFUDSM);
      m_AFUDSM = WkspcAlloc();
   }

   return ServiceBase::Release(timeout);
}

btPhysAddr SWSimSPLAFU::NextPhys()
{
   btPhysAddr p   = m_NextPhys;
   btPhysAddr tmp = p >> LOG2_CL;
   m_NextPhys = (tmp + 1) << LOG2_CL;
   return p;
}

btBool SWSimSPLAFU::InternalWkspcAlloc(btWSSize Length, SWSimSPLAFU::WkspcAlloc &Alloc)
{
   btVirtAddr v = (btVirtAddr) new(std::nothrow) btByte[Length];

   if ( NULL == v ) {
      return false;
   }

   btPhysAddr p = NextPhys();

   Alloc = WkspcAlloc(v, p, Length);

   // We provide a mechanism to track an allocation by either its virtual or physical address.
   std::pair<virt_to_alloc_iter, bool> vres = m_VirtMap.insert(std::make_pair(v, Alloc));
   ASSERT(vres.second);

   std::pair<phys_to_alloc_iter, bool> pres = m_PhysMap.insert(std::make_pair(p, Alloc));
   ASSERT(pres.second);

   return true;
}

void SWSimSPLAFU::WorkspaceAllocate(btWSSize             Length,
                                    TransactionID const &TranID)
{
   AutoLock(this);

   WkspcAlloc a;

   if ( !InternalWkspcAlloc(Length, a) ) {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasAFUNoMemory,
                                                                     "InternalWkspcAlloc failed");
      getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                          pExcept) );
      return;
   }

   INFO("alloc " << a);

   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                  TranID,
                                                                                  a.Virt(),
                                                                                  a.Phys(),
                                                                                  a.Size()) );
}

btBool SWSimSPLAFU::InternalWkspcFree(btVirtAddr Virt, SWSimSPLAFU::WkspcAlloc &Alloc)
{
   // Find the memory allocation, given the virtual address.
   virt_to_alloc_iter viter = m_VirtMap.find(Virt);

   if ( m_VirtMap.end() == viter ) {
      // No such allocation found.
      return false;
   }

   // To remove the physical address mapping.
   btPhysAddr p = (*viter).second.Phys();
   phys_to_alloc_iter piter = m_PhysMap.find(p);

   Alloc = WkspcAlloc(Virt, p, (*viter).second.Size());

   // Delete the memory
   delete[] (*viter).second.Virt();

   // Remove the mapping from the virtual address map.
   m_VirtMap.erase(viter);

   // Remove the mapping from the physical address map.
   if ( m_PhysMap.end() != piter ) {
      m_PhysMap.erase(piter);
   }

   return true;
}

void SWSimSPLAFU::WorkspaceFree(btVirtAddr           Address,
                                TransactionID const &TranID)
{
   AutoLock(this);

   WkspcAlloc a;

   if ( !InternalWkspcFree(Address, a) ) {
      // No such allocation found.
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasAFUNoMemory,
                                                                     "no such allocation");
      getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                      pExcept) );
      return;
   }

   INFO("free " << a);

   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                              TranID) );
}

btBool SWSimSPLAFU::CSRRead(btCSROffset Offset,
                            btCSRValue *pValue)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("unsupported CSR " << SWSimSPLAFU::CSR(Offset, 0, false));
      return false;
   }

   SWSimSPLAFU::CSR csr = (*iter).second;

   ASSERT(csr.Readable());
   if ( !csr.Readable() ) {
      ERR("CSR " << csr << " is not readable!");
      return false;
   }

   *pValue = csr.Value();

   return true;
}

btBool SWSimSPLAFU::CSRWrite(btCSROffset Offset,
                             btCSRValue  Value)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("unsupported CSR " << SWSimSPLAFU::CSR(Offset, 0, false));
      return false;
   }

   (*iter).second.Value(Value);

   Simulator((*iter).second, Value);

   return true;
}

btBool SWSimSPLAFU::CSRWrite64(btCSROffset Offset,
                               bt64bitCSR  Value)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("unsupported CSR " << SWSimSPLAFU::CSR(Offset, 0, false));
      return false;
   }

   (*iter).second.Value(Value);

   Simulator64((*iter).second, Value);

   return true;
}

void SWSimSPLAFU::StartTransactionContext(TransactionID const &TranID,
                                          btVirtAddr           Address,
                                          btTime               Pollrate)
{
   // aalkernel/spl2_pip_dksm/spl2_afu_pip.c:start_spl2_transaction()

   // aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_trans_setup()
   Driver_SPLReset();

   Driver_SetSPLDSM();

   Driver_SetAFUDSM();

   if ( NULL == Address ) {
      // The user wants access to the AFU DSM before starting the transaction.
      // We don't actually start the transaction when Address is NULL.
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                     TranID,
                                                                                     m_AFUDSM.Virt(),
                                                                                     m_AFUDSM.Size()) );
      return;
   }

   if ( Driver_TransStart(TranID, Address, Pollrate) ) {
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStarted(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                     TranID,
                                                                                     m_AFUDSM.Virt(),
                                                                                     m_AFUDSM.Size()) );
   } else {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasParameterValueInvalid,
                                                                     "Invalid AFU context workspace");
      getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                    pExcept) );
   }
}

void SWSimSPLAFU::StopTransactionContext(TransactionID const &TranID)
{
   // Disable the SPL channel
   CSRWrite(CSR_SPL2_CH_CTRL, 0);

   // Reset SPL
   Driver_SPLReset();

   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionStopped(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                  TranID) );
}

void SWSimSPLAFU::SetContextWorkspace(TransactionID const &TranID,
                                      btVirtAddr           Address,
                                      btTime               Pollrate)
{
   btID      exNum   = errAFUWorkSpace;
   btID      exReas  = reasParameterValueInvalid;
   btcString exDescr = "Invalid AFU context workspace";

   ASSERT(NULL != Address);
   if ( NULL == Address ) {
      goto _SEND_ERR;
   }

   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientContextWorkspaceSet(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                   TranID) );

   if ( Driver_TransStart(TranID, Address, Pollrate) ) {
      return; // success
   }

_SEND_ERR:
   IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                  TranID,
                                                                  exNum,
                                                                  exReas,
                                                                  exDescr);
   getRuntime()->schedDispatchable( new(std::nothrow) SPLClientTransactionFailed(dynamic_ptr<ISPLClient>(iidSPLClient, ClientBase()),
                                                                                 pExcept) );
}

void SWSimSPLAFU::Simulator(CSR CurrWrite, btCSRValue /*unused*/)
{
   if ( CSR_SPL2_CH_CTRL == CurrWrite.Offset() ) {

      if ( flag_is_set(CurrWrite.Value(), 1) ) {
         INFO("SPL Device Reset");
      }

      if ( flag_is_set(CurrWrite.Value(), 2) ) {
         INFO("SPL Channel Enabled");

         // Get the virtual address of the AFU Context from the CSR.
         csr_const_iter csriter = m_CSRMap.find(CSR_AFU_CNTXT_BASE);
         struct VAFU2_CNTXT *pAFUCtx = (struct VAFU2_CNTXT *)(*csriter).second.Value();

         // Do the memory copy.
         ::memcpy(pAFUCtx->pDest, pAFUCtx->pSource, (size_t)(pAFUCtx->num_cl * CL(1)));

         // Determine the virtual address of the AFU DSM by using the physical address from
         // the CSR.
         csriter = m_CSRMap.find(CSR_AFU_DSM_BASE);

         btPhysAddr AFUDSMPhys = (btPhysAddr)(*csriter).second.Value();
         phys_to_alloc_const_iter physiter = m_PhysMap.find(AFUDSMPhys);

         struct VAFU2_DSM *pAFUDSM = (struct VAFU2_DSM *)(*physiter).second.Virt();

         // Set the performance counters in the AFU DSM.
         pAFUDSM->AFU_DSM_LATENCY     = 0x000000c0000000ca;
         pAFUDSM->AFU_DSM_PERFORMANCE = 0x000000c00000001a;

         // Set the "done" status in the AFU Context.
         pAFUCtx->Status = VAFU2_CNTXT_STATUS_DONE;

      } else if ( flag_is_set(m_LastCHCtrlWrite.Value(), 2) ) {
         INFO("SPL Channel Disabled");
      }

      m_LastCHCtrlWrite = CurrWrite;
   } else if ( CSR_AFU_DSM_SCRATCH == CurrWrite.Offset() ) {
      // Determine the virtual address of the AFU DSM by using the physical address from
      // the CSR.
      csr_const_iter csriter = m_CSRMap.find(CSR_AFU_DSM_BASE);

      btPhysAddr AFUDSMPhys = (btPhysAddr)(*csriter).second.Value();
      phys_to_alloc_const_iter physiter = m_PhysMap.find(AFUDSMPhys);

      struct VAFU2_DSM *pAFUDSM = (struct VAFU2_DSM *)(*physiter).second.Virt();

      // The VAFU sets offset AFU_DSM_SCRATCH within the AFU DSM in response to
      // CSR writes to CSR_AFU_DSM_SCRATCH.
      pAFUDSM->AFU_DSM_SCRATCH = CurrWrite.Value();
   }

}

void SWSimSPLAFU::Simulator64(CSR CurrWrite, bt64bitCSR /*unused*/)
{
   btPhysAddr p = __PHYS_ADDR_CONST(0);

   if ( CSR_SPL_DSM_BASE == CurrWrite.Offset() ) {
      // The SPL DSM is being set. The value of the current CSR write is the physical address.
      p = (btPhysAddr)CurrWrite.Value();

      // Determine the virtual addr for the given physical addr.
      phys_to_alloc_const_iter iter = m_PhysMap.find(p);

      if ( m_PhysMap.end() == iter ) {
         ERR("Invalid SPL DSM Value in CSR_SPL_DSM_BASE: 0x" <<
             std::hex << std::setw(16) << std::setfill('0') << p <<
             std::dec << std::setw(0)  << std::setfill(' '));
         return;
      }

      struct CCIAFU_DSM *pSPLDSM = (struct CCIAFU_DSM *)(*iter).second.Virt();
      pSPLDSM->cci_afu_id = SPL_CCI_AFU_ID;

      INFO("Set SPL DSM");
   } else if ( CSR_AFU_DSM_BASE == CurrWrite.Offset() ) {
      // The AFU DSM is being set. The value of the current CSR write is the physical address.
      p = (btPhysAddr)CurrWrite.Value();

      // Determine the virtual addr for the given physical addr.
      phys_to_alloc_const_iter iter = m_PhysMap.find(p);

      if ( m_PhysMap.end() == iter ) {
         ERR("Invalid AFU DSM Value in CSR_AFU_DSM_BASE: 0x" <<
             std::hex << std::setw(16) << std::setfill('0') << p <<
             std::dec << std::setw(0)  << std::setfill(' '));
         return;
      }

      struct VAFU2_DSM *pAFUDSM = (struct VAFU2_DSM *)(*iter).second.Virt();
      pAFUDSM->vafu2.AFU_ID[0] = SPL_VAFU2_AFU_ID;
      pAFUDSM->vafu2.AFU_ID[1] = 0;

      INFO("Set AFU DSM");
   } else if ( CSR_SPL2_CNTXT_BASE == CurrWrite.Offset() ) {
      INFO("Set SPL Context");
   } else if ( CSR_AFU_CNTXT_BASE == CurrWrite.Offset() ) {
      INFO("Set AFU Context");
   }
}

btBool SWSimSPLAFU::Driver_SPLReset()
{
   return CSRWrite(CSR_SPL2_CH_CTRL, 1);
}

btBool SWSimSPLAFU::Driver_SetSPLDSM()
{
   volatile struct CCIAFU_DSM *pSPLDSM = (volatile struct CCIAFU_DSM *)m_SPLDSM.Virt();
   ::memset((void *)pSPLDSM, 0, sizeof(struct CCIAFU_DSM));

   btBool res = CSRWrite64(CSR_SPL_DSM_BASE, m_SPLDSM.Phys());

      // Poll for the AFU ID
   while ( 0 == pSPLDSM->cci_afu_id ) {
      ; // wait
   }

   INFO("Set the SPL DSM, then saw CCI ID = 0x" <<
        std::hex << std::setw(8) << std::setfill('0') << pSPLDSM->cci_afu_id <<
        std::dec << std::setw(0) << std::setfill(' '));

   return res;
}

btBool SWSimSPLAFU::Driver_SetAFUDSM()
{
   volatile struct VAFU2_DSM  *pAFUDSM = (volatile struct VAFU2_DSM *)m_AFUDSM.Virt();
   ::memset((void *)pAFUDSM, 0, sizeof(struct VAFU2_DSM));

   btBool res = CSRWrite64(CSR_AFU_DSM_BASE, m_AFUDSM.Phys());

   // Poll for the SPL VAFU ID
   while ( 0 == pAFUDSM->vafu2.AFU_ID[0] ) {
      ; // wait
   }

   INFO("Set the AFU DSM, then saw AFU ID = 0x" <<
        std::hex << std::setw(8) << std::setfill('0') << pAFUDSM->vafu2.AFU_ID[0] <<
        std::dec << std::setw(0) << std::setfill(' '));

   return res;
}

btBool SWSimSPLAFU::Driver_TransStart(TransactionID const &TranID, btVirtAddr AFUCtx, btTime /*unused*/)
{
   // aalkernel/spl2_pip_dksm/spl2_primitives.c:spl2_trans_start()
   struct SPL2_CNTXT *pSPLCtx = (struct SPL2_CNTXT *)m_SPLContext.Virt();

   ::memset(pSPLCtx, 0, sizeof(struct SPL2_CNTXT));

   virt_to_alloc_const_iter viter = m_VirtMap.find(AFUCtx);

   if ( m_VirtMap.end() == viter ) {
      ERR("Invalid AFU Context Workspace: " << (void *)AFUCtx);
      return false;
   }

   pSPLCtx->phys_addr_page_table  = (btUnsigned64bitInt)(*viter).second.Phys();
   pSPLCtx->virt_addr_afu_context = (btUnsigned64bitInt)AFUCtx;
   pSPLCtx->page_size             = 1; // 1 = 2MB, only supported value currently
   //pSPLCtx->num_valid_ptes        = TODO
   pSPLCtx->control_flags         = 1; // 1 = virtual

   // Set the SPL Context
   CSRWrite64(CSR_SPL2_CNTXT_BASE, m_SPLContext.Phys());

   // Set the AFU Context
   CSRWrite64(CSR_AFU_CNTXT_BASE, (btUnsigned64bitInt)AFUCtx);

   // Enable the SPL channel
   CSRWrite(CSR_SPL2_CH_CTRL, 2);

   return true;
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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::SWSimSPLAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

SWSIMSPLAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
SWSIMSPLAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

