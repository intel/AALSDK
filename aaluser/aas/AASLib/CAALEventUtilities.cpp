// Copyright(c) 2008-2016, Intel Corporation
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
/// @file CAALEventUtilities.cpp
/// @brief EventDelivery Utilities
/// @ingroup EventUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/12/2008     HM       Initial version started
/// 12/14/2008     HM       Allocate and Free done
/// 01/04/2009     HM       Updated Copyright
/// 03/06/2009     HM       Cleaned up PrintExceptionDescription() to have the
///                            error message not split, easily
/// 01/12/2010     HM       Distinguished between Exception Event and Exception
///                            Transaction Event based upon customer feedback
/// 06/29/2011     JG       Added WrapTransactionID@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/utils/AALEventUtilities.h"
#include "aalsdk/CAALEvent.h"
#include "aalsdk/CAALBase.h"
#include "aalsdk/INTCDefs.h"
#include "aalsdk/AALLogger.h"

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name:          WrapTransactionID
// Description:   Wraps a copy of a TransactionID into the context of another.
//                This is very useful when implementing the common design
//                pattern of creating nested transactions. I.e., a transaction
//                where other asynchronous transactions are executed.
//                The returned TransactionID can be used for the nested
//                Transactions.  The original TransactionID will be carried as
//                the context.  The original "Wrapped" TransactionID can be
//                extracted from the final event of the nested Transaction by
//                using UnWrapTransactionIDFromEvent.
//
// Input:        rTranID - reference to the TransactionID to wrap.
// Comments:      SEE UnWrapTransactionIDFromEvent()
//=============================================================================
TransactionID WrapTransactionID(const TransactionID &rTranID)
{
   return TransactionID(reinterpret_cast<btApplicationContext>(new TransactionID(rTranID)));
}


//=============================================================================
// Name:          UnWrapAndReThrow
// Description:   Re-throw an Event to an object's owner
// Input:         Parameters
//                   'This' pointer to the object, the event will be rethrown
//                      to 'This' object's owner (typically). 'This' MUST have
//                      been obtained by dynamic_ptr(iidBase,oldThis)
//                   The Event to be rethrown
//                   EventDispatcher & EventHandler
//                The TransactionID in the Event is expected to be wrapped,
//                   i.e. its context is a pointer to an allocated copy of the
//                   TransactionID for the rethrow target.
// Comments:      The Original Transaction id would be wrapped by a statement
//                   like this: TransactionID tid (new TransactionID(rTranID));
//                SEE UnWrapTransactionIDFromEvent(), below
//                   where rTranID is the original TranID, and tid is what is
//                   passed in the event
//=============================================================================
void UnWrapAndReThrow(IBase              *This,
                      const IEvent       &theEvent,
                      IRuntime           *pDispatcher,
                      btEventHandler      Handler)
{
   ITransactionEvent &rTransEvt   = dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent);
   TransactionID     *pOrigTranID = reinterpret_cast<TransactionID *>(rTransEvt.TranID().Context());

   // ReThrow the transaction
   ReThrow(This, theEvent, pDispatcher, Handler, pOrigTranID);

   // Delete the wrapper
   if ( NULL != pOrigTranID ) {
      AAL_VERBOSE(LM_EDS, "UnWrapAndReThrow:OrigTranID is " << *pOrigTranID);

      // We're deleting the allocated copy of the original TransactionID, so clear the pointer
      //  from the Event's TransactionID's Context to prevent a future invalid dereference.

      TransactionID tid(rTransEvt.TranID());
      tid.Context(NULL);
      rTransEvt.SetTranID(tid);

      delete pOrigTranID;
   }
}

//=============================================================================
// Name:          UnWrapTransactionIDFromEvent
// Description:   UnWrap the inner TransactionID from the outer one, and return it
// Input:         The TransactionID in the Event is expected to be wrapped,
//                   i.e. its context is a pointer to an allocated copy of the
//                   TransactionID for the rethrow target.
// Returns:       This function returns a copy of rTranID
// Comments:      The Original Transaction id would be wrapped by a statement
//                   like this:
//                      TransactionID tid( static_cast<btApplicationContext>(new TransactionID( rTranID)));
//                   where rTranID is the original TranID, and tid is what is
//                   passed in the event
//                Note that this comment originally specified the following code:
//                   TransactionID tid (new TransactionID(rTranID));
//=============================================================================
TransactionID UnWrapTransactionIDFromEvent(const IEvent &theEvent, btBool bDeleteOrig)
{
   // Retrieve the original TransactionID
   ITransactionEvent &rTransEvt   = dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent);
   TransactionID     *pOrigTranID = reinterpret_cast<TransactionID *>(rTransEvt.TranID().Context());
   TransactionID      temp        = *pOrigTranID; // copy 1

   if ( ( NULL != pOrigTranID ) && bDeleteOrig ) {
      AAL_VERBOSE(LM_EDS, "UnWrapTransactionIDFromEvent:OrigTranID is " << *pOrigTranID);

      // We're deleting the allocated copy of the original TransactionID, so clear the pointer
      //  from the Event's TransactionID's Context to prevent a future invalid dereference.
      TransactionID tid(rTransEvt.TranID());
      tid.Context(NULL);
      rTransEvt.SetTranID(tid);

      // Delete the allocated copy of the original TransactionID.
      delete pOrigTranID;   // should be valid, but you never know
   }

   return temp; // copy 2, then copy into the target
}

//=============================================================================
// Name:          ReThrow
// Description:   Re-throw an Event to an object's owner
// Input:         Parameters
//                   'This' pointer to the object, the event will be rethrown
//                      to 'This' object's owner (typically). 'This' MUST have
//                      been obtained by dynamic_ptr(iidBase,oldThis)
//                   The Event to be rethrown
//                   EventDispatcher & EventHandler
//                The TransactionID is copied into the Event
// Comments:      The Original Transaction id would be wrapped by a statement
//                   like this: TransactionID tid (new TransactionID(rTranID));
//                   where rTranID is the original TranID, and tid is what is
//                   passed in the event
//=============================================================================
void ReThrow(IBase               *This,
             const IEvent        &theEvent,
             IRuntime            *pDispatcher,
             btEventHandler       Handler,
             const TransactionID *pTranID)
{
   // Get theEvent as a CAALEvent so can increment the reference count and change the object to the AFU (This)
   CAALEvent& rCEvent = dynamic_ref<CAALEvent>(iidCEvent, (const_cast<IEvent &>(theEvent)));

   // DEBUG CODE
   //         IEvent* pIEvent = const_cast<IEvent *>(&theEvent);
   //         AAL_VERBOSE(LM_AFU, "IEvent being rethrown is " << pIEvent << endl);

   // increment the event because we are going to rethrow it
   rCEvent.IncRef();

   // Set the object to AFU for rethrow
   rCEvent.SetObject( This);

   // Put the TransactionID into the Event
   dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).SetTranID(*pTranID);

   //Post to object's parent listen
   rCEvent.setHandler(Handler);

   pDispatcher->schedDispatchable(dynamic_ptr<CAALEvent>(iidCEvent, theEvent));
}


//=============================================================================
// Name:          PrintExceptionDescription
// Description:   External function to print information in Exception and
//                   ExceptionTransaction Events
//=============================================================================
void PrintExceptionDescription(IEvent const &theEvent)
{
   // The Has() method is used here to show how an object can be examined to determine if it supports a particular
   // interface. Exceptions are either derived from ExceptionEvents or ExceptionTransactionEvents
   if ( theEvent.Has(iidExEvent) ) {
      //ExceptionEvent

      AAL_ERR(LM_EDS, "\nEXCEPTION EVENT THROWN:  " <<
                      dynamic_ref<IExceptionEvent>(iidExEvent, theEvent).Description() << endl);

   } else if ( theEvent.Has(iidExTranEvent) ) {
      //ExceptionTransaction

      AAL_ERR(LM_EDS, "\nEXCEPTION TRANSACTION EVENT THROWN:  " <<
                      dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, theEvent).Description() << endl);

   } else if (AAL_IS_EXCEPTION(theEvent.SubClassID())) {
      IExceptionTransactionEvent *trycast_p;

      trycast_p = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, theEvent);
      if (NULL != trycast_p) {
         AAL_ERR(LM_EDS, "\nUNKNOWN EXCEPTION TRANSACTION EVENT THROWN:  " << trycast_p->Description() << endl);
      } else {
         AAL_ERR(LM_EDS, "\nUNKNOWN EXCEPTION EVENT THROWN:  subclass ID " << hex << theEvent.SubClassID() << endl);
      }

   } else {
      // theEvent not an exception.
      ASSERT(false);
   }
}

END_NAMESPACE(AAL)

