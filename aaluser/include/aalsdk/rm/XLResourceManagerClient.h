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
/// @file XLResourceManagerClient.h
/// @brief XLResourceManagerClient - Public Interface to ResourceManagerClient
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///          
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/20/2014     JG       Initial version started@endverbatim
//****************************************************************************
#ifndef __AALSDK_XL_AASRESOURCEMANAGERCLIENT_H__
#define __AALSDK_XL_AASRESOURCEMANAGERCLIENT_H__
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/kernel/aalrm_client.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Constants and Events for methods in IResourceManagerClientService
//=============================================================================

//=============================================================================
// Name: IResourceManagerClient
// Description: Interface implemented by Resource Manager Clients
//=============================================================================
class XLRESOURCEMANAGERCLIENT_API IResourceManagerClient
{
public:
   virtual ~IResourceManagerClient(){};

   virtual void resourceAllocated( NamedValueSet const &nvsInstancerecord,
                                   TransactionID const &tid ) = 0;
   virtual void resourceRequestFailed( NamedValueSet const &nvsManifest,
                                       const IEvent &rEvent ) = 0;
   virtual void resourceManagerException( const IEvent &rEvent ) = 0;

};

END_NAMESPACE(AAL)

#endif // __AALSDK_XL_AASRESOURCEMANAGERCLIENT_H__

