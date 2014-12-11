// Copyright (c) 2014, Intel Corporation
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
/// @file SWSimCCIAFU.cpp
/// @brief Implementation of Software Simulated(NLB) CCI AFU Service.
/// @ingroup ICCIAFU
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

#include <aalsdk/service/ICCIClient.h>
#include "SWSimCCIAFU.h"

#ifdef INFO
# undef INFO
#endif // INFO
#define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)

BEGIN_NAMESPACE(AAL)

#define CSR_CIPUCTL              0x280
#define CSR_AFU_DSM_BASEL        0x1a00
#define CSR_AFU_DSM_BASEH        0x1a04
#define CSR_SRC_ADDR             0x1a20
#define CSR_DST_ADDR             0x1a24
#define CSR_NUM_LINES            0x1a28
#define CSR_CTL                  0x1a2c
#define CSR_CFG                  0x1a34

#define DSM_STATUS_TEST_COMPLETE 0x40

#ifndef LOG2_CL
# define LOG2_CL 6
#endif // LOG2_CL
#ifndef CL
# define CL(x) ((x) * 64)
#endif // CL

void SWSimCCIAFU::init(TransactionID const &TranID)
{
   m_NextPhys = __PHYS_ADDR_CONST(1) << LOG2_CL;

   m_CSRMap.insert(std::make_pair(CSR_CIPUCTL,       CSR(CSR_CIPUCTL,       0, true)));
   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_BASEL, CSR(CSR_AFU_DSM_BASEL, 0, false)));
   m_CSRMap.insert(std::make_pair(CSR_AFU_DSM_BASEH, CSR(CSR_AFU_DSM_BASEH, 0, false)));
   m_CSRMap.insert(std::make_pair(CSR_SRC_ADDR,      CSR(CSR_SRC_ADDR,      0, false)));
   m_CSRMap.insert(std::make_pair(CSR_DST_ADDR,      CSR(CSR_DST_ADDR,      0, false)));
   m_CSRMap.insert(std::make_pair(CSR_NUM_LINES,     CSR(CSR_NUM_LINES,     0, false)));
   m_CSRMap.insert(std::make_pair(CSR_CTL,           CSR(CSR_CTL,           0, false)));
   m_CSRMap.insert(std::make_pair(CSR_CFG,           CSR(CSR_CFG,           0, false)));

   QueueAASEvent( new(std::nothrow) AAL::AAS::ObjectCreatedEvent(getRuntimeClient(),
                                                                 Client(),
                                                                 dynamic_cast<IBase *>(this),
                                                                 TranID) );
}

btBool SWSimCCIAFU::Release(TransactionID const &TranID, btTime timeout)
{
   return ServiceBase::Release(TranID, timeout);
}

btBool SWSimCCIAFU::Release(btTime timeout)
{
   return ServiceBase::Release(timeout);
}


btPhysAddr SWSimCCIAFU::NextPhys()
{
   btPhysAddr p   = m_NextPhys;
   btPhysAddr tmp = p >> LOG2_CL;
   m_NextPhys = (tmp + 1) << LOG2_CL;
   return p;
}

void SWSimCCIAFU::WorkspaceAllocate(btWSSize             Length,
                                    TransactionID const &TranID)
{
   AutoLock(this);

   btVirtAddr v = (btVirtAddr) new(std::nothrow) btByte[Length];

   if ( NULL == v ) {
      IEvent *pExcept = new(std::nothrow) AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                               TranID,
                                                                               errAFUWorkSpace,
                                                                               reasAFUNoMemory,
                                                                               "new failed");
      SendMsg( new(std::nothrow) CCIClientWorkspaceAllocateFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
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

   SendMsg( new(std::nothrow) CCIClientWorkspaceAllocated(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                          TranID,
                                                          a.Virt(),
                                                          a.Phys(),
                                                          a.Size()) );
}

void SWSimCCIAFU::WorkspaceFree(btVirtAddr           Address,
                                TransactionID const &TranID)
{
   AutoLock(this);

   // Find the memory allocation, given the virtual address.
   virt_to_alloc_iter viter = m_VirtMap.find(Address);

   if ( m_VirtMap.end() == viter ) {
      // No such allocation found.
      IEvent *pExcept = new(std::nothrow) AAL::AAS::CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                                               TranID,
                                                                               errAFUWorkSpace,
                                                                               reasAFUNoMemory,
                                                                               "no such allocation");
      SendMsg( new(std::nothrow) CCIClientWorkspaceFreeFailed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
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

   SendMsg( new(std::nothrow) CCIClientWorkspaceFreed(dynamic_ptr<ICCIClient>(iidCCIClient, ClientBase()),
                                                      TranID) );
}

btBool SWSimCCIAFU::CSRRead(btCSROffset Offset,
                            btCSRValue *pValue)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("Unsupported CSR " << SWSimCCIAFU::CSR(Offset, 0, false));
      return false;
   }

   SWSimCCIAFU::CSR csr = (*iter).second;

   ASSERT(csr.Readable());
   if ( !csr.Readable() ) {
      ERR("CSR " << csr << " is not readable!");
      return false;
   }

   *pValue = csr.Value();

   return true;
}

btBool SWSimCCIAFU::CSRWrite(btCSROffset Offset,
                             btCSRValue  Value)
{
   AutoLock(this);

   csr_iter iter = m_CSRMap.find(Offset);

   ASSERT(m_CSRMap.end() != iter);
   if ( m_CSRMap.end() == iter ) {
      ERR("Unsupported CSR " << SWSimCCIAFU::CSR(Offset, 0, false));
      return false;
   }

   (*iter).second.Value(Value);

   Simulator((*iter).second);
   m_LastCSRWrite = (*iter).second;

   return true;
}

btBool SWSimCCIAFU::CSRWrite64(btCSROffset Offset,
                               bt64bitCSR  Value)
{
   if ( CSRWrite(Offset + 4, Value >> 32) ) {
      return CSRWrite(Offset, Value & 0xffffffff);
   }
   return false;
}

void SWSimCCIAFU::Simulator(CSR CurrWrite)
{
   if ( (CSR_AFU_DSM_BASEH == m_LastCSRWrite.Offset()) &&
        (CSR_AFU_DSM_BASEL == CurrWrite.Offset()) ) {
      // Extract the physical address of the AFU DSM from the CSR's.
      btPhysAddr AFUDSM = (m_LastCSRWrite.Value() << 32) | CurrWrite.Value();

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

   } else if ( CSR_CTL == CurrWrite.Offset() ) {

      if ( flags_are_set(CurrWrite.Value(), 7) ) {
         INFO("Forcibly Stopping");
      } else if ( flags_are_set(CurrWrite.Value(), 3) ) {
         INFO("Starting Copy");

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


         // Write a non-zero value to the 32 bits at AFU DSM offset DSM_STATUS_TEST_COMPLETE.
         btPhysAddr AFUDSM;
         csriter = m_CSRMap.find(CSR_AFU_DSM_BASEH);
         AFUDSM  = (btPhysAddr)((*csriter).second.Value()) << 32;

         csriter = m_CSRMap.find(CSR_AFU_DSM_BASEL);
         AFUDSM |= (btPhysAddr)((*csriter).second.Value());

         physiter = m_PhysMap.find(AFUDSM);

         volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                    ((*physiter).second.Virt()  + DSM_STATUS_TEST_COMPLETE);

         *StatusAddr = 1;

      } else if ( flag_is_clr(m_LastCSRWrite.Value(), 1) &&
                  flag_is_set(CurrWrite.Value(), 1) ) {
         INFO("Device Reset");
      }

   } else if ( CSR_CFG == CurrWrite.Offset() ) {

      if ( 0 != CurrWrite.Value() ) {
         ERR("The simulator supports Native Loopback mode 0 only.");
      }

   }
}

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


#define SERVICE_FACTORY AAL::AAS::InProcSvcsFact< SWSimCCIAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

SWSIMCCIAFU_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
SWSIMCCIAFU_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

