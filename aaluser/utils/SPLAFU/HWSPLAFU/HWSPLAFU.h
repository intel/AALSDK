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
/// @file HWSPLAFU.h
/// @brief Definitions for SPL AFU Service.
/// @ingroup HWSPLAFU
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
/// 07/18/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __HWSPLAFU_H__
#define __HWSPLAFU_H__
#include <aalsdk/service/ISPLAFU.h>
#include <aalsdk/service/ISPLClient.h>
#include <aalsdk/service/SPLAFUService.h>
#include <aalsdk/service/HWSPLAFUService.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWSPLAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because ISPLAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::ISPLAFU' used as base for dll-interface class 'AAL::HWSPLAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by SPLAFU to interact with an FPGA-accelerated
///        SPL.
///
/// HWSPLAFU is selected by passing the Named Value pair (SPLAFU_NVS_KEY_TARGET, SPLAFU_NVS_VAL_TARGET_FPGA)
/// in the arguments to IRuntime::allocService when requesting an SPLAFU.
class HWSPLAFU_API HWSPLAFU : public HWAFUWkspcDelegate<MASTER_VIRT_MODE>,
                              public ISPLAFU
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <DeviceServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(HWSPLAFU, HWAFUWkspcDelegate<MASTER_VIRT_MODE>)
   {
      SetInterface(        iidSPLAFU,   dynamic_cast<ISPLAFU *>(this));
      SetInterface(iidHWSPLAFU, dynamic_cast<ISPLAFU *>(this));
   }

   virtual btBool init( IBase *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);
   // </DeviceServiceBase>

   // <ISPLAFU>
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

   virtual void StartTransactionContext(TransactionID const &TranID,
                                        btVirtAddr           Address=NULL,
                                        btTime               Pollrate=0);

   virtual void StopTransactionContext(TransactionID const &TranID);

   virtual void SetContextWorkspace(TransactionID const &TranID,
                                    btVirtAddr           Address,
                                    btTime               Pollrate=0);
   // </ISPLAFU>

protected:
   static void TransactionHandler(const IEvent & );
};

/// @}

END_NAMESPACE(AAL)

#endif // __HWSPLAFU_H__

