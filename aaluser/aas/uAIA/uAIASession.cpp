// Copyright (c) 2007-2015, Intel Corporation
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
/// @file uAIASession.cpp
/// @brief Resolves recursive import/export dependency between AASLib and uAIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 1/22/2013      TSW      Separating out UIDriverClient::msgPayload{} and
///                          UIDriverClient::uidrvManip to resolve recursive
///                          dependency resulting from AASLib on these.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/uaia/uAIASession.h"
#include "aalsdk/uaia/uAIA.h"

BEGIN_NAMESPACE(AAL)

IuAIASession::~IuAIASession() {}

IBindAFUDevEvent::~IBindAFUDevEvent() {}

CBindDevEvent::CBindDevEvent(IBase *pObject,
                             CAFUDev *pdev,
                             TransactionID const &TransID) :
   CTransactionEvent(pObject,TransID),
   m_pdev(pdev)
{
   SetSubClassInterface(tranevtBindAFUDevEvent, dynamic_cast<IBindAFUDevEvent *>(this));
}

IAFUDev *CBindDevEvent::pAFUDev() const { return dynamic_cast<IAFUDev *>(m_pdev); }



uAIASession::uAIASession(IBase                *pOwnerBase,
                         CAIA                 &rAIA,
                         btApplicationContext  Context,
                         btEventHandler        DefaultEventHandler,
                         ServiceBase          *pServiceBase) :
   CAASBase(Context),
   m_bIsOK(false),
   m_BaseProxy(rAIA),
   m_ruAIA(rAIA),
   m_OwnerReturnAddress(dynamic_ptr<IBase>(iidBase, &rAIA), DefaultEventHandler, Context),
   m_returnAddress(dynamic_cast<IBase *>(this), uAIASession::MessageHandler, this),
   m_pServiceBase(pServiceBase)
{
   // Get the message marshaller  (sets m_AIAMarshaller= pAIAProxy()->GetMarshaller()
   //   with an if not. See proxy accesssors
   m_AIAMarshaller = ruAIA().GetMarshaller();

   m_bIsOK = true;
}

void uAIASession::Bind(btObjectType         Handle,
                       NamedValueSet        nvsOptions,
                       TransactionID const &tid)
{
   m_ruAIA.SendMessage(
                        BindDevice(m_AIAMarshaller,
                                                  Handle,
                                                  nvsOptions,
                                                  tid,
                                                  &m_returnAddress)
                      );
}

void uAIASession::UnBind(btObjectType         Handle,
                         TransactionID const &tid)
{
   m_ruAIA.SendMessage( 
                        UnBindDevice(m_AIAMarshaller,
                                                    Handle,
                                                    tid,
                                                    &m_returnAddress)
                      );
}

void uAIASession::ReBind(IAFUDev             *DevObject,
                         NamedValueSet        nvsOptions,
                         TransactionID const &tid)
{ /* TODO */ }

void uAIASession::MessageHandler(IEvent const &theEvent)
{
   class EventWrapper
   {
   public:
      EventWrapper(uAIASession  *p,
                   CAFUDev      *dev,
                   TransactionID t) :
         usessp(p),
         pdev(dev),
         tid(t)
       {}

      uAIASession  *usessp;
      CAFUDev      *pdev;
      TransactionID tid;
   };

   // TODO: Check for exception

   // SWitch on the top level event ID
   btIID iid = theEvent.SubClassID();

   switch ( theEvent.SubClassID() ) {

      // The event was a driver message
      case tranevtUIDriverClientEvent : {

         IUIDriverClientEvent &revt = dynamic_ref<IUIDriverClientEvent>(tranevtUIDriverClientEvent, theEvent);

         // Get this address from the context of the return address

         uAIASession *This = static_cast<uAIASession*>(revt.msgRoute().Context());

         uid_msgIDs_e foo = revt.MessageID();

         switch ( revt.MessageID() ) {
            case rspid_UID_BindComplete : {
   //            AAL_DEBUG(LM_UAIA, "uAIAMangement::MessageHandler sees rspid_UID_BindComplete\n");

               // Get the uidrvMessage from the AAL event
               struct aalui_extbindargs * extBindParmsp = reinterpret_cast<aalui_extbindargs *>(revt.Payload());

               // The payload of the BindComplete Driver Message has the extended bin parameters
//              reinterpret_cast<struct aalui_extbindargs *>(pMessage->payload());

               CAFUDev * pdev = new CAFUDev(revt.DevHandle(),    // Low level device handle
                                            This);
               if ( ( NULL == pdev ) || !pdev->IsOK() ) {
   //               AAL_ERR(LM_UAIA, "uAIAMangement::MessageHandler - CAFUDev creation failed\n");
                  // TODO: Try to do better here. Generate exception event
               }
               // Initialize the AFUdev and carry forward the original event wrapped in the Transaction ID
               EventWrapper *pevtw = new EventWrapper(This, pdev, revt.msgTranID() );

               // Make sure the completion of the Initialize comes here and not to the owner
               TransactionID tid(pevtw, MessageHandler,true);
               pdev->Initialize( extBindParmsp, tid );

            } break;

            case rspid_UID_UnbindComplete : {

   //            AAL_DEBUG(LM_UAIA, "uAIAMangement::MessageHandler sees rspid_UID_UnbindComplete\n");

               // and generate the event
               This->QueueAASEvent(This->OwnerMessageRoute().Handler(),
                                    new CTransactionEvent(static_cast<IBase *>(This),
                                                         tranevtUnBindAFUDevEvent,
                                                         TransactionID(revt.msgTranID())));
            } break;

            default :
   //            AAL_DEBUG(LM_UAIA, "uAIAMangement::MessageHandler sees UNKNOWN MessageID()\n");
            break;
         }
      } break; // End case tranevtUIDriverClientEvent

      // The event was a AFUDev event
      case exttranevtInitAFUDevEvent : {

         IExceptionTransactionEvent *pTevt = dynamic_ptr<IExceptionTransactionEvent>(iidTranEvent, theEvent);
         EventWrapper *clientevt = static_cast<EventWrapper *>(pTevt->TranID().Context());

         // The Session pointer
         uAIASession *This = clientevt->usessp;

         // Bind is now complete
         This->QueueAASEvent(This->OwnerMessageRoute().Handler(),
                              new CExceptionTransactionEvent(static_cast<IBase *>(This),
                                                            exttranevtBindAFUDevEvent,
                                                            TransactionID(clientevt->tid),
                                                            pTevt->ExceptionNumber(),
                                                            pTevt->Reason(),
                                                            pTevt->Description()
                                                            )
                           );
         delete clientevt;
      } break;

      case tranevtInitAFUDevEvent : {

         // Get the wrapper from the events Transaction ID
         ITransactionEvent *pTevt = dynamic_ptr<ITransactionEvent>(iidTranEvent, theEvent);
         EventWrapper *clientevt = static_cast<EventWrapper*>(pTevt->TranID().Context());

         // The Session pointer
         uAIASession *This = clientevt->usessp;

         // Bind is now complete
         This->QueueAASEvent(This->OwnerMessageRoute().Handler(),
                             new CBindDevEvent( static_cast<IBase *>(This),
                                                clientevt->pdev,
                                                TransactionID(clientevt->tid)
                                              )
                             );
         delete clientevt;
      } break;

      default : {
         // Unexpected event
      } break;
   } // switch ( theEvent.SubClassID() )

} // uAIASession::MessageHandler()

void uAIASession::QueueAASEvent(btEventHandler Eventhandler, CAALEvent *pEvent)
{

   pEvent->setHandler(Eventhandler);
   AALService().getRuntime()->schedDispatchable(pEvent);
}

END_NAMESPACE(AAL)


