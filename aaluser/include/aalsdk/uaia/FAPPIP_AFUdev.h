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
/// @file FAPPIP_AFUdev.h
/// @brief Definitions for the FAP PIP AFU Device Class.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/10/2008     HM/JG    Added MapWSID
/// 12/14/2008     HM       Changed MapWSID signature from char* to void*
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2009     HM       Add m_pManagement to CAFUDev, and made Destroy()
///                            really work properly, UnBinding and then
///                            deleting itself
/// 06/22/2009     JG       Massive changes to support new proxy mechanism and
///                            to fix build dependencies that required external
///                            linking to the module, breaking plug-in model.
/// 12/28/2009     JG       Removed implementation and moved into
///                         uAIA/AFUdev.cpp
/// 12/28/2009     JG       Support for CSR Map
/// 02/05/2010     JG       Added workspace cleanup in Destroy
/// 02/11/2010     JG       Added support for glib version 4.4
/// 09/02/2011     JG       Removed Proxy@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_FAPPIP_AFUDEV_H__
#define __AALSDK_UAIA_FAPPIP_AFUDEV_H__
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


/// @todo Document CAFUDev

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


   //=============================================================================
   // Name: CAFUDev
   // Description: The AFUDev object is the proxy to the physical AFU engine
   //              implementation on a device. It abstracts a session between the
   //              Host AFU and the device through the AIA.
   //
   //              The CAFUDev send messages to the device through ITransaction
   //              objects.
   //
   //              The MessageRoute object of the AIASession contains the default
   //              callback and the context when the device send message back
   //              up through the AIA.
   //  Inputs: handle   - AAL handle to device this object proxies
   //          pSession - Pointer to the  Universal AIA interface (proxy)
   // Comments: The uidrvMessage::uidrvMessageRoute contains the default event
   //           handler and context for this AIA session and is extracted from the
   //           AIA session through the OwnerMessageRoute() method and cached in
   //           object.
   //=============================================================================
   class UAIA_API CAFUDev : public IAFUDev, public CAALBase
   {
      struct wrapper
      {
         wrapper(CAFUDev *pdev, btWSID wsid, TransactionID const &rtid, btUnsignedInt *pne) :
            pafudev(pdev),
            id(wsid),
            tid(rtid),
            pnumEvents(pne)
         {}

         CAFUDev       *pafudev;
         btWSID         id;
         TransactionID  tid;
         btUnsignedInt *pnumEvents;
      };

   public:

      // Constructor
      CAFUDev(void         *handle,
              IuAIASession *pSession);

      virtual ~CAFUDev();

      // Accessor for the event handler for the Dev owner
      btEventHandler Handler();

      // Initialize the object
      void Initialize(struct aalui_extbindargs *extBindParmsp, TransactionID const &tid);

      // Send a message to the device
      btBool SendTransaction(IAFUTransaction *pAFUmessage, TransactionID const &rtid);

      // This provides a Direct Mapping from user space to the AFU's registers - very fast
      // NOT YET IMPLEMENTED - PLACEHOLDER, currently returns NULL
      IAFUCSRMap * GetCSRMAP();

      static void _CSRMapHandler(IEvent const &theEvent);
      void CSRMapHandler(IEvent        const &theEvent,
						 btWSID               id,
                         TransactionID const &rtid,
                         btUnsignedInt       *pnumEvents);

      // Wrapper method around map specific CSR accessor/mutators
      btBool atomicSetCSR(btCSROffset CSR, btCSRValue Value)
      {
         return ( (NULL == _atomicSetCSR) ? false : (this->*_atomicSetCSR)(CSR, Value) );
      }

      btBool atomicGetCSR(btCSROffset CSR, btCSRValue *Value)
      {
         return ( (NULL == _atomicGetCSR) ? false : (this->*_atomicGetCSR)(CSR, Value) );
      }

      //Memmap methods
      btBool MapWSID(btWSSize Size, btWSID wsid, btVirtAddr *pRet);

      void UnMapWSID(btVirtAddr ptr, btWSSize Size, btWSID wsid);

      // CSR methods

      // Workspace methods, export the WSM itself rather than wrapping the interfaces. Not needed.
      WorkSpaceMapper & WSM();

      // Destroy
      void Destroy(TransactionID const &TranID);

      void FreeAllWS();

      // Accessor for Handle
      btObjectType Handle();

   protected:
      btBool (CAFUDev::*_atomicSetCSR)(btCSROffset CSR, btCSRValue  Value);
      btBool (CAFUDev::*_atomicGetCSR)(btCSROffset CSR, btCSRValue *Value);

      btBool atomicSetCSR_64x128B(btCSROffset CSR, btCSRValue  Value);
      btBool atomicGetCSR_64x128B(btCSROffset CSR, btCSRValue *Value);

      btBool atomicSetCSR_32x4B(btCSROffset CSR, btCSRValue  Value);
      btBool atomicGetCSR_32x4B(btCSROffset CSR, btCSRValue *Value);

   protected:
         btObjectType                      m_Handle;
         struct aalui_extbindargs          m_extBindParms;
         uidrvMessageRoute                 m_returnAddress;
         WorkSpaceMapper                   m_WSM;
         IuAIASession                     *m_pSession;
         UIDriverClient_uidrvMarshaler_t   m_AIAMarshaller;
         btVirtAddr                        m_csrwritemap;
         btUnsigned32bitInt                m_csrwritesize;
         btUnsigned32bitInt                m_csrwrite_item_size;
         btVirtAddr                        m_csrreadmap;
         btUnsigned32bitInt                m_csrreadsize;
         btUnsigned32bitInt                m_csrread_item_size;
   }; // class CAFUDev


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif //__AALSDK_UAIA_FAPPIP_AFUDEV_H__

