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
/// @file SWSimALIAFU.cpp
/// @brief Implementation of Software Simulated(NLB) ALI AFU Service.
/// @ingroup SWSimALIAFU
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
#include "SWSimALIAFU.h"

#include <aalsdk/utils/NLBVAFU.h>

#ifdef INFO
# undef INFO
#endif // INFO
#if 0
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif
#ifdef ERR
# undef ERR
#endif // ERR
#if 1
# define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define ERR(x)
#endif

BEGIN_NAMESPACE(AAL)

#ifndef LOG2_CL
# define LOG2_CL 6
#endif // LOG2_CL
#ifndef CL
# define CL(x) ((x) * 64)
#endif // CL

btBool SWSimALIAFU::init(IBase               *pclientBase,
                         NamedValueSet const &optArgs,
                         TransactionID const &TranID)
{
   ICCIClient *pClient = dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase());
   ASSERT( NULL != pClient );
   if ( NULL == pClient ) {
      /// ObjectCreatedExceptionEvent Constructor.
      initFailed( new CExceptionTransactionEvent( this,
                                                  TranID,
                                                  errBadParameter,
                                                  reasMissingInterface,
                                                  "Client did not publish ICCIClient Interface") );
      return false;
   }

   btInt i;

   m_NextPhys = __PHYS_ADDR_CONST(1) << LOG2_CL;

   for ( i = 0 ; i < sizeof(m_PerfCounters) / sizeof(m_PerfCounters[0]) ; ++i ) {
      m_PerfCounters[i] = 0;
   }

   m_CSRMap.insert(std::make_pair(QLP_CSR_CIPUCTL,     CSR(QLP_CSR_CIPUCTL,     0, true)));
   m_CSRMap.insert(std::make_pair(QLP_CSR_CAFU_STATUS, CSR(QLP_CSR_CAFU_STATUS, 0, true)));
   m_CSRMap.insert(std::make_pair(QLP_CSR_ADDR_PERF1C, CSR(QLP_CSR_ADDR_PERF1C, 0, false)));
   m_CSRMap.insert(std::make_pair(QLP_CSR_ADDR_PERF1,  CSR(QLP_CSR_ADDR_PERF1,  0, true)));

   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_BASEL,   CSR(CSR_AFU_DSM_BASEL,   0, false)));
   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_BASEH,   CSR(CSR_AFU_DSM_BASEH,   0, false)));
   m_CSRMap.insert(std::make_pair(CSR_SRC_ADDR,        CSR(CSR_SRC_ADDR,        0, false)));
   m_CSRMap.insert(std::make_pair(CSR_DST_ADDR,        CSR(CSR_DST_ADDR,        0, false)));
   m_CSRMap.insert(std::make_pair(CSR_NUM_LINES,       CSR(CSR_NUM_LINES,       0, false)));
   m_CSRMap.insert(std::make_pair(CSR_CTL,             CSR(CSR_CTL,             0, false)));
   m_CSRMap.insert(std::make_pair(CSR_CFG,             CSR(CSR_CFG,             0, false)));

   initComplete(TranID);
   return true;
}

btBool SWSimALIAFU::Release(TransactionID const &TranID, btTime timeout)
{
   return ServiceBase::Release(TranID, timeout);
}


btPhysAddr SWSimALIAFU::NextPhys()
{
   btPhysAddr p   = m_NextPhys;
   btPhysAddr tmp = p >> LOG2_CL;
   m_NextPhys = (tmp + 1) << LOG2_CL;
   return p;
}
/*
void SWSimALIAFU::WorkspaceAllocate(btWSSize             Length,
                                    TransactionID const &TranID)
{
   AutoLock(this);

   btVirtAddr v = (btVirtAddr) new(std::nothrow) btByte[Length];

   if ( NULL == v ) {
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasAFUNoMemory,
                                                                     "new failed");
      getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                                                          pExcept) );
      return;
   }

   btPhysAddr p = NextPhys();
   WkspcAlloc a(v, p, Length);

   // We provide a mechanism to track an allocation by either its virtual or physical address.
   std::pair<virt_to_alloc_iter, bool> vres = m_VirtMap.insert(std::make_pair(v, a));
   ASSERT(vres.second);

   std::pair<phys_to_alloc_iter, bool> pres = m_PhysMap.insert(std::make_pair(p, a));
   ASSERT(pres.second);

   INFO("alloc " << a);

   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                                                  TranID,
                                                                                  a.Virt(),
                                                                                  a.Phys(),
                                                                                  a.Size()) );
}

void SWSimALIAFU::WorkspaceFree(btVirtAddr           Address,
                                TransactionID const &TranID)
{
   AutoLock(this);

   // Find the memory allocation, given the virtual address.
   virt_to_alloc_iter viter = m_VirtMap.find(Address);

   if ( m_VirtMap.end() == viter ) {
      // No such allocation found.
      IEvent *pExcept = new(std::nothrow) CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                     TranID,
                                                                     errAFUWorkSpace,
                                                                     reasAFUNoMemory,
                                                                     "no such allocation");
      getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                                                      pExcept) );
      return;
   }

   INFO("free " << (*viter).second);

   // To remove the physical address mapping.
   btPhysAddr p = (*viter).second.Phys();
   phys_to_alloc_iter piter = m_PhysMap.find(p);

   delete[] (*viter).second.Virt();
   m_VirtMap.erase(viter);

   if ( m_PhysMap.end() != piter ) {
      m_PhysMap.erase(piter);
   }

   getRuntime()->schedDispatchable( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, getServiceClientBase()),
                                                                              TranID) );
}

btBool SWSimALIAFU::CSRRead(btCSROffset Offset,
                            btCSRValue *pValue)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("Unsupported CSR " << SWSimALIAFU::CSR(Offset, 0, false));
      return false;
   }

   SWSimALIAFU::CSR csr = (*iter).second;

   ASSERT(csr.Readable());
   if ( !csr.Readable() ) {
      ERR("CSR " << csr << " is not readable!");
      return false;
   }

   SimulatorRead((*iter).second);
   *pValue = csr.Value();

   return true;
}

btBool SWSimALIAFU::CSRWrite(btCSROffset Offset,
                             btCSRValue  Value)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("Unsupported CSR " << SWSimALIAFU::CSR(Offset, 0, false));
      return false;
   }

   (*iter).second.Value(Value);

   SimulatorWrite((*iter).second);
   m_LastCSRWrite = (*iter).second;

   return true;
}

btBool SWSimALIAFU::CSRWrite64(btCSROffset Offset,
                               bt64bitCSR  Value)
{
   if ( CSRWrite(Offset + 4, Value >> 32) ) {
      return CSRWrite(Offset, Value & 0xffffffff);
   }
   return false;
}

void SWSimALIAFU::SimulatorRead(CSR csr)
{
   if ( QLP_CSR_ADDR_PERF1 == csr.Offset() ) {

      // We need PERF1C in order to determine which counter to retrieve.
      csr_iter iter = m_CSRMap.find(QLP_CSR_ADDR_PERF1C);

      btUnsigned32bitInt i = (*iter).second.Value();
      btInt shift = 0;

      if ( flag_is_set(i, 0x80000000) ) {
         flag_clrf(i, 0x80000000);
         shift = 32;
      }

      switch ( i ) {
         case QLP_PERF_CACHE_RD_HITS : // FALL THROUGH
         case QLP_PERF_CACHE_WR_HITS : // FALL THROUGH
         case QLP_PERF_CACHE_RD_MISS : // FALL THROUGH
         case QLP_PERF_CACHE_WR_MISS : // FALL THROUGH
         case QLP_PERF_EVICTIONS     : // FALL THROUGH
            break;

         default :
            ERR("Invalid QLP performance counter: " << i);
         return;
      }

      const btUnsigned32bitInt CounterValue = (btUnsigned32bitInt) ( m_PerfCounters[i] >> shift );

      //if ( 0 == shift ) {
      //   INFO("QLP perf low [" << i << "] = " << CounterValue);
      //} else {
      //   INFO("QLP perf high[" << i << "] = " << CounterValue);
      //}

      // Set QLP_CSR_ADDR_PERF1 to the performance counter value.
      iter = m_CSRMap.find(QLP_CSR_ADDR_PERF1);

      (*iter).second.Value(CounterValue);
   }
}

void SWSimALIAFU::SimulatorWrite(CSR csr)
{
   if ( (QLP_CSR_CIPUCTL == csr.Offset()) ) {
      csr_iter csriter = m_CSRMap.find(QLP_CSR_CAFU_STATUS);

      if ( flag_is_set(csr.Value(), CIPUCTL_RESET_BIT) ) {
         // Start of the CAFU Reset sequence.
         // Software will next poll on the 'Status Ready' bit in CAFU_STATUS.

         (*csriter).second.Value((*csriter).second.Value() | CAFU_STATUS_READY_BIT);
      } else {
         // End of the CAFU Reset sequence. We need to reset the 'Status Ready' bit to zero.

         (*csriter).second.Value((*csriter).second.Value() & ~CAFU_STATUS_READY_BIT);
      }

   } else if ( (CSR_AFU_DSM_BASEH == m_LastCSRWrite.Offset()) &&
               (CSR_AFU_DSM_BASEL == csr.Offset()) ) {
      // Extract the physical address of the AFU DSM from the CSR's.
      btPhysAddr AFUDSM = (m_LastCSRWrite.Value() << 32) | csr.Value();

      // Look up the virtual address of the AFU DSM by it's physical address.
      phys_to_alloc_const_iter iter = m_PhysMap.find(AFUDSM);

      ASSERT(m_PhysMap.end() != iter);
      if ( m_PhysMap.end() == iter ) {
         ERR("Invalid AFU DSM Value in CSR_AFU_DSM_BASEH / CSR_AFU_DSM_BASEL: " <<
             std::hex << std::setw(16) << std::setfill('0') << AFUDSM <<
             std::dec << std::setw(0)  << std::setfill(' '));
         return;
      }

      WkspcAlloc a = (*iter).second;

      // Write the AFU ID into the AFU DSM.
      // c000c9660d8242729aeffe5f84570612

      btUnsigned32bitInt *pu32 = (btUnsigned32bitInt *)a.Virt();
      *pu32++ = 0x84570612;
      *pu32++ = 0x9aeffe5f;
      *pu32++ = 0x0d824272;
      *pu32++ = 0xc000c966;

   } else if ( CSR_CTL == csr.Offset() ) {

      // Examine CSR_CFG to determine the requested test mode.
      csr_const_iter csriter = m_CSRMap.find(CSR_CFG);

      btCSRValue val = (*csriter).second.Value();
      btBool IsContMode = false;

      if ( flag_is_set(val, NLB_TEST_MODE_CONT) ) {
         flag_clrf(val, NLB_TEST_MODE_CONT);
         IsContMode = true;
      }

      btPhysAddr AFUDSMPhys;
      csriter     = m_CSRMap.find(CSR_AFU_DSM_BASEH);
      AFUDSMPhys  = (btPhysAddr)((*csriter).second.Value()) << 32;

      csriter     = m_CSRMap.find(CSR_AFU_DSM_BASEL);
      AFUDSMPhys |= (btPhysAddr)((*csriter).second.Value());

      phys_to_alloc_const_iter physiter = m_PhysMap.find(AFUDSMPhys);

      nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)(*physiter).second.Virt();

      if ( flags_are_set(csr.Value(), 7) ) {
         INFO("Forcibly Stopping");

         if ( IsContMode ) {
            // Signal completion for the continuous mode test.
            pAFUDSM->test_complete = 1;
         }

      } else if ( flags_are_set(csr.Value(), 3) ) {

         switch ( val & NLB_TEST_MODE_MASK ) {
            case NLB_TEST_MODE_LPBK1 : SimLpbk1(); break;
            case NLB_TEST_MODE_READ  : SimRead();  break;
            case NLB_TEST_MODE_WRITE : SimWrite(); break;
            case NLB_TEST_MODE_TRPUT : SimTrput(); break;

            default :
               ERR("Unsupported test mode: 0x"
                      << std::hex << ((*csriter).second.Value() & NLB_TEST_MODE_MASK) << std::dec);
            break;
         }

         if ( !IsContMode ) {
            // Signal completion for the non-continuous mode test.
            pAFUDSM->test_complete = 1;
         }

      } else if ( flag_is_clr(m_LastCSRWrite.Value(), 1) &&
                  flag_is_set(csr.Value(), 1) ) {
         INFO("Device Reset");
      }

   } else if ( CSR_CFG == csr.Offset() ) {

      switch ( csr.Value() & NLB_TEST_MODE_MASK ) {
         case NLB_TEST_MODE_LPBK1 : // FALL THROUGH
         case NLB_TEST_MODE_READ  : // FALL THROUGH
         case NLB_TEST_MODE_WRITE : // FALL THROUGH
         case NLB_TEST_MODE_TRPUT : // FALL THROUGH
         break;

         default :
           ERR("Unsupported test mode: 0x"
                  << std::hex << (csr.Value() & NLB_TEST_MODE_MASK) << std::dec);
         break;
      }

   }
}

void SWSimALIAFU::SimLpbk1()
{
   INFO("LPBK1");

   // Determine the Source buffer address from CSR_SRC_ADDR
   // and use it to look up the virtual address.
   csr_const_iter csriter = m_CSRMap.find(CSR_SRC_ADDR);

   btPhysAddr SrcPhys = (*csriter).second.Value() << LOG2_CL;

   phys_to_alloc_const_iter physiter = m_PhysMap.find(SrcPhys);

   btVirtAddr SrcVirt = (*physiter).second.Virt();

   INFO("   src " << (*physiter).second);

   // Likewise, determine the Destination buffer address from CSR_DST_ADDR
   // and use it to look up the virtual address.
   csriter = m_CSRMap.find(CSR_DST_ADDR);
   btPhysAddr DstPhys = (*csriter).second.Value() << LOG2_CL;
   physiter = m_PhysMap.find(DstPhys);
   btVirtAddr DstVirt = (*physiter).second.Virt();

   INFO("   dst " << (*physiter).second);

   // Determine the number of bytes to copy from CSR_NUM_LINES.
   csriter = m_CSRMap.find(CSR_NUM_LINES);

   size_t bytes = (size_t)((*csriter).second.Value() * CL(1));

   // Do the memory copy.
   ::memcpy(DstVirt, SrcVirt, bytes);

   // Update the performance counters. Assume cache misses.
   m_PerfCounters[QLP_PERF_CACHE_RD_MISS] += bytes >> LOG2_CL;
   m_PerfCounters[QLP_PERF_CACHE_WR_MISS] += bytes >> LOG2_CL;
}

void SWSimALIAFU::SimRead()
{
   INFO("READ");

   // Determine the Source buffer address from CSR_SRC_ADDR
   // and use it to look up the virtual address.
   csr_const_iter csriter = m_CSRMap.find(CSR_SRC_ADDR);

   btPhysAddr SrcPhys = (*csriter).second.Value() << LOG2_CL;

   phys_to_alloc_const_iter physiter = m_PhysMap.find(SrcPhys);

   btVirtAddr SrcVirt = (*physiter).second.Virt();

   INFO("   src " << (*physiter).second);

   // Determine the workspace size.
   csriter = m_CSRMap.find(CSR_NUM_LINES);

   size_t bytes = (size_t)((*csriter).second.Value() * CL(1));


   btUnsigned32bitInt *pSrc = (btUnsigned32bitInt *)SrcVirt;
   const btUnsigned32bitInt *pSrcEnd = (const btUnsigned32bitInt *)pSrc +
                                          (bytes / sizeof(btUnsigned32bitInt));
   btUnsigned32bitInt sum = 0;

   for ( ; pSrc < pSrcEnd ; ++pSrc ) {
      sum += *pSrc;
   }

   // Update the performance counters. Assume cache misses.
   m_PerfCounters[QLP_PERF_CACHE_RD_MISS] += bytes >> LOG2_CL;

   // Update the DSM.

   // Extract the physical address of the AFU DSM from the CSR's.
   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEH);

   btPhysAddr AFUDSMPhys = (*csriter).second.Value() << 32;

   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEL);

   AFUDSMPhys |= (*csriter).second.Value();

   physiter = m_PhysMap.find(AFUDSMPhys);

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)(*physiter).second.Virt();

   // (No idea whether these numbers are even in the same universe, let alone ballpark, as
   //  the accurate measurements. We are simulating after all..)
   pAFUDSM->num_clocks     = 5 * (bytes / CL(1));
   pAFUDSM->num_reads      = bytes / CL(1);
   pAFUDSM->start_overhead = 2;
   pAFUDSM->end_overhead   = 1;
}

void SWSimALIAFU::SimWrite()
{
   INFO("WRITE");

   // Determine the Destination buffer address from CSR_DST_ADDR
   // and use it to look up the virtual address.
   csr_const_iter csriter = m_CSRMap.find(CSR_DST_ADDR);
   btPhysAddr DstPhys = (*csriter).second.Value() << LOG2_CL;

   phys_to_alloc_const_iter physiter = m_PhysMap.find(DstPhys);
   btVirtAddr DstVirt = (*physiter).second.Virt();

   INFO("   dst " << (*physiter).second);

   // Determine the workspace size.
   csriter = m_CSRMap.find(CSR_NUM_LINES);

   size_t bytes = (size_t)((*csriter).second.Value() * CL(1));


   btUnsigned32bitInt *pDst = (btUnsigned32bitInt *)DstVirt;
   const btUnsigned32bitInt *pDstEnd = (const btUnsigned32bitInt *)pDst +
                                          (bytes / sizeof(btUnsigned32bitInt));

   btUnsigned32bitInt data = 0xd0000000;

   for ( ; pDst < pDstEnd ; ++pDst ) {
      *pDst = data++;
   }

   // Update the performance counters. Assume cache misses.
   m_PerfCounters[QLP_PERF_CACHE_WR_MISS] += bytes >> LOG2_CL;

   // Update the DSM.

   // Extract the physical address of the AFU DSM from the CSR's.
   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEH);

   btPhysAddr AFUDSMPhys = (*csriter).second.Value() << 32;

   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEL);

   AFUDSMPhys |= (*csriter).second.Value();

   physiter = m_PhysMap.find(AFUDSMPhys);

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)(*physiter).second.Virt();

   // (No idea whether these numbers are even in the same universe, let alone ballpark, as
   //  the accurate measurements. We are simulating after all..)

   pAFUDSM->num_clocks     = 5 * (bytes / CL(1));
   pAFUDSM->num_writes     = bytes / CL(1);
   pAFUDSM->start_overhead = 2;
   pAFUDSM->end_overhead   = 1;
}

void SWSimALIAFU::SimTrput()
{
   INFO("TRPUT");

   // Determine the Source buffer address from CSR_SRC_ADDR
   // and use it to look up the virtual address.
   csr_const_iter csriter = m_CSRMap.find(CSR_SRC_ADDR);

   btPhysAddr SrcPhys = (*csriter).second.Value() << LOG2_CL;

   phys_to_alloc_const_iter physiter = m_PhysMap.find(SrcPhys);

   btVirtAddr SrcVirt = (*physiter).second.Virt();

   INFO("   src " << (*physiter).second);

   // Likewise, determine the Destination buffer address from CSR_DST_ADDR
   // and use it to look up the virtual address.
   csriter = m_CSRMap.find(CSR_DST_ADDR);
   btPhysAddr DstPhys = (*csriter).second.Value() << LOG2_CL;
   physiter = m_PhysMap.find(DstPhys);
   btVirtAddr DstVirt = (*physiter).second.Virt();

   INFO("   dst " << (*physiter).second);

   // Determine the size of the buffers using CSR_NUM_LINES.
   csriter = m_CSRMap.find(CSR_NUM_LINES);

   size_t bytes = (size_t)((*csriter).second.Value() * CL(1));

   btUnsigned32bitInt *pSrc = (btUnsigned32bitInt *)SrcVirt;
   const btUnsigned32bitInt *pSrcEnd = (const btUnsigned32bitInt *)pSrc +
                                          (bytes / sizeof(btUnsigned32bitInt));
   btUnsigned32bitInt sum = 0;

   btUnsigned32bitInt *pDst = (btUnsigned32bitInt *)DstVirt;
   const btUnsigned32bitInt *pDstEnd = (const btUnsigned32bitInt *)pDst +
                                          (bytes / sizeof(btUnsigned32bitInt));

   btUnsigned32bitInt data = 0xd0000000;

   for ( ; pSrc < pSrcEnd ; ++pSrc ) {
      sum += *pSrc;
      *pDst = data++;
   }

   // Update the performance counters. Assume cache misses.
   m_PerfCounters[QLP_PERF_CACHE_RD_MISS] += bytes >> LOG2_CL;
   m_PerfCounters[QLP_PERF_CACHE_WR_MISS] += bytes >> LOG2_CL;

   // Update the DSM.

   // Extract the physical address of the AFU DSM from the CSR's.
   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEH);

   btPhysAddr AFUDSMPhys = (*csriter).second.Value() << 32;

   csriter = m_CSRMap.find(CSR_AFU_DSM_BASEL);

   AFUDSMPhys |= (*csriter).second.Value();

   physiter = m_PhysMap.find(AFUDSMPhys);

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)(*physiter).second.Virt();

   // (No idea whether these numbers are even in the same universe, let alone ballpark, as
   //  the accurate measurements. We are simulating after all..)
   pAFUDSM->num_clocks     = 5 * (bytes / CL(1));
   pAFUDSM->num_reads      = bytes / CL(1);
   pAFUDSM->num_writes     = bytes / CL(1);
   pAFUDSM->start_overhead = 2;
   pAFUDSM->end_overhead   = 1;
}
*/
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


#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::SWSimALIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

SWSIMALIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
SWSIMALIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

