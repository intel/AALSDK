//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2014, Intel Corporation.
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
//  Copyright(c) 2008-2014, Intel Corporation.
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
//        FILE: aalrm-file.c
//     CREATED: 02/13/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE:  Implements the file character driver API for the AAL Resource
//           Manager Client Service.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-13-08       JG       Initial version started
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
// 02/09/2009     JG       Added support for RMSS cancel transaction   TODO - Make trancnt set atomic
// 05/13/2010     JG       Fixed bug in close where transactions were canceled
//                            after devices were freed.
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
// 08/28/2013     JG       Added suport for kOSAL
//****************************************************************************
#define MODULE_FLAGS AALRMC_DBG_FILE

#include "aalsdk/kernel/kosal.h"
#include "aalrm-int.h"
#include "aalsdk/kernel/aalrm_server-services.h"

//=============================================================================
// Name: aalrm_open
// Description: Implements the open system call
// Interface: public
// Inputs: inode - pointer to inode for device node
//         file - pointer to file instance for this open
// Outputs: none.
// Comments: Creates a session instance with the RMC service. The session
//           is the state and context between an application process and the
//           RMC service.
//=============================================================================
btInt aalrm_open(struct inode *inode, struct file *file)
{
   btInt minor, ret = 0;
   struct aalresmgr_session *session = NULL;

   DPRINTF (AALRMC_DBG_FILE, ":RM-Opened by pid = %d tgid = %d\n",current->pid, current->tgid );

   // minor number must be zero
   minor = iminor (inode);
   if (minor != 0){
      return -ENODEV;
   }

   //-------------------
   // Create the session
   //-------------------
   session = (struct aalresmgr_session *)kosal_kmalloc(sizeof(struct aalresmgr_session));
   if(session == NULL){
      DPRINTF (AALRMC_DBG_FILE, ": open failed to malloc\n");
      return -ENOMEM;
   }

   // Initialize session's lists, queues and sync objects
   kosal_list_init(&session->m_devlist);
   kosal_list_init(&session->m_sessq);
   kosal_init_waitqueue_head(&session->m_waitq);
   kosal_mutex_init(&session->m_sem);

   // Transaction count
   session->m_transcnt=0;

   // Initialize the event queue
   aal_queue_init(&session->m_eventq);

   // Get a pointer to the resource manager client service singleton
   session->m_resmgr = &resmgr;

   // Record the owning process
   //   This is used to identify a process as owning a resource
   session->m_tgpid = current->tgid;


   // Add this session to the session queue of the resource manager
   rmssess_to_rmcsrv(session).register_sess(&session->m_sessq);

   // Save this session on the file instance
   file->private_data = session;

   DPRINTF(AALRMC_DBG_FILE, ": RM-AAL Session created\n");

   return ret;
}


//=============================================================================
// Name: aalrm_close
// Description: Implements the close system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: Close pulls the plug on any outstanding transactions. This implies
//           that notifications for completion may not be sent to the
//           application. Ideally the app is in a quiescent state before
//           calling.
//=============================================================================
btInt aalrm_close (struct inode *inode, struct file *file)
{
   btInt ret = 0, loopcnt=0;
   struct aaldev_owner *itr, *tmp;
   struct aalresmgr_session *psess = file->private_data;

   // Set the transaction ID to cancel requests
   struct aalrms_req_tranID tranID =
   {
     .m_context = psess
   };

   DPRINTF(AALRMC_DBG_FILE, ": RM-CLOSE Entered\n");

   // Cancel all outstanding transactions
   if(psess->m_transcnt != 0){
     //-----------------------------------
     // Wait no more that 1 second per
     // outstanding transaction to cancel
     //-----------------------------------
     loopcnt = psess->m_transcnt * 10;

     // Cancel all outstanding requests for this session
     i_rmserver(rmssess_to_rmssrv(psess)).cancel_all_requests(  &tranID  );

     while((psess->m_transcnt != 0) && loopcnt--){
        DPRINTF(AALRMC_DBG_FILE, ": Waiting up to %d seconds on %d transactions to cancel.\n",loopcnt/10,psess->m_transcnt);
        kosal_mdelay(100);
     }
   }

   // Free all devices that have not bound
   if (kosal_sem_get_krnl_alertable(&psess->m_sem)) { /* FIXME */ }

   // Walk the owner list and delete each entry
   kosal_list_for_each_entry_safe(itr, tmp, &psess->m_devlist, m_devicelist, struct aaldev_owner){
     DPRINTF(AALRMC_DBG_FILE, ": Removing ownership from unbound devices\n");
     dev_removeOwner(itr->m_device, psess->m_tgpid);
   }

   kosal_sem_put(&psess->m_sem);

   // flush the event queue
   aalrm_flush_eventqueue( psess );

   // remove the session from the RMC queue
   rmssess_to_rmcsrv(psess).unregister_sess(&psess->m_sessq);
   kfree(psess);
   DPRINTF(AALRMC_DBG_FILE, ": RM-CLOSE Exited\n");
   return ret;
}


//=============================================================================
// Name: aalrm_poll
// Description: Called from select, poll or epoll call.
// Interface: public
// Inputs: .
// Returns: Mask indicating if there are any events available
// Comments:
//=============================================================================
btUnsignedInt aalrm_poll( struct file *file, poll_table *wait )
{
   btUnsignedInt mask = 0;

   // Get the user session
   struct aalresmgr_session *psess = (struct aalresmgr_session *) file->private_data;

   // Put session's waitq in the poll table
   poll_wait( file, &psess->m_waitq, wait );

   // Check for any resource request completions
   if (kosal_sem_get_krnl_alertable( &psess->m_sem )) { /* FIXME */ }
   if( !_aal_q_empty( &psess->m_eventq ) ){
      mask |= POLLPRI;  // Device request completion
   }
   up( &psess->m_sem );

   // Return the event mask
   return mask;
}


