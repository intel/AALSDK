// Copyright (c) 2008-2015, Intel Corporation
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
/// @file AALEventUtilities.h
/// @brief EventDelivery Utilities
/// @ingroup EventUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:      WHAT:
/// 12/12/2008     HM        Initial version started
/// 12/14/2008     HM       Allocate and Free done
/// 01/04/2009     HM       Updated Copyright
/// 06/29/2011     JG       Added event_to_tranevent and WrapTransactionID@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_AALEVENTUTILITIES_H__
#define __AALSDK_UTILS_AALEVENTUTILITIES_H__
#include <aalsdk/eds/AASEventDeliveryService.h>
#include <aalsdk/aas/_xlRuntimeServices.h>

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

/// @addtogroup EventUtils
/// @{

#define event_to_tranevent(e)  (dynamic_ref<ITransactionEvent>(iidTranEvent, e))

/// @brief Wraps a copy of a TransactionID into the context of another.
///
/// This is very useful when implementing the common design pattern of creating
/// nested transactions. I.e., a transaction where other asynchronous transactions
/// are executed.
///
/// The returned TransactionID can be used for the nested Transactions.  The original
/// TransactionID will be carried as the context.  The original "Wrapped" TransactionID
/// can be extracted from the final event of the nested Transaction by using
/// UnWrapTransactionIDFromEvent.
///
/// @param[in]  rTranID  The TransactionID to wrap.
///
/// @return The wrapped TransactionID object.
AASLIB_API TransactionID WrapTransactionID(const TransactionID &rTranID);

/// @brief UnWrap the inner TransactionID from the outer one, and return it.
///
/// The TransactionID in the Event is expected to be wrapped, i.e. its context is a
/// pointer to an allocated copy of the TransactionID for the rethrow target.
///
/// The Original Transaction id would be wrapped by a statement like this:
/// @code
/// TransactionID tid( new TransactionID( rTranID ) );@endcode
/// where rTranID is the original TranID, and tid is what is passed in the event.
///
/// @param[in]  theEvent     The event whose TransactionID is assumed to be wrapped.
/// @param[in]  bDeleteOrig  Signals whether to delete the allocated copy of the original TransactionID.
///
/// @return A copy of the original (wrapped) TransactionID.
AASLIB_API TransactionID UnWrapTransactionIDFromEvent(const IEvent &theEvent, btBool bDeleteOrig=true);

/// @brief Re-throw an Event to an object's owner.
///
/// The Original Transaction id would be wrapped by a statement like this:
/// @code
/// TransactionID tid( new TransactionID( rTranID ) );@endcode
/// where rTranID is the original TranID, and tid is what is passed in the event.
///
/// @param[in]  This  'This' pointer to the object, the event will be rethrown
///   to 'This' object's owner (typically). 'This' MUST have been obtained by
/// @code
/// dynamic_ptr(iidBase, oldThis)@endcode
/// @param[in]  theEvent     The Event to be rethrown.
/// @param[in]  pDispatcher  Pointer to Event Dispatcher interface.
/// @param[in]  Handler      Event handler pointer.
/// @param[in]  pTranID      Pointer to the TransactionID to be copied into the Event.
AASLIB_API void ReThrow(IBase               *This,
                        const IEvent        &theEvent,
                        AAL::XL::RT::IXLRuntimeServices    *pDispatcher,
                        btEventHandler       Handler,
                        const TransactionID *pTranID);

/// @brief Re-throw an Event to an object's owner.
///
/// The TransactionID in the Event is expected to be wrapped, i.e. its context
/// is a pointer to an allocated copy of the TransactionID for the rethrow target.
///
/// The Original Transaction id would be wrapped by a statement like this:
/// @code
/// TransactionID tid( new TransactionID( rTranID ) );@endcode
/// where rTranID is the original TranID, and tid is what is passed in the event.
///
/// @param[in]  This  'This' pointer to the object. the event will be rethrown
///   to 'This' object's owner (typically). 'This' MUST have been obtained by
/// @code
/// dynamic_ptr(iidBase, oldThis)@endcode
/// @param[in]  theEvent     The Event to be rethrown.
/// @param[in]  pDispatcher  Pointer to Event Dispatcher interface.
/// @param[in]  Handler      Event handler pointer.
AASLIB_API void UnWrapAndReThrow(IBase            *This,
                                 const IEvent     &theEvent,
                                 AAL::XL::RT::IXLRuntimeServices    *pDispatcher,
                                 btEventHandler    Handler);

/// @} group EventUtils

   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_UTILS_AALEVENTUTILITIES_H__

