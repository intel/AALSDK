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
/// @file aalbus_configgmon_msgs.c
/// @brief This OS independent module process AAL Bus configuration monitor
///        messages
/// @ingroup System
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 02/25/2015     JG       Initial version created
//****************************************************************************
#include "aalsdk/kernel/kosal.h"
#include "aalbus_session.h"

#include "aalsdk/kernel/aalbus_iconfigmonitor.h"
#include "aalbus_events.h"

#include "aalbus_poll.h"

#define MODULE_FLAGS AALRMS_DBG_MOD

//=============================================================================
// Name: aalbus_process_configmon_event
// Description: Process an event for the Configuration Monitor Service
// Interface: private
// Inputs: pqitem - message to process
//         presp  - response buffer
// Outputs: length of output buffer is copied to *Outbufsize.
//          return code: 0 == success
// Comments: Kernel event is destroyed
// Comments: This function processes requests that typically originate from the
//           Resource Manager Client Service (RMCS). These requests are
//           in messages derived from the aal_q_item type.
//           Some/most messages are directed toward user space services but
//           not all.
//           NOTE: Requests are moved from the request queue to the pending
//                 queue. This must be done atomically so that requests are
//                 always in a trackable state so that it can be canceled.

// TODO BUG occurs if max owners exceeded and device removed
//=============================================================================
int aalbus_process_configmon_event( struct aalbus_session               *psess,
                                    struct aal_q_item                   *pqitem,
                                    struct aalbus_configmonitor_message *presp,
                                    btWSSize                            *pOutbufsize )
{
   
   struct aalbus_config_update_event *pevt_update = NULL;
   int                                  ret = 0;

   PTRACEIN;

   // Lock-out cancels. Do not make this interruptible - TODO perhaps shold be changed to down_killable
//   kosal_sem_get_user( &rmserver.m_sem );

   //-----------------------------------------------------------------------------
   // Update the ioctl header
   //  Since most RMSS requests are forwarded to the user mode service application
   //  the user request header is prepared up front
   //-----------------------------------------------------------------------------
   presp->id = QI_QID( pqitem );
   presp->req_handle = pqitem;

   pevt_update = qip_to_updateevent( pqitem );

   // Do not exceed buffer
   if(*pOutbufsize < QI_LEN( pqitem )){
      aalbus_config_update_event_destroy( pevt_update );
      PERR( ": No room to return the event\n" );
      *pOutbufsize = 0;
      return -EINVAL;
   }

   // Copy the payload section
   memcpy( presp, CONF_EVTP_MSG( pevt_update ), QI_LEN( pqitem ) );

   *pOutbufsize = QI_LEN( pqitem );
   presp->size = QI_LEN( pqitem );

   PNOTICE( ": CMD - rspid_KRMS_ConfigUpdate - Event Destroyed\n" );
   aalbus_config_update_event_destroy( pevt_update );


//   kosal_sem_put( &rmserver.m_sem );
   return ret;
}
