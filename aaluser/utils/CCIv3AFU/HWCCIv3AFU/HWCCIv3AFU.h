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
/// @file HWCCIv3AFU.h
/// @brief Definitions for CCIv3 Hardware AFU Service.
/// @ingroup HWCCIv3AFU
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
#ifndef __HWCCIV3AFU_H__
#define __HWCCIV3AFU_H__
#include <aalsdk/service/CCIv3AFUService.h>
#include <aalsdk/service/HWCCIv3AFUService.h>
#include <aalsdk/service/ICCIv3AFU.h>
#include <aalsdk/service/ICCIClient.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWCCIv3AFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because ICCIv3AFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::ICCIv3AFU' used as base for dll-interface class 'AAL::HWCCIv3AFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by CCIv3AFU to interact with an FPGA-accelerated
///        CCI.
///
/// HWCCIv3AFU is selected by passing the Named Value pair (CCIV3AFU_NVS_KEY_TARGET, CCIV3AFU_NVS_VAL_TARGET_FPGA)
/// in the arguments to IRuntime::allocService when requesting a CCIv3AFU.
class HWCCIV3AFU_API HWCCIv3AFU : public HWAFUWkspcDelegate<MASTER_VIRT_MODE>,
                                  public ICCIv3AFU
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <DeviceServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(HWCCIv3AFU, HWAFUWkspcDelegate<MASTER_VIRT_MODE>)
   {
      SetInterface(        iidCCIv3AFU,   dynamic_cast<ICCIv3AFU *>(this));
      SetSubClassInterface(iidHWCCIv3AFU, dynamic_cast<ICCIv3AFU *>(this));
   }

   virtual void init(TransactionID const &TranID);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);
   // </DeviceServiceBase>

   // <ICCIv3AFU>
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
   // </ICCIv3AFU>
};

/// @}

END_NAMESPACE(AAL)

#endif // __HWCCIV3AFU_H__

