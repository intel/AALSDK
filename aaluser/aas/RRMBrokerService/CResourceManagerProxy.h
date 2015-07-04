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
/// @file CResourceManagerProxy.h
/// @brief ResourecManagerProxy is the local proxy interface to the remote
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
#ifndef __AALSDK_XL_CRESOURCEMANAGERPROXY_H__
#define __AALSDK_XL_CRESOURCEMANAGERPROXY_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/CUnCopyable.h>
#include <aalsdk/rm/AALResourceManagerProxy.h>
#include <aalsdk/osal/CriticalSection.h>

#include "aalsdk/kernel/aalrm.h"             // kernel transport services

BEGIN_NAMESPACE(AAL)

//==========================================================================
// Name: rmMessage
// Description: Wrapper for messages
//==========================================================================
class rmMessage : private CUnCopyable
{
public:
   rmMessage();
   virtual ~rmMessage();

   void clear();

   // Expected to be used only once, after initialization, so does NOT clear
   //    an existing payload before allocating a new one
   void setSize(btWSSize PayloadSize);

   operator struct aalrm_ioctlreq *        ()   { return &m_message;            }
            struct aalrm_ioctlreq * GetReqp()   { return &m_message;            }

   btObjectType             handle()      const { return m_message.res_handle;  }
   rms_msgIDs_e             id()          const { return m_message.id;          }
   rms_result_e             result_code() const { return m_message.result_code; }
   btWSSize                 size()        const { return m_message.size;        }
   stTransactionID_t const &tranID()      const { return m_message.tranID;      }
   btObjectType const       context()     const { return m_message.context;     }
   btVirtAddr   const       payload()     const { return m_message.payload;     }

protected:
   struct aalrm_ioctlreq m_message;
}; // class rmcMessage



//==========================================================================
// Name: CResourceManagerProxy
// Description: The ResourceManagerClient is a wrapper object around the
//              low level driver interface to the Remote Resource Manager.
//==========================================================================
class CResourceManagerProxy : public  IResourceManagerProxy,
                              private CriticalSection,
                              private CUnCopyable
{
public:

   //==========================================================================
   // Name: ResourceManagerProxy
   // Description: Constructor
   //==========================================================================
   CResourceManagerProxy();

   //==========================================================================
   // Name: ~ResourceManagerProxy
   // Description: Destructor
   //==========================================================================
   ~CResourceManagerProxy();

   //==========================================================================
   // Name: Open
   // Description: Open the channel to the service
   //==========================================================================
   btBool Open();

   //==========================================================================
   // Name: Close
   // Description: lose the channel to the service
   //==========================================================================
   void Close();

   //==========================================================================
   // Name:          GetMessage
   // Description:   Polls for messages and returns when one is available
   //     OutPut:    NamedValueSet - Message
   //                TransactionID - Transaction ID if message is a response
   // Comment:       Returns a NamedValueSet with the message. The
   //
   //==========================================================================
   btBool GetMessage(NamedValueSet &, TransactionID &);

   //==========================================================================
   // Name: Send
   // Description: Sends a message down RMC connection
   //==========================================================================
   btBool SendRequest( NamedValueSet const   &nvsManifest,
                       TransactionID const   &tid);

   btBool IsOK() { return m_bIsOK; }

private:
   btUnsignedInt MapEnumID(rms_msgIDs_e);
   btIID MapEnumReasonCode(rms_result_e);
   btcString MapEnumResultString(rms_result_e);

   int    m_fdRMClient;
   btBool m_bIsOK;
}; // class CResourceManagerProxy


END_NAMESPACE(AAL)

#endif // __AALSDK_XL_CRESOURCEMANAGERPROXY_H__

