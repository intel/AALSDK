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
/// @file AALuAIA_Messaging.h
/// @brief Defines the object wrapper for the AAL Universal Device Driver.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 1/22/2013      TSW      UIDriverClient::msgPayload{} -> UIDriverClient_msgPayload{},
///                          UIDriverClient::uidrvManip{} -> UIDriverClient_uidrvManip{},
///                          uidrvMarshaler_t -> UIDriverClient_uidrvMarshaler_t.@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_AALUAIA_MESSAGING_H__
#define __AALSDK_UAIA_AALUAIA_MESSAGING_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/uaia/uidrvMessage.h> // uidrvMessageRoute
#include <aalsdk/uaia/AIA.h>          // IAFUTransaction

#include <aalsdk/kernel/aalui.h>      // uid_msgIDs_e


/// @todo Document UIDriverClient_msgPayload and related.


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)

//==========================================================================
// Name: UIDriverClient_msgPayload
// Description: Utility wrapper class for payloads that need to be deleted
//              when done with or not.
//==========================================================================

class UAIA_API UIDriverClient_msgPayload
{
public:
   UIDriverClient_msgPayload(btVirtAddr ptr, btWSSize size);
   UIDriverClient_msgPayload(UIDriverClient_msgPayload const &rOther);
   UIDriverClient_msgPayload(btWSSize size);
   virtual ~UIDriverClient_msgPayload();

   btVirtAddr ptr() { return m_ptr;  }
   btWSSize  size() { return m_size; }

protected:
   btBool     m_free;
   btVirtAddr m_ptr;
   btWSSize   m_size;

private:
   // Strictly not allowed
   UIDriverClient_msgPayload() {}
   UIDriverClient_msgPayload * operator * () { return this; }
   UIDriverClient_msgPayload   operator = (UIDriverClient_msgPayload const &rOther) { return *this; }
}; // UIDriverClient_msgPayload{}


class UIDriverClient;
typedef UIDriverClient & (*UIDriverClient_uidrvMarshaler_t)(UIDriverClient & ,
                                                            uid_msgIDs_e ,
                                                            UIDriverClient_msgPayload ,
                                                            stTransactionID_t const * ,
                                                            uidrvMessageRoute * ,
                                                            btObjectType );

//
// Structure for defining a manipulator with a single argument
//
struct UAIA_API UIDriverClient_uidrvManip
{
   //Constructor - Initialize the struct
   UIDriverClient_uidrvManip(UIDriverClient_uidrvMarshaler_t fop,
                             uid_msgIDs_e                    cmd,
                             UIDriverClient_msgPayload       payload,
                             TransactionID const            &tranID,
                             uidrvMessageRoute              *mgsRoutep,
                             btObjectType                    dev);

   virtual ~UIDriverClient_uidrvManip();

   UIDriverClient_uidrvMarshaler_t m_fop;
   uid_msgIDs_e                    m_cmd;
   UIDriverClient_msgPayload       m_payload;
   stTransactionID_t               m_tid;
   uidrvMessageRoute              *m_mgsRoutep;
   btObjectType                    m_DevObject;
}; // UIDriverClient_uidrvManip{}


//==========================================================================
//==========================================================================
//                         UI Driver Function Objects
//
// These methods construct a function object which
// is then passed into the << operator as an argument. The function object
// is then used to execute the multi-argument method
//==========================================================================
//==========================================================================

//==========================================================================
// Name: BindDevice
// Description:
//==========================================================================
inline UIDriverClient_uidrvManip BindDevice(UIDriverClient_uidrvMarshaler_t fop,
                                            btObjectType                    Handle,
                                            NamedValueSet                   nvsManifest,
                                            TransactionID const            &tid,
                                            uidrvMessageRoute              *mgsRoutep)
{
   // Marshal the nvs out to a buffer for transport
   std::string stManifest = StdStringFromNamedValueSet(nvsManifest);
   btWSSize           len = (btWSSize)stManifest.size() + 1;

   UIDriverClient_msgPayload Cpayload(len);

   BufFromString(Cpayload.ptr(), stManifest);

   // Return the function object containing the method and its args
   return UIDriverClient_uidrvManip(fop,
                                    reqid_UID_Bind,
                                    Cpayload,
                                    tid,
                                    mgsRoutep,
                                    Handle);
}

//==========================================================================
// Name: UnBindDevice
// Description:
//==========================================================================
inline UIDriverClient_uidrvManip UnBindDevice(UIDriverClient_uidrvMarshaler_t fop,
                                              btObjectType                    Handle,
                                              TransactionID const            &tid,
                                              uidrvMessageRoute              *mgsRoutep)
{
   // Return the function object containing the method and its args
   return UIDriverClient_uidrvManip(fop,
                                    reqid_UID_UnBind,
                                    UIDriverClient_msgPayload(0),
                                    tid,
                                    mgsRoutep,
                                    Handle);
}

//class CAFUDev;

//==========================================================================
// Name:          TranIDWrapper
// Description:   Standard way to wrap a tranID in another TranID
//==========================================================================
struct UAIA_API TranIDWrapper
{
   btObjectType  Context;    // Context of wrapping TranID, now occupied by pointer to this
   TransactionID origTranID; // Original TranID
   btObjectType  pCAFUDev;   // really a CAFUDev*

   TranIDWrapper(btObjectType argContext, const TransactionID &rTranID, btObjectType pCAFUDev_arg);
};

//==========================================================================
// Name: AFUTransaction
// Description: This method wraps a message pointed to by the IAFUtran and
//              turns it into a manipulator (of sorts).  That simply means
//              it can be passed to the UIDriverClient::operator <<.
// Inputs: fop - pointer message marshaller method
//         pDev - pointer to the CAFUDev of the device the message is
//                destined for.
//         Handle - AAL device handle associated with the device TODO should come from CAFUDev
//         IAFUTran - Pointer to message to send.
//         tid - Transaction ID for this transaction
//         mgsRoutep - pointer to return address TODO should come from CAFUDev
// Comments: The AFUTransaction encapsulates all of the information
//           necessary to marshal the message and return any responses.
//           NOTE: Some of the information passed in the constructor such
//           Handle and mesgRoutep could be extracted from the CAFUDev and
//           should in a future release.
//==========================================================================
inline UIDriverClient_uidrvManip AFUTransaction(UIDriverClient_uidrvMarshaler_t fop,
                                                btObjectType                    pDev, // CAFUDev *pDev,
                                                btObjectType                    Handle,
                                                IAFUTransaction                *IAFUtran,
                                                TransactionID const            &tid,
                                                uidrvMessageRoute              *mgsRoutep)
{
   //
   // The msgPayload wrapper is a smart container for the message payload buffer being
   //  sent by the requester.  Its purpose is primarily to enable the correct garbage
   //  collection to to occur at the end of the messages life cycle
   //  (e.g., paylod clean-up).
   UIDriverClient_msgPayload Cpayload(IAFUtran->GetPayloadPtr(), IAFUtran->GetSize());

   /*
    * ADD AFUDEV TO WRAPPER
    *
    * whole point of this is that if there is a TransactionID in the IAFUTran, then it contains
    * an event handler that has to be vectored to, and we have to tunnel the regular tid
    *
    * transactionID wrapping explanation:
    *
    *    tidWrapper is the Transaction ID that will end up carrying the information of both
    *       TransactionID's passed in. It will carry 'tid' embedded in a struct TranIDWrapper.origTranID member.
    *       It will carry the IAFUTran's TransactionID in its own members (e.g. EventDelivery, filter, and
    *       intID), and its context in the Context field of the TranIDWrapper.
    *
    *    tidWrapper.Context---->TranIDWrapper
    *           |                  |->Context is original context from IAFUTran->TranID, now being used to carry TranIDWrapper
    *           |                  |->origTranID is tid from user
    *           |->.other fields are from IAFUTran->TranID
    *
    * To unwrap this:
    * 1) tidWrapper contains the vectoring components of IAFUTran->TranID, and so ensures
    *    that the correct EventHandler is called
    * 2) At the destination, the Context of the tidWrapper will not be the original
    *    context. Rather, the original IAFUTran->TranID's context will be obtained by
    *    using the tidWrapper Context() as a pointer to a TranIDWrapper, the Context field
    *    of which will be the original IAFUTran->TranID.Context()
    * 3) At the destination, the original tid is in the TranIDWrapper's origTranID field
    *
    * See example in WkSp_MappingEventHandlerObject::WkSp_Allocate_Mapping_EventHandler
    */
   TransactionID tidWrapper(tid);

   TransactionID *pAFUtid = IAFUtran->GetTranIDPtr();
   if ( pAFUtid ) {                                   // Only wrap if something to wrap
      tidWrapper = *pAFUtid;                          // initialize tidWrapper to IAFUTran->TranID
      tidWrapper.Context(new struct TranIDWrapper(pAFUtid->Context(), tid, pDev));
   }

   // Return the function object containing the method and its args
   return UIDriverClient_uidrvManip(fop,
                                    IAFUtran->GetCommand(),
                                    Cpayload,
                                    tidWrapper,
                                    mgsRoutep,
                                    Handle);
}


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_UAIA_AALUAIA_MESSAGING_H__

