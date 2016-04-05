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
/// @file aalbus_session.c
/// @brief This module implements the functionality for creating/destroying
///        AAL bus session objects used for connections with user mode
/// @ingroup System
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
/// COMMENTS: 
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 02/14/2015     JG       Initial version of aalrmsserver created
//****************************************************************************
#include "aalbus_poll.h"
#include "aalbus_session.h"
#include "aalbus_events.h"

#define MODULE_FLAGS AALBUS_DBG_POLL


//=============================================================================
// Name: aalbus_createSession
// Description: Create and AAL Bus session object
// Interface: public
// Inputs: pid - Process ID of session owner
//         
// Returns: Pointer to session.
//          NULL if failure
// Comments: 
//=============================================================================
struct aalbus_session *aalbus_createSession(btPID  pid)
{
   
   PTRACEIN;

	// Allocate the session
	struct aalbus_session * psession = (struct aalbus_session *)kosal_kmalloc(sizeof(struct aalbus_session));
	if (unlikely(psession == NULL)){
		PDEBUG(": failed to malloc session object\n");
		return NULL;
	}

	// Initialize session's lists, queues and sync objects
	kosal_list_init(&psession->m_sessions);

	// Initialize queues
	kosal_init_waitqueue_head(&psession->m_pollobj);
	aal_queue_init(&psession->m_eventq);

	kosal_mutex_init(&psession->m_sem);

	// Record the owning process
	psession->m_pid = pid;

	return psession;
}


//=============================================================================
// Name: aalbus_destroySession
// Description: Destroy an AAL Bus session object
// Interface: public
// Inputs: psess - session
//         
// Returns: none
// Comments: 
//=============================================================================
void aalbus_destroySession(struct aalbus_session *psess)
{

   PTRACEIN;

   int ret = 0;
   struct aal_q_item *pqitem;

   // Flush the event Queue
   PNOTICE( "Flushing event queue\n" );
   while( !_aal_q_empty( &psess->m_eventq ) ) {

      // Get the message
      pqitem = _aal_q_dequeue( &psess->m_eventq );
      if( pqitem == NULL ) {
         DPRINTF( UIDRV_DBG_IOCTL, ": Invalid or corrupted request on flush\n" );
         continue;
      }

      DPRINTF( UIDRV_DBG_IOCTL, ": Not Empty\n" );
      // Switch on message type
      switch( pqitem->m_id ) {
         // Bind Complete
         case evtid_bus_ConfigUpdate:  {
            aalbus_config_update_event_destroy( qip_to_updateevent( pqitem ) );
         }
         break;
         default:
            PERR( "Encountered unknown event (%x)  while flushing - Potential leak leak\n", pqitem->m_id );

      } // switch (pqitem->m_id)

   }

   // Delete 
   kosal_kfree(psess, sizeof(struct aalbus_session));

   return;
}
