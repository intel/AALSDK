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
/// @file aalbus_poll.c
/// @brief This module implements the functionality used to enable a generic 
///        event poll mechanism in Windows.
/// @ingroup System
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
/// COMMENTS: 
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 02/6/2015      JG       Initial version of aalrmsserver created
//****************************************************************************
#include "aalbus_poll.h"
#include "aalbus_session.h"
#define MODULE_FLAGS AALBUS_DBG_POLL

#if 1
//=============================================================================
// Name: EvtRequestCancelPoll
// Description: Callback when Poll request is canceled
// Interface: public
// Inputs: Request - Request associated with POLL
// Comments: 
//=============================================================================
void EvtRequestCancelPoll(WDFREQUEST Request)
{
	struct aalbus_session * psess = NULL;
	PTRACEIN;

	PVERBOSE("Poll Canceled %p\n", Request);
	WdfRequestComplete(Request, STATUS_CANCELLED);
	
	psess = (struct aalbus_session *)WdfRequestGetInformation(Request);

	ASSERT(NULL != psess);
	psess->m_pollobj = NULL;

}


//=============================================================================
// Name: aalbus_poll
// Description: Implements a Poll with asynchronous notifications
// Interface: public
// Inputs: waitq - Object used for signaling
//         peventq = Event queue
// Returns: true - waitq is not signalled (waiting for an event).
//          false - waitq is signaled (events are available)
// Comments: User application waits on a waitq (IRP) for an event to be 
//           available from the eventq. If this function returns TRUE the
//           caller should save the waitq object
//=============================================================================
btBool aalbus_poll( kosal_poll_object      waitq, 
                    aal_queue_t           *peventq)
{

   ASSERT(NULL != peventq);
   PTRACEIN;
   
   EVT_WDF_REQUEST_CANCEL EvtRequestCancelPoll;

   btBool ret = true;
   PDEBUG("Poll waitq (WDFRequest) %p\n", waitq);


   // Mark the IRP as cancelable
   WdfRequestMarkCancelable(waitq, EvtRequestCancelPoll);

   // If there is a request on the queue wakeup sleeper
   if (_aal_q_empty(peventq)) {
      PDEBUG("Poll Queue is empty\n");
      PDEBUG("Poll Waiting\n");
      ret = true;
   }
   else{
      PDEBUG("Poll Queue not empty waking immediatly\n");
      kosal_wake_up_interruptible(&waitq);
	   ret = false;
   }

   return ret;
}
#endif
