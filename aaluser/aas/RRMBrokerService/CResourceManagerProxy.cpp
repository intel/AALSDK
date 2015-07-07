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
/// @file CResourceManagerProxy.cpp
/// @brief Implementation of the Resource Manager Proxy
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/19/2014     JG       Initial version started@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "CResourceManagerProxy.h"
#include "aalsdk/AALIDDefs.h"
#include "aalsdk/kernel/aalrm_client.h"
#include "aalsdk/AALLoggerExtern.h"          // Logger



#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
     #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
     #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif


BEGIN_NAMESPACE(AAL)
//==========================================================================
// Name: rmMessage
// Description: Wrapper for messages
//==========================================================================
rmMessage::rmMessage()
{
   memset(&m_message, 0, sizeof(struct aalrm_ioctlreq));
}

rmMessage::~rmMessage()
{
   if ( NULL != m_message.payload ) {
      delete[] m_message.payload;
   }
}

void rmMessage::clear()
{
   if ( NULL != m_message.payload ) {
      delete[] m_message.payload;
   }
   memset(&m_message, 0, sizeof(struct aalrm_ioctlreq));
}

//==========================================================================
// Name: setSize
// Description: Allocate a payload buffer
//==========================================================================
void rmMessage::setSize(btWSSize PayloadSize)
{
   ASSERT(PayloadSize > 0);
   if ( PayloadSize > 0 ) {
      m_message.payload = (btVirtAddr)new char[(size_t)PayloadSize];
      memset(m_message.payload, 0, (size_t)PayloadSize);
      m_message.size = PayloadSize;
   }
}


//==========================================================================
// Name: ResourceManagerProxy
// Description: Constructor
//==========================================================================
CResourceManagerProxy::CResourceManagerProxy() :
   m_fdRMClient(-1),
   m_bIsOK(false)
{}

//==========================================================================
// Name: ~CResourceManagerProxy
// Description: Destructor
//==========================================================================
CResourceManagerProxy::~CResourceManagerProxy()
{
   Close();
}

//==========================================================================
// Name: Open
// Description: Open channel to remote service
//==========================================================================
btBool CResourceManagerProxy::Open()
{
   // Open the device
   if ( -1 != m_fdRMClient ) {
      return true;
   }

   std::string strName="/dev/";

   strName += RESMGR_DEV_NAME;

   m_fdRMClient = open(strName.c_str(), O_RDWR);
   if ( -1 == m_fdRMClient ) {
      m_bIsOK = false;
      return m_bIsOK;
   }
   m_bIsOK = true;
   return true;
}

//==========================================================================
// Name: Close
// Description: Close channel to remote service
//==========================================================================
void CResourceManagerProxy::Close()
{
   if ( -1 != m_fdRMClient ) {
      close(m_fdRMClient);
      m_fdRMClient = -1;
      m_bIsOK = false;
   }
}

//==========================================================================
// Name: Message ID mapping functions
// Description:
// Input:
// OutPut:    NamedValueSet - Message
//            TransactionID - Transaction ID if message is a response
// Comment:       Returns a NamedValueSet with the message. The
//
//==========================================================================
btUnsignedInt CResourceManagerProxy::MapEnumID(rms_msgIDs_e num)
{
   switch(num)
   {
      case rspid_URMS_RequestDevice:
         return rm_msg_id_resourceAllocated;
      case reqid_URMS_ReleaseDevice:
         return rm_msg_id_resourceFreed;
      case rspid_Shutdown:
         return rm_msg_id_shutdown;
      default:
         return rm_msg_id_Unknown;
   }
}


btIID CResourceManagerProxy::MapEnumReasonCode(rms_result_e num)
{
   switch(num)
   {
      case rms_resultOK:
         return errOK;
      case rms_resultErrno:
         return errCreationFailure;
      default:
         return errCauseUnknown;
   }
}

btcString CResourceManagerProxy::MapEnumResultString(rms_result_e num)
{
   switch(num)
   {
      case rms_resultOK:
         return strNoError;
      case rms_resultErrno:
         return strNoResourceDescr;
      default:
         return strUnknownCause;
   }

}


//==========================================================================
 // Name:          GetMessage
 // Description:   Polls for messages and returns when one is available
 //     OutPut:    NamedValueSet - Message
 //                TransactionID - Transaction ID if message is a response
 // Comment:       Returns a NamedValueSet with the message. The
 //
 //==========================================================================
btBool CResourceManagerProxy::GetMessage( NamedValueSet &nvs,
                                          TransactionID &tid )
{
   if ( !IsOK() ) {
      return NULL;
   }

   rmMessage Message;

   struct pollfd pollfds[1];
   int           ret;

   pollfds[0].fd     = m_fdRMClient;
   pollfds[0].events = POLLPRI;

   // Break out of the while with a return

   do
   {
      // Check for messages first
      if ( 0 == ( ret = ioctl(m_fdRMClient, AALRM_IOCTL_GETMSG_DESC, Message.GetReqp()) ) ) {

         // Initially the message has a zero sized payload. The message descriptor message indicates
         //  the size of the available message.
         //  Allocate a payload buffer from the size returned in the message
         Message.setSize(Message.size());

         // Get the message and its payload
         if ( -1 == ioctl(m_fdRMClient, AALRM_IOCTL_GETMSG, Message.GetReqp()) ) {
            goto FAILED;
         }

         // Covert record into a Named Value for upstream processing
         nvs = NamedValueSet(Message.payload(), Message.size());

         nvs.Add(RM_MESSAGE_KEY_ID, MapEnumID(Message.id()));
         nvs.Add(RM_MESSAGE_KEY_RESULTCODE, (Message.result_code() == rms_resultOK ? errOK : errMethodFailure) );
         nvs.Add(RM_MESSAGE_KEY_REASONCODE, MapEnumReasonCode(Message.result_code()));
         nvs.Add(RM_MESSAGE_KEY_REASONSTRING, MapEnumResultString(Message.result_code()));
         tid = Message.tranID();
         Message.clear();
         return true;
      }

      ret = poll(pollfds,1,-1);

   }while( ( 0 == ret ) || ( ret != -EAGAIN ) );

FAILED:
   perror("CResourceManagerProxy::GetMessage poll failed");
   // If got here then the poll failed
   return false;
}

//==========================================================================
// Name: SendRequest
// Description: Request a service matching Manifest criteria.
//==========================================================================
btBool CResourceManagerProxy::SendRequest(NamedValueSet const   &nvsManifest,
                                          TransactionID const   &tid)
{
   AutoLock(this);

   if ( !IsOK() ) {
      return IsOK();
   }

   struct aalrm_ioctlreq req;

   memset(&req, 0, sizeof(req));

   // Marshal the NVS into an ostringstream
   std::string        temp(nvsManifest);
   btUnsigned32bitInt len  = temp.length();

   btVirtAddr buf = (btVirtAddr)new(std::nothrow) btByte[len];

   if(NULL == buf){
      m_bIsOK = false;
      return IsOK();
   }

   BufFromString(buf, temp);

   req.id      = reqid_URMS_RequestDevice;
   req.size    = len;
   req.payload = buf;
   req.tranID  = (stTransactionID_t const&)tid;

   if ( -1 == ioctl(m_fdRMClient, AALRM_IOCTL_SENDMSG, &req) ) {
      perror("CResourceManagerProxy::Send");
      m_bIsOK = false;
   }
   delete[] buf;
   return IsOK();
}

END_NAMESPACE(AAL)

