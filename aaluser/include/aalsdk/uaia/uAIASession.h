// Copyright (c) 2007-2014, Intel Corporation
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
/// @file uAIASession.h
/// @brief Defines Service for the Universal AIA Session.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/29/2009     JG       Added support for CAFUDev initialize
/// 09/01/2011     JG       Removed Proxys@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_UAIASESSION_H__
#define __AALSDK_UAIA_UAIASESSION_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALBase.h>                     // IBase
#include <aalsdk/CAALBase.h>                    // CAASBase
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/AALEvent.h>                    // IEvent
#include <aalsdk/CAALEvent.h>                   // CTransactionEvent
#include <aalsdk/eds/AASEventDeliveryService.h> // IEventDispatcher
#include <aalsdk/aas/AALService.h>              // ServiceBase
#include <aalsdk/uaia/uidrvMessage.h>           // uidrvMessageRoute
#include <aalsdk/uaia/AIA.h>                    // IAFUDev
#include <aalsdk/uaia/IUAIASession.h>           // IuAIASession, forward reference for CAIA
#include <aalsdk/uaia/AALuAIA_Messaging.h>      // UIDriverClient_uidrvMarshaler_t
#include <aalsdk/uaia/FAPPIP_AFUdev.h>          // CAFUDev


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


//=============================================================================
// Name: IBindAFUDevEvent
// Description: Bind Device Complete Transaction Event Interface
// Comments:
//=============================================================================
class UAIA_API IBindAFUDevEvent
{
public:
   virtual IAFUDev *pAFUDev() const = 0;
   virtual ~IBindAFUDevEvent();
};

//=============================================================================
// Name: CBindDevEvent
// Description: Bind Device Complete Transaction Event
// Comments:
//=============================================================================
class UAIA_API CBindDevEvent : public CTransactionEvent,
                               public IBindAFUDevEvent
{
public:
   CBindDevEvent(IBase               *pObject,
                 CAFUDev             *pdev,
                 TransactionID const &TransID);
   virtual IAFUDev *pAFUDev() const;

protected:
   CAFUDev *m_pdev;
};


//=============================================================================
// Name: uAIASession
// Description: Implements a session on an AIA. The AIA session implements
//              the IManagement interface via a Proxy. Each session has its
//              own Context and and event handler.
// Comments: The m_uAIAProxy is a proxy to the uAIA interface of the AIA. It
//           points through the session owners BaseProxy. The  AIAProxy is
//           typed as an AALProxy<uAIA,iiduAIA>. The constructor used here
//           implements an implicit type conversion from AALBaseProxy to
//           AALProxy<uSIS,iisuAIA>.
//           the m_returnAddress is a uidrvMessage::uidrvMessageRoute which
//           hold the session owners AAL default return event information.
//           I.e., The event handler and Context of the AFU owner.  This
//           information is used by the AIA to route return messages from the
//           lower level kernel framework to the upper layer user framework.
//=============================================================================
class UAIA_API uAIASession: public CAASBase, public IuAIASession
{
public:
   uAIASession(IBase                *pOwnerBase,
               CAIA                 &rAIA,
               btApplicationContext  Context,
               btEventHandler        DefaultEventHandler,
               ServiceBase          *pServiceBase);

   CAIA & ruAIA()      { return m_ruAIA; }
   btBool IsOK() const { return m_bIsOK; }

   //------------
   // IManagement
   //------------
   void Bind(btObjectType         Handle,
             NamedValueSet        nvsOptions,
             TransactionID const &tid);

   void UnBind(btObjectType         Handle,
               TransactionID const &tid);

   void ReBind(IAFUDev             *DevObject,
               NamedValueSet        nvsOptions,
               TransactionID const &tid);


   //-------------------------------------------------------------
   // Name: OwnerMessageRoute
   // Description: Returns the message route object. The message
   //              route object contains information required to
   //              forward AAL events to the owner of the session.
   //              It is primarily used to assist the interface
   //              adapting between the native driver interface
   //              and the AAL event delivery interface.
   //------------------------------------------------------------
   uidrvMessageRoute const & OwnerMessageRoute() const { return m_OwnerReturnAddress; }

   //-------------------------------------------------------------
   // Name: MessageHandler
   // Description: Session event handler. This handler performs
   //              intermediate processing of low-level events and
   //              the events exposed to the Host AFU.
   //
   //              This handler maybe called by the low level
   //              the AIA marshaller. via a UIDriverClientEvent.
   //              This type of event basically wraps a low level
   //              message into an AAL Event type.
   //              The AFUDev may also generate  events  to the
   //              session. For example before binding is complete
   //              the AFUdev is initialized.  The bind complete
   //              cannot be sent to the end application until
   //              the Initialize operation has completed.
   //--------------------------------------------------------------
   static void MessageHandler(AAL::IEvent const &theEvent);

   ServiceBase  &AALService() const { return *m_pServiceBase; }

   // Convenience function to Queue an event to anyone
   void QueueAASEvent(btEventHandler Eventhandler, CAALEvent *pEvent);


protected:
   btBool                          m_bIsOK;
   IBase                          *m_Owner;
   IBase                          &m_BaseProxy;
   CAIA                           &m_ruAIA;
   uidrvMessageRoute               m_OwnerReturnAddress;
   uidrvMessageRoute               m_returnAddress;
   btInt                           m_refcount;
   UIDriverClient_uidrvMarshaler_t m_AIAMarshaller;
   ServiceBase                    *m_pServiceBase;
};


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif // __AALSDK_UAIA_UAIASESSION_H__

