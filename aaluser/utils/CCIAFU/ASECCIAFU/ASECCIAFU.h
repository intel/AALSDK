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
/// @file ASECCIAFU.h
/// @brief Definitions for ASE CCI AFU Service.
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
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/31/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __ASECCIAFU_H__
#define __ASECCIAFU_H__
#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/ase/ase_common.h>

#include <aalsdk/service/CCIAFUService.h>
#include <aalsdk/service/ASECCIAFUService.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup ASECCIAFU
/// @{

/// @brief This is the Delegate of the Strategy pattern used by CCIAFU to interact with the
/// AFU Simulation Environment.
///
/// ASECCIAFU is selected by passing the Named Value pair (CCIAFU_NVS_KEY_TARGET, CCIAFU_NVS_VAL_TARGET_ASE)
/// in the arguments to IRuntime::allocService when requesting a CCIAFU.
class ASECCIAFU_API ASECCIAFU : public ServiceBase,
                                public ICCIAFU
{
public:
   // <ServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(ASECCIAFU, ServiceBase),
      m_Last3c4(0xffffffff),
      m_Last3cc(0xffffffff)
   {
      SetInterface(        iidCCIAFU,    dynamic_cast<ICCIAFU *>(this));
      SetSubClassInterface(iidASECCIAFU, dynamic_cast<ICCIAFU *>(this));
   }

   virtual void init(TransactionID const &TranID);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);
   // </ServiceBase>

   // <ICCIAFU>
   virtual void WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID);

   virtual void     WorkspaceFree(btVirtAddr           Address,
                                  TransactionID const &TranID);

   virtual btBool         CSRRead(btCSROffset CSR,
                                  btCSRValue *pValue);

   virtual btBool        CSRWrite(btCSROffset CSR,
                                  btCSRValue  Value);
   virtual btBool      CSRWrite64(btCSROffset CSR,
                                  bt64bitCSR  Value);
   // </ICCIAFU>

protected:
   typedef std::map<btVirtAddr, buffer_t> map_t;
   typedef map_t::iterator                map_iter;
   typedef map_t::const_iterator          const_map_iter;

   btCSRValue             m_Last3c4;
   btCSRValue             m_Last3cc;
   map_t                  m_WkspcMap;

   static CriticalSection sm_ASEMtx;
};

/// @} group ASECCIAFU

END_NAMESPACE(AAL)

#endif // __ASECCIAFU_H__

