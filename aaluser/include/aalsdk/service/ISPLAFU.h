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
/// @file ISPLAFU.h
/// @brief ISPLAFU Service definition.
/// @ingroup ISPLAFU
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
/// 07/18/2014     TSW      Adapted from UtilsAFU.@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_ISPLAFU_H__
#define __AALSDK_SERVICE_ISPLAFU_H__
#include <aalsdk/service/ICCIAFU.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup ISPLAFU
/// @{

/// Key for selecting an AFU delegate.
#define SPLAFU_NVS_KEY_TARGET "SPLAFUTarget"
/// Value - selects ASESPLAFU
# define SPLAFU_NVS_VAL_TARGET_ASE   "SPLAFUTarget_ASE"
/// Value - selects HWSPLAFU
# define SPLAFU_NVS_VAL_TARGET_FPGA  "SPLAFUTarget_FPGA"
/// Value - selects SWSimSPLAFU
# define SPLAFU_NVS_VAL_TARGET_SWSIM "SPLAFUTarget_SWSim"

/// SPL Service interface.
/// @brief Defines the functionality available to all SPL AFU's.
class ISPLAFU : public ICCIAFU
{
public:
   virtual ~ISPLAFU() {}

   /// @brief Sets the SPL AFU context and (optionally) starts an SPL transaction.
   ///
   /// When supplied, Address must be the start of the workspace (AFU Context) to be used for the
   /// SPL transaction. The SPL transaction will be started when non-NULL Address is given. If no
   /// Address is given, then the SPL transaction can be started by calling
   /// ISPLAFU::SetContextWorkspace.
   ///
   /// @param[in]  TranID    returned in the notification event.
   /// @param[in]  Address   user mode virtual pointer to AFU Context. (optional)
   /// @param[in]  Pollrate  driver polling period in milliseconds.
   ///
   /// Pollrate specifies the interval between status checks of the SPL device performed by the
   /// driver software.
   ///
   /// On success, the status is notified via ISPLClient::OnTransactionStarted, which also provides
   /// the address and size of the AFU Device Status Memory.
   ///
   /// On failure, an error notification is sent via ISPLClient::OnTransactionFailed.
   virtual void StartTransactionContext(TransactionID const &TranID,
                                        btVirtAddr           Address=NULL,
                                        btTime               Pollrate=0) = 0;

   /// @brief Forcibly terminate any in-progress SPL transaction.
   ///
   /// The notification of termination of the transaction is sent via ISPLClient::OnTransactionStopped,
   /// regardless of whether transaction was in progress as of the call to StopTransactionContext.
   virtual void StopTransactionContext(TransactionID const &TranID) = 0;

   /// @brief Starts an SPL transaction in the case that no AFU Context was provided to
   ///        ISPLAFU::StartTransactionContext.
   ///
   /// When ISPLAFU::StartTransactionContext is called with a NULL AFU Context pointer, the
   /// framework replies with a notification via ISPLClient::OnTransactionStarted, also supplying
   /// a pointer to the AFU Device Status Memory. Applications can then query the AFU DSM to
   /// perform further configuration before starting the transaction.
   ///
   /// @param[in]  TranID    returned in the notification event.
   /// @param[in]  Address   user mode virtual pointer to the AFU Context. (required)
   /// @param[in]  Pollrate  driver polling period in milliseconds.
   ///
   /// Address must be the start of a non-NULL AFU Context workspace to be used for the SPL transaction.
   ///
   /// Pollrate specifies the interval between status checks of the SPL device performed by the
   /// driver software.
   ///
   /// On success, the status is notified via ISPLClient::OnContextWorkspaceSet.
   /// On failure, an error notification is sent via ISPLClient::OnTransactionFailed.
   virtual void SetContextWorkspace(TransactionID const &TranID,
                                    btVirtAddr           Address,
                                    btTime               Pollrate=0) = 0;
}; // ISPLAFU

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_ISPLAFU_H__

