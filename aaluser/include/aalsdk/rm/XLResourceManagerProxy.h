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
/// @file XLResourceManagerProxy.h
/// @brief IResourecManagerProxy is the local proxy interface to the remote
///        Resource Manager Service.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/19/2014     JG       Initial version started@endverbatim
//****************************************************************************
#ifndef __AALSDK_XL_RESOURCEMANAGERPROXY_H__
#define __AALSDK_XL_RESOURCEMANAGERPROXY_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALNamedValueSet.h>

#include <aalsdk/AALTransactionID.h>

BEGIN_NAMESPACE(AAL)
/////////////////////////////////////////////////////////////////////////////
///
///                      Resource Manager Message Keys and values
///
/////////////////////////////////////////////////////////////////////////////
#define RM_MESSAGE_KEY_ID                 "rm_message_key_id"
typedef enum
{
   rm_msg_id_Unknown = 0,
   rm_msg_id_resourceAllocated,
   rm_msg_id_resourceFreed,
   rm_msg_id_shutdown,

}rm_msg_ids;

#define RM_MESSAGE_KEY_RESULTCODE        "rm_message_key_resultcode"
#define RM_MESSAGE_KEY_REASONCODE        "rm_message_key_reasoncode"
#define RM_MESSAGE_KEY_REASONSTRING      "rm_message_key_reasonstring"



//==========================================================================
// Name: IResourceManagerProxy
// Description: The IResourceManagerProxy is a wrapper object around the
//              low level interface to the Remote Resource Manager.
//==========================================================================
class IResourceManagerProxy
{
public:

   //==========================================================================
   // Name: ~ResourceManagerProxy
   // Description: Destructor
   //==========================================================================
   virtual ~IResourceManagerProxy(){};

   //==========================================================================
   // Name: Open
   // Description: Open the channel to the service
   //==========================================================================
   virtual btBool Open() = 0;

   //==========================================================================
   // Name: Close
   // Description: Close the channel to the service
   //==========================================================================
   virtual void Close() = 0;

   //==========================================================================
   // Name:          GetMessage
   // Description:   Polls for messages and returns when one is available
   //     OutPut:    NamedValueSet - Message
   //                TransactionID - Transaction ID if message is a response
   // Comment:       Returns a NamedValueSet with the message. The
   //
   //==========================================================================
   virtual btBool GetMessage(NamedValueSet &, TransactionID &tid) = 0;

   //==========================================================================
   // Name: SendRequest
   // Description: Sends a request down Remote Resource Manager connection
   //==========================================================================
   virtual btBool SendRequest( NamedValueSet const   &nvsManifest,
                               TransactionID const   &tid) = 0 ;

   virtual btBool IsOK() = 0 ;
}; // class IResourceManagerProxy


END_NAMESPACE(AAL)

#endif // __AALSDK_XL_RESOURCEMANAGERPROXY_H__

