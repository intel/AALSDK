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
/// @file ALIAFUProxy.h
/// @brief Definitions for the ALI AFU Proxy Class used by the AIA Service.
/// @ingroup AIAService
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 9/15/2015      JG       Initial version
//****************************************************************************
#ifndef __AALSDK_ALIAFUPROXY_H__
#define __AALSDK_ALIAFUPROXY_H__
#if 0
#include <aalsdk/AALTypes.h>
#include <aalsdk/CAALBase.h>                    // CAALBase
#include <aalsdk/AALEvent.h>                    // IEvent
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/uaia/uidrvMessage.h>           // uidrvMessageRoute
#include <aalsdk/uaia/AIA.h>                    // IAFUDev, IAFUTransaction, IAFUCSRMap
#include <aalsdk/uaia/IUAIASession.h>           // IuAIASession
#include <aalsdk/uaia/AALuAIA_Messaging.h>      // UIDriverClient_uidrvMarshaler_t

#include <aalsdk/kernel/aalui.h>                // aalui_extbindargs

#include <aalsdk/utils/AALWorkSpaceUtilities.h> // WorkSpaceMapper
#endif

#include <aalsdk/uaia/AIA.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/INTCDefs.h>
#include <aalsdk/uaia/IAFUProxy.h>

#include "AIA-internal.h"


//=============================================================================
// Name: ALIAFUProxy
// Description: AFU Proxies are objects that abstract the connection/transport
//              layer and implementation details of the Accelerated Function
//              Unit (AFU). It provides a local representation of the AFU.
//=============================================================================
class UAIA_API ALIAFUProxy : public AAL::ServiceBase,
                             public IAFUProxy,
                             public IAFUProxyClient
{
public:

   DECLARE_AAL_SERVICE_CONSTRUCTOR(ALIAFUProxy, ServiceBase),
      m_pSvcClient(NULL),
      m_pClient(NULL),
      m_pAIABase(NULL),
      m_pAIA(NULL),
      m_devHandle(NULL)
   {
      m_bIsOK = false;         // CAASBase set it to true

      if(SetInterface(iidAFUProxy,dynamic_cast<IAFUProxy*>(this))!= EObjOK) {
         return;
      }
   }
   ~ALIAFUProxy();

   // Manditory interfaces from ServiceBase
   // <ServiceBase>
   virtual AAL::btBool init( AAL::IBase *pclientBase,
                             AAL::NamedValueSet const &optArgs,
                             AAL::TransactionID const &rtid);

   virtual AAL::btBool Release(AAL::TransactionID const &rTranID, AAL::btTime timeout = AAL_INFINITE_WAIT);
   //</ServiceBase>

   // Send a message to the device
   AAL::btBool SendTransaction( IAIATransaction *pAFUmessage);


#if 0
   // Accessors to memory mapped regions
   AAL::btVirtAddr getCSRBase();

   AAL::btVirtAddr getMMIORBase();
   AAL::btUnsigned32bitInt getMMIORsize();

   AAL::btVirtAddr getUMSGBase();
   AAL::btUnsigned32bitInt getUMSGsize();
#endif
protected:
   void AFUEvent( AAL::IEvent const &theEvent);


   AAL::IServiceClient   *m_pSvcClient;
   IAFUProxyClient       *m_pClient;
   AAL::IBase            *m_pAIABase;
   AIAService            *m_pAIA;
   btHANDLE               m_devHandle;


};

#endif //__AALSDK_ALIAFUPROXY_H__

