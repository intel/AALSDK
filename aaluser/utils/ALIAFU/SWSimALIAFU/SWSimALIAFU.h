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
/// @file SWSimALIAFU.h
/// @brief Definitions for Software Simulated(NLB) ALI AFU Service.
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
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifndef __SWSIMALIAFU_H__
#define __SWSIMALIAFU_H__
#include <aalsdk/service/ALIAFUService.h>
#include <aalsdk/service/SWSimALIAFUService.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aas/AALService.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup SWSimALIAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IALIAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IALIAFU' used as base for dll-interface class 'AAL::SWSimALIAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with a
/// Software Simulation of ALI (Native Loopback).
///
/// SWSimALIAFU is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_SWSIM)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class SWSIMALIAFU_API SWSimALIAFU : public ServiceBase /*,
                                        public IALIAFU  // FIXME: There is no IALIAFU */
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <ServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SWSimALIAFU, ServiceBase),
      m_NextPhys(0)
   {
//      SetInterface(        iidALIAFU,      dynamic_cast<IALIAFU *>(this));
//      SetInterface(iidSWSIMALIAFU, dynamic_cast<IALIAFU *>(this));
   }

   virtual btBool init( IBase *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);
   // </ServiceBase>

   // <IALIAFU>
   virtual void WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID);

   virtual void     WorkspaceFree(btVirtAddr           Address,
                                  TransactionID const &TranID);

   virtual btBool         CSRRead(btCSROffset Offset,
                                  btCSRValue *pValue);

   virtual btBool        CSRWrite(btCSROffset Offset,
                                  btCSRValue  Value);
   virtual btBool      CSRWrite64(btCSROffset Offset,
                                  bt64bitCSR  Value);
   // </IALIAFU>

protected:
   struct WkspcAlloc
   {
      WkspcAlloc(btVirtAddr v=NULL, btPhysAddr p=__PHYS_ADDR_CONST(0), btWSSize s=0) :
         m_Virt(v),
         m_Phys(p),
         m_Size(s)
      {}

      btVirtAddr Virt() const { return m_Virt; }
      btPhysAddr Phys() const { return m_Phys; }
      btWSSize   Size() const { return m_Size; }

      btVirtAddr m_Virt;
      btPhysAddr m_Phys;
      btWSSize   m_Size;
   };
   friend std::ostream & operator << (std::ostream & , const SWSimALIAFU::WkspcAlloc & );

   typedef std::map<btVirtAddr, WkspcAlloc>  virt_to_alloc_map;
   typedef virt_to_alloc_map::iterator       virt_to_alloc_iter;
   typedef virt_to_alloc_map::const_iterator virt_to_alloc_const_iter;

   typedef std::map<btPhysAddr, WkspcAlloc>  phys_to_alloc_map;
   typedef phys_to_alloc_map::iterator       phys_to_alloc_iter;
   typedef phys_to_alloc_map::const_iterator phys_to_alloc_const_iter;

   struct CSR
   {
      CSR(btCSROffset offset=0, btCSRValue value=0, btBool bReadable=false) :
         m_Offset(offset),
         m_Value(value),
         m_bReadable(bReadable)
      {}

      btCSROffset Offset() const { return m_Offset;    }
      btCSRValue   Value() const { return m_Value;     }
      void   Value(btCSRValue v) { m_Value = v;        }
      btBool    Readable() const { return m_bReadable; }

      btCSROffset m_Offset;
      btCSRValue  m_Value;
      btBool      m_bReadable;
   };
   friend std::ostream & operator << (std::ostream & , const SWSimALIAFU::CSR & );

   typedef std::map<btCSROffset, SWSimALIAFU::CSR> csr_map;
   typedef csr_map::iterator                         csr_iter;
   typedef csr_map::const_iterator                   csr_const_iter;

   btPhysAddr NextPhys();
   void       SimulatorRead(CSR csr);
   void       SimulatorWrite(CSR csr);
   void       SimLpbk1();
   void       SimRead();
   void       SimWrite();
   void       SimTrput();

   btPhysAddr        m_NextPhys;
#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because these members are not accessed outside SWSimALIAFU.
# pragma warning(disable : 4251) // needs to have dll-interface to be used by clients of class 'AAL::SWSimALIAFU'
#endif // __AAL_WINDOWS__
   virt_to_alloc_map m_VirtMap;
   phys_to_alloc_map m_PhysMap;
   csr_map           m_CSRMap;
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
   CSR               m_LastCSRWrite;
   btCSRValue        m_PerfCounters[11];
};

inline std::ostream & operator << (std::ostream &os, const SWSimALIAFU::WkspcAlloc &a)
{
   os << "virt="    << (void *)a.m_Virt <<
         " phys=0x" << std::hex << std::setw(16) << std::setfill('0') << a.m_Phys <<
         " size="   << std::dec << std::setw(0)  << std::setfill(' ') << a.m_Size;
   return os;
}

inline std::ostream & operator << (std::ostream &os, const SWSimALIAFU::CSR &c)
{
   os << "offset=0x" << std::hex << std::setw(8) << std::setfill('0') << c.m_Offset
      << " value=0x" << std::setw(8) << c.m_Value
      << std::dec << std::setw(0) << std::setfill(' ');
   return os;
}

/// @}

END_NAMESPACE(AAL)

#endif // __SWSIMALIAFU_H__

