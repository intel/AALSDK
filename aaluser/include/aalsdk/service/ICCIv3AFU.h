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
/// @file ICCIv3AFU.h
/// @brief ICCIv3AFU Service definition.
/// @ingroup ICCIv3AFU
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
#ifndef __AALSDK_SERVICE_ICCIV3AFU_H__
#define __AALSDK_SERVICE_ICCIV3AFU_H__
#include <aalsdk/AAL.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup ICCIv3AFU
/// @{

/// Key for selecting an AFU delegate.
#define CCIV3AFU_NVS_KEY_TARGET "CCIv3AFUTarget"
/// Value - selects ASECCIAFU
# define CCIV3AFU_NVS_VAL_TARGET_ASE   "CCIv3AFUTarget_ASE"
/// Value - selects HWCCIAFU
# define CCIV3AFU_NVS_VAL_TARGET_FPGA  "CCIv3AFUTarget_FPGA"
/// Value - selects SWSimCCIAFU
# define CCIV3AFU_NVS_VAL_TARGET_SWSIM "CCIv3AFUTarget_SWSim"

/// CCIv3 Service interface.
/// @brief Defines the functionality available to all CCIv3 AFU's.
class ICCIv3AFU
{
public:
   virtual ~ICCIv3AFU() {}

   /// @brief Allocate a Workspace.
   ///
   /// @param[in]  Length   Requested length, in bytes.
   /// @param[in]  TranID   Returned in the notification event.
   ///
   /// On success, the workspace parameters are notified via ICCIClient::OnWorkspaceAllocated.
   /// On failure, an error notification is sent via ICCIClient::OnWorkspaceAllocateFailed.
   virtual void WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID) = 0;

   /// @brief Free a previously-allocated Workspace.
   ///
   /// The provided workspace Address must have been acquired previously by ICCIAFU::WorkspaceAllocate.
   ///
   /// @param[in]  Address  User virtual address of the workspace.
   /// @param[in]  TranID   Returned in the notification event.
   ///
   /// On success, a notification is sent via ICCIClient::OnWorkspaceFreed.
   /// On failure, an error notification is sent via ICCIClient::OnWorkspaceFreeFailed.
   virtual void WorkspaceFree(btVirtAddr           Address,
                              TransactionID const &TranID) = 0;

   /// @brief Read a CCI-attached AFU's CSR.
   /// @note  Applies only to those CCI AFU CSR's which are noted as read/write.
   ///
   /// Synchronous function; no TransactionID. Generally very fast.
   ///
   /// @param[in]  CSR    Number of CSR to set, starting at 0. Specifics of indexing are
   ///                       somewhat platform specific. SPL 1 is always an AFU index.
   ///                       SPL 2 is currently defined as an index from the beginning of
   ///                       the global CSR space, e.g. AFU space starts at offset 0xA00,
   ///                       or index 640 (decimal).
   /// @param[out] Value  Where to place value read. Returned value will be 32-bit in
   ///                       QPI and PCI incarnations.
   ///
   /// @return whether the read was successful.
   virtual btBool CSRRead(btCSROffset CSR, btCSRValue *pValue) = 0;

   /// @brief Write an AFU's CSR.
   ///
   /// Synchronous function; no TransactionID. Generally very fast.
   ///
   /// @param[in]  CSR    Number of CSR to set, starting at 0. Specifics of indexing are
   ///                       somewhat platform specific. SPL 1 is always an AFU index.
   ///                       SPL 2 is currently defined as an index from the beginning of
   ///                       the global CSR space, e.g. AFU space starts at offset 0xA00,
   ///                       or index 640 (decimal).
   /// @param[in]  Value  Value to set it to. QPI/PCIe uses 32-bit CSRs.
   ///
   /// @return whether the write was successful.
   virtual btBool   CSRWrite(btCSROffset CSR, btCSRValue Value) = 0;

   /// @brief Write an AFU's 64-bit CSR, upper 4 bytes followed by lower 4 bytes.
   /// @note This API represents a special hw/sw contract for the targeted CSR by enforcing the
   ///       ordering of the writes in the manner described. Refer to the description of the CSRs
   ///       in the hardware documentation for further details.
   ///
   /// Synchronous function; no TransactionID. Generally very fast.
   ///
   /// @param[in]  CSR    Number of CSR to set, starting at 0. Specifics of indexing are
   ///                       somewhat platform specific. SPL 1 is always an AFU index.
   ///                       SPL 2 is currently defined as an index from the beginning of
   ///                       the global CSR space, e.g. AFU space starts at offset 0xA00,
   ///                       or index 640 (decimal).
   /// @param[in]  Value  Value to set it to.
   ///
   /// @return whether the write was successful.
   virtual btBool CSRWrite64(btCSROffset CSR, bt64bitCSR Value) = 0;

}; // ICCIv3AFU

/// @} group ICCIv3AFU

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_ICCIV3AFU_H__

