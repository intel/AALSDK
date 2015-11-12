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
/// @file IAFUProxy.h
/// @brief Definitions for the AFU Proxy Class used by the AIA Service.
/// @ingroup AIAService
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 8/28/2015      JG       Initial version
//****************************************************************************
#ifndef __AALSDK_IAFUPROXY_H__
#define __AALSDK_IAFUPROXY_H__

#include <aalsdk/AALTypes.h>
#include <aalsdk/kernel/ccipdriver.h>

//#include <aalsdk/CAALBase.h>                    // CAALBase
//#include <aalsdk/AALEvent.h>                    // IEvent
//#include <aalsdk/AALTransactionID.h>
//#include <aalsdk/uaia/uidrvMessage.h>           // uidrvMessageRoute
//#include <aalsdk/uaia/AIA.h>                      // IAFUDev, IAFUTransaction, IAFUCSRMap
//#include <aalsdk/uaia/IUAIASession.h>           // IuAIASession
//#include <aalsdk/uaia/AALuAIA_Messaging.h>      // UIDriverClient_uidrvMarshaler_t

//#include <aalsdk/kernel/aalui.h>                // aalui_extbindargs

//#include <aalsdk/utils/AALWorkSpaceUtilities.h> // WorkSpaceMapper

//==========================================================================
// Name: IUIDriverEvent
// Description: AAL Event object containing a message from the UI Device
//              Driver Stack (e.g., A message from the AFU
// IID: iidUICEvent
 //==========================================================================
 class UAIA_API IUIDriverEvent
 {
 public:
    virtual ~IUIDriverEvent(){};
    virtual AAL::btHANDLE                  DevHandle()  const              = 0;
    virtual AAL::uid_msgIDs_e              MessageID()  const              = 0;
    virtual AAL::btVirtAddr                Payload()    const              = 0;
    virtual AAL::btWSSize                  PayloadLen() const              = 0;
    virtual AAL::stTransactionID_t const & msgTranID()  const              = 0;
    virtual AAL::btObjectType              Context()    const              = 0;
    virtual AAL::uid_errnum_e              ResultCode() const              = 0;
    virtual void                           ResultCode(AAL::uid_errnum_e e) = 0;
 };

//=============================================================================
// Name: IAFUProxyClient
// Description: AFU Proxies are objects that abstract the connection/transport
//              layer and implementation details of the Accelerated Function
//              Unit (AFU). It provides a local representation of the AFU.
// IID: iidAFUProxyClient
//=============================================================================
class IAFUProxyClient
{
public:
   virtual ~IAFUProxyClient(){}
   virtual void AFUEvent( AAL::IEvent const &theEvent) = 0;
};


//=============================================================================
// Name: IAFUProxy
// Description: AFU Proxies are objects that abstract the connection/transport
//              layer and implementation details of the Accelerated Function
//              Unit (AFU). It provides a local representation of the AFU.
// IID: iidAFUProxy
//=============================================================================
class UAIA_API IAFUProxy
{
public:
   ~IAFUProxy(){};
   // Send a message to the device
   virtual AAL::btBool SendTransaction( IAIATransaction *pAFUmessage )       = 0;

   // Map/Unmap Workspace IDs to virtual memory addresses
   virtual AAL::btBool MapWSID(AAL::btWSSize Size, AAL::btWSID wsid, AAL::btVirtAddr *pRet) = 0;
   virtual void UnMapWSID(AAL::btVirtAddr ptr, AAL::btWSSize Size)           = 0;

#if 0
   // Accessors to memory mapped regions
   virtual AAL::btVirtAddr getCSRBase()                                       = 0;

   virtual AAL::btVirtAddr getMMIORBase()                                     = 0;
   virtual AAL::btUnsigned32bitInt getMMIORsize()                             = 0;

   virtual AAL::btVirtAddr getUMSGBase()                                      = 0;
   virtual AAL::btUnsigned32bitInt getUMSGsize()                              = 0;
#endif
}; // class CAFUDev

#endif //__AALSDK_IAFUPROXY_H__

