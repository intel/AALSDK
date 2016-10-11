//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2016, Intel Corporation.
//
//  This program  is  free software;  you  can redistribute it  and/or  modify
//  it  under  the  terms of  version 2 of  the GNU General Public License  as
//  published by the Free Software Foundation.
//
//  This  program  is distributed  in the  hope that it  will  be useful,  but
//  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
//  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
//  General Public License for more details.
//
//  The  full  GNU  General Public License is  included in  this  distribution
//  in the file called README.GPLV2-LICENSE.TXT.
//
//  Contact Information:
//  Henry Mitchel, henry.mitchel at intel.com
//  77 Reed Rd., Hudson, MA  01749
//
//                                BSD LICENSE
//
//  Copyright(c) 2008-2016, Intel Corporation.
//
//  Redistribution and  use  in source  and  binary  forms,  with  or  without
//  modification,  are   permitted  provided  that  the  following  conditions
//  are met:
//
//    * Redistributions  of  source  code  must  retain  the  above  copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in  binary form  must  reproduce  the  above copyright
//      notice,  this  list of  conditions  and  the  following disclaimer  in
//      the   documentation   and/or   other   materials   provided  with  the
//      distribution.
//    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
//      contributors  may  be  used  to  endorse  or promote  products derived
//      from this software without specific prior written permission.
//
//  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
//  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
//  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
//  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
//  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
//  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
//  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
//  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
//  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
//******************************************************************************
//****************************************************************************
//        FILE: aalrm_server-file.c
//     CREATED: 02/20/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE:  Implements the file character driver for the plolicy manager
//           interface service interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-20-08       JG       Initial version started
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/23/2009     JG       Initial code cleanup
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMS_DBG_FILE

#include "aalrm_server-int.h"


//=============================================================================
// Name: aalrm_server_open
// Description: Implements the open system call
// Interface: public
// Inputs: inode - inode structure of the special device file
//         file - per open file structure
// Outputs: none.
// Comments: Creates a session instance with the HW subsystem
//=============================================================================
int aalrm_server_open(struct inode *inode, struct file *file)
{
   int minor, ret = 0;
   struct aalrm_server_session *session = NULL;

   // minor number must be zero
   minor = iminor (inode);
   if (minor != 0){
      return -ENODEV;
   }

   //-------------------
   // Create the session
   //-------------------
   if (kosal_sem_get_krnl_alertable(&rmserver.m_sem)) { /* FIXME */ }

   // For now we are only allowing one session enforcing single RM user service
   if(!kosal_list_is_empty(&rmserver.m_sessq)){
      DPRINTF (AALRMS_DBG_FILE, ": open failed because device already opened\n");
      kosal_sem_put(&rmserver.m_sem);
      return -EBUSY;
   }

   kosal_sem_put(&rmserver.m_sem);

   // Create the session
   session = (struct aalrm_server_session *)kosal_kmalloc(sizeof(struct aalrm_server_session));
   if(session == NULL){
      DPRINTF (AALRMS_DBG_FILE, ": open failed to malloc\n");
      return -ENOMEM;
   }

   // Initialize session's lists, queues and sync objects
   kosal_list_init(&session->m_sessq);
   kosal_init_waitqueue_head(&session->m_waitq);
   kosal_mutex_init(&session->m_sem);


   // Save the bus pointer
   session->m_aalbus = aalbus_get_bus();

   // Get a pointer to the RMSS singleton
   session->m_rmserver = &rmserver;

   // Default events
   //  used for selectively enabling events
   session->m_eventflgs = KRMS_DEFAULT_EVENTS;

   // Add this session to the session queue of the RMSS
   rms_sess_to_rms_server(session).register_sess(&session->m_sessq);

   // Save this session on the file instance
   file->private_data = session;

   DPRINTF(AALRMS_DBG_FILE, ": AAL Resourse Manager Server Session created\n");

   return ret;
}

//=============================================================================
// Name: aalrm_server_close
// Description: Implements the close system call
// Interface: public
// Inputs: inode - inode structure of the special device file
//         file - per open file structure
// Outputs: none.
// Comments: Close pulls the plug on any outstanding transactions. This implies
//           that notifcations for completion may not be sent to the
//           application. Ideally the app is in a quiescent state before
//           calling.
//=============================================================================
int
aalrm_server_close (struct inode *inode, struct file *file)
{
   int ret = 0;
   struct aalrm_server_session *session = file->private_data;
   DPRINTF(AALRMS_DBG_FILE, ": CLOSE Entered\n");

   // Disable config updates
   session->m_aalbus->register_config_update_handler(NULL,NULL);

   //TODO make sure all messages are flushed
   // aalrm_flush_queues(session);

   // Unregister the interface with the AALBus Service Interface Broker
   rms_sess_to_rms_server(session).unregister_sess(&session->m_sessq);
   kosal_kfree(session, sizeof(struct aalrm_server_session));
   DPRINTF(AALRMS_DBG_FILE, ": CLOSE Exited\n");
   return ret;
}


//=============================================================================
// Name: aalrm_server_poll
// Description: Called from select, poll or epoll call.
// Interface: public
// Inputs: file - pointer to file instance
//         wait - pointer to the process poll_table
// Outputs: none.
// Comments:
//=============================================================================
unsigned int aalrm_server_poll ( struct file *file, poll_table *wait )
{
   unsigned int mask = 0;

   // Get the user session
   struct aalrm_server_session *psess = (struct aalrm_server_session *) file->private_data;

   // Put RMSS's request queue in the poll table
   poll_wait( file, &rmserver.m_reqq_wq, wait );

   if (kosal_sem_get_krnl_alertable( &psess->m_sem )) { /* FIXME */ }

   // If there is a request on the queue so wake up sleeper
   if( !aalrms_reqq_empty() ){
      DPRINTF( AALRMS_DBG_FILE, ": Message available. Waking sleepers\n" );
      mask |= POLLPRI;  // Device request completion
   }
   up( &psess->m_sem );

   // Return the event mask
   return mask;
}
