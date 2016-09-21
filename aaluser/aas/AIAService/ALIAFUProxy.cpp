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
/// @file ALIAFUProxy.cpp
/// @brief Implementation of the ALI AFUProxy Class. The AFUProxy class
///          is an abstraction of the AFU.
/// @ingroup uAIA
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// PURPOSE: 
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/15/2015     JG       Initial version@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aalsdk/AALLoggerExtern.h"

#include "ALIAFUProxy.h"
#include "AIA-internal.h"

#include "AIATransactions.h"

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: ALIAFUProxy
// Description: The AFUDev object is the proxy to the physical AFU engine
//              implementation on a device. It abstracts a session between the
//              Host AFU and the device through the AIA.
//
//              The ALIAFUProxy sends messages to the device through
//              ITransaction objects.
// Comments:
//=============================================================================

// Destructor
ALIAFUProxy::~ALIAFUProxy() {}

//=============================================================================
// Name: init
// Description: Initialize the Proxy
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
btBool ALIAFUProxy::init( IBase *pclientBase,
                          NamedValueSet const &optArgs,
                          TransactionID const &rtid)
{
   // Make sure the AIA passed in its interface
   ASSERT( true == optArgs.Has(AIA_SERVICE_BASE_INTERFACE) );
   if(true != optArgs.Has(AIA_SERVICE_BASE_INTERFACE)){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Did not get AIA interface for proxy"));
      return true;
   }


   // Get the AIA pointer
   if( ENamedValuesOK != optArgs.Get(AIA_SERVICE_BASE_INTERFACE, static_cast<btAny*>(static_cast<btAny>(&m_pAIABase)) )){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "FATAL Error: Manifest said AIA interface present but could not be acquired!"));
      return true;
   }
   m_pAIA = dynamic_ptr<AIAService>(iidAIAService, m_pAIABase);

   //
   // Check Client for proper interface

   m_pClient = dynamic_ptr<IAFUProxyClient>(iidAFUProxyClient, pclientBase);
   ASSERT(m_pClient != NULL);
   // TODO initFailed if m_pClient == NULL?


   // Bind the AFU device to complete transfer of ownership
   // Get the device handle if there is one
   if( optArgs.Has(keyRegHandle) ) {
      optArgs.Get(keyRegHandle, &m_devHandle);
   }else {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                rtid,
                                                errBadParameter,
                                                reasNoDevice,
                                                "No device handle in Configuration Record!"));
      return true;
    }
   // Bind to device and report when we get Bind complete AFU Event,
   //  The first argument to the bind transaction is the Owner (i.e., ProxyClient). This is where
   //  all HW messages will be sent by default.
   m_pAIA->SendMessage(m_devHandle, new BindAFUDevice(m_pClient, rtid), dynamic_cast<IAFUProxyClient*>(this) );

}

//=============================================================================
// Name: SendTransaction
// Description: Send a message to the device
// Inputs: pAFUmessage - Transaction object
//         rtid - Transaction ID
// Outputs: true - success
// Comments:
//=============================================================================
btBool ALIAFUProxy::SendTransaction(IAIATransaction *pAFUmessage)
{
   m_pAIA->SendMessage(m_devHandle, pAFUmessage, m_pClient);
   return true;  /// SendMessage is a void TDO cleanup
}



AAL::btBool ALIAFUProxy::MapWSID(AAL::btWSSize Size, AAL::btWSID wsid, AAL::btVirtAddr *pRet, AAL::NamedValueSet const &optArgs)
{
   return m_pAIA->MapWSID(Size, wsid, pRet, optArgs);
}

void ALIAFUProxy::UnMapWSID(AAL::btVirtAddr ptr, AAL::btWSSize Size)
{
   m_pAIA->UnMapWSID(ptr, Size);
}



//=============================================================================
// Name: AFUEvent
// Description: Callback when an upstream messsage is received
// Inputs: theEvent - AIA message event
// Outputs: true - success
// Comments:
//=============================================================================
void ALIAFUProxy::AFUEvent( AAL::IEvent const &theEvent)
{
   IUIDriverEvent * puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                            theEvent);

   ASSERT(NULL != puidEvent);
   if (NULL == puidEvent)
   {
      return;
   }

   switch(puidEvent->MessageID())
   {
      case rspid_UID_Shutdown:
      {
         std::cerr<<"Got rspid_UID_Shutdown event" << std::endl;
      }
      break;
      case rspid_UID_UnbindComplete:
      {
         m_pAIA->AFUProxyRelease(this);
         ServiceBase::Release( TransactionID(puidEvent->msgTranID()),
                               AAL_INFINITE_WAIT);
      }
      break;

      // Bind Complete
      case rspid_UID_BindComplete:
      {
         // Initialization is complete
         // TODO - Save Bind Parms.
         m_pAIA->AFUProxyAdd(this);
         initComplete(puidEvent->msgTranID());
      }
      break;
   }

}

//=============================================================================
// Name: Release
// Description: Unbinds the AFUProxy and Releases
// Inputs: rtid - TransactionID
//         timeout - Timeout
// Outputs: true - success
// Comments:
//=============================================================================
btBool ALIAFUProxy::Release(AAL::TransactionID const &rtid, AAL::btTime timeout)
{
   m_pAIA->SendMessage(m_devHandle, new UnBindAFUDevice(rtid), dynamic_cast<IAFUProxyClient*>(this) );
   return true;
}

END_NAMESPACE(AAL)

