// Copyright(c) 2015-2016, Intel Corporation
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
/// @file IResConfig.h
/// @brief IResConfig definition.
/// @ingroup IResConfig
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/15/2015     JG       Initial version.@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_IRESCONFIG_H__
#define __AALSDK_SERVICE_IRESCONFIG_H__
#include <aalsdk/AALTypes.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup IResConfig
/// @{

/// Resource Configuration Service Client interface.
/// @brief Defines the notification interface for clients of Resource Configuration Service.
///        Interface ID: iidResConfClient
class IResConfigServiceClient
{
public:
   virtual ~IResConfigServiceClient() {}

   /// @brief Notification callback for IResConfigServce::Configure().
   ///
   /// Sent in response to a successful completion of the Resource Configuration)
   ///
   /// @param[in]  TranID      The transaction ID provided in the call to ISPLAFU::StartTransactionContext.
   /// @returns void
   virtual void  configureComplete(TransactionID const &TranID) = 0;
   /// @brief  Notification callback for IResConfigServce::Configure().
   ///
   /// Sent in response to failure of completion of Resource Configuration.
   /// @param theEvent A reference to an Event containing details of the failure.
   /// @returns void
   virtual void  configureFailed(const IEvent &theEvent) = 0;

};


/// Resource Configuration Service Client interface.
/// @brief Defines the notification interface for clients of Resource Configuration Service.
///        Interface ID: iidResConfService
class IResConfigService
{
public:
   virtual ~IResConfigService() {}

   /// @brief Notification callback for SPL transaction started success.
   ///
   /// Sent in response to a successful transaction start request (ISPLAFU::StartTransactionContext).
   ///
   /// @param[in]  nvsManifest The Manifest describing the Resource Configuration desired.
   /// @param[in]  TranID      The transaction ID provided in the call to IResConfigServiceClient.
   virtual void  configureResource( NamedValueSet const &nvsManifest,
                                    TransactionID const &TranID) = 0;


};


/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_IRESCONFIG_H__

