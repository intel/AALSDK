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
//        FILE: aaluidrv-file.c
//     CREATED: 08/26/2008
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the file related code for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           Universal Interface Driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 08/26/2008     JG       Initial version started
// 11/11/2008     JG       Added legal header
// 12/10/2008     JG       Added support for Close and cleanup
// 12/16/2008     JG       Began support for abort and shutdown
//                            Added Support for WSID object
//                            Major interface changes.
// 01/04/2009     HM       Updated Copyright
// 01/05/2009     JG       Fixed bug in session_destroy that accessed a
//                         semaphore after the object was destroyed.
//                         Enabled flush event queue
// 01/14/2009     JG       Cleanup and refactoring
// 10/12/2009     JG       Support for device method changes
// 01/07/2010     JG       Fixed a bug in destroy_session that compared the
//                            return code of removeowner() the wrong value
//                            which resulted in an invalid log message.
// 05/18/2010     HM       Labeled kosal_sem_get_krnl_alertable() with FIXME. Need a
//                            global fix for these. Getting it to compile.
// 11/09/2010     HM       Removed extraneous kernel includes
// 02/26/2013     AG       Add wsid tracking and validation routines
//****************************************************************************
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalids.h"
#include "aaluidrv-int.h"

#define MODULE_FLAGS UIDRV_DBG_MOD

////////////////////////////////////////////////////////////////////////////////


//-----------
// Prototypes
//-----------
struct uidrv_session * uidrv_session_create(btPID pid);



//=============================================================================
// Name: uidrv_open
// Description: Implements the open system call
// Interface: public
// Inputs: inode - pointer to inode for device node
//         file - pointer to file instance for this open
// Outputs: none.
// Comments: Creates a per process session instance with the device control
//           subsystem.The session holds the state and context between an
//           application process, the device and associated servciecs (e.g,
//           workspace manager).
//           The UDDI maintains a list of all open sessions.
//=============================================================================
int uidrv_open  (struct inode *inode, struct file *file)
{
   struct uidrv_session *psess = NULL;
   int ret = 0;

   DPRINTF (UIDRV_DBG_FILE, ":Opened by pid = %d tgid = %d\n",current->pid, current->tgid );

   // Create a session
   psess = uidrv_session_create( current->tgid );
   if( unlikely( psess == NULL ) ) {
      DPRINTF (UIDRV_DBG_FILE, "Create session failed.\n" );
      return -ENOMEM;
   }

   file->private_data = psess;

   // Add it to the list of sessions
   if (kosal_sem_get_krnl_alertable( &thisDriver.m_qsem)) { /* FIXME */ }
   list_add_tail( &psess->m_sessions, &thisDriver.m_sessq);
   up( &thisDriver.m_qsem);

   DPRINTF (UIDRV_DBG_FILE, "Application Session Created sess=%p\n", psess);
   return ret;
}

//=============================================================================
// Name: uidrv_session_create
// Description: Create a new application session
// Interface: public
// Inputs: pid - ID of process
// Outputs: none.
// Comments: The process ID is used to identify the session uniquely
//=============================================================================
struct uidrv_session * uidrv_session_create(btPID pid)
{
   struct uidrv_session * psession = (struct uidrv_session * )kosal_kmalloc(sizeof(struct uidrv_session));
   if(unlikely (psession == NULL) ){
      DPRINTF (UIDRV_DBG_FILE, ": failed to malloc session object\n");
      return NULL;
   }

   // Initialize session's lists, queues and sync objects
   kosal_list_init(&psession->m_sessions);
   kosal_list_init(&psession->m_devicelist);

   // Initialize queues
   kosal_init_waitqueue_head(&psession->m_waitq);
   aal_queue_init(&psession->m_eventq);

   kosal_mutex_init(&psession->m_sem);

   // Record the owning process
   psession->m_pid = pid;

   // Bind the UI driver message handler
   psession->m_msgHandler.sendevent = uidrv_sendevent;
   psession->m_msgHandler.getwsid = uidrv_getwsid;
   psession->m_msgHandler.freewsid = uidrv_freewsid;
   psession->m_msgHandler.valwsid = uidrv_valwsid;

   // Record a pointer to the singleton UDDI
   psession->m_aaldriver = &thisDriver;

   return psession;
}

//=============================================================================
// Name: uidrv_close
// Description: Implements the close system call
// Interface: public
// Inputs: .
// Outputs: none.
// Comments: Close pulls the plug on any outstanding transactions. This implies
//           that notifications for completion may not be sent to the
//           application. Ideally the app is in a quiescent state before
//           calling.
//=============================================================================
int uidrv_close (struct inode *inode, struct file *file)
{
   struct uidrv_session *psess = file->private_data;
   file->private_data = NULL;
   DPRINTF (UIDRV_DBG_FILE, ": Closing session %p\n",psess);

   return uidrv_session_destroy(psess);
}

//=============================================================================
// Name: uidrv_session_destroy
// Description: Destroy the application session
// Interface: public
// Inputs: psess - session to detroy
// Outputs: none.
// Comments: responsible for flushing all queues and canceling any outstanding
//           transactions.  All devices are freed.
//=============================================================================
int uidrv_session_destroy(struct uidrv_session * psess)
{

    struct aaldev_owner  *itr=NULL, *tmp=NULL;
    struct aal_device *pdev=NULL;
    struct aaldev_ownerSession *ownerSessp = NULL;

    DPRINTF (UIDRV_DBG_FILE, ": Destroying session %p\n",psess);

    if (kosal_sem_get_krnl_alertable(&psess->m_sem)) { /* FIXME */ }

    // Flush the event queue and wake anything waiting
    uidrv_flush_eventqueue(psess);
    wake_up_interruptible (&psess->m_waitq);  //NOT SURE IF THIS IS SAFE

    DPRINTF (UIDRV_DBG_IOCTL, ": Closing UI channel\n");

    //--------------------------------------------------------
    // Walk through the list of devices and free them
    //--------------------------------------------------------
    kosal_list_for_each_entry_safe(itr, tmp, &psess->m_devicelist, m_devicelist, struct aaldev_owner) {

       DPRINTF (UIDRV_DBG_IOCTL, ": Walking device list %p %p \n",psess,itr);

       // itr has the aaldev_owner manifest
       pdev = itr->m_device;

       // Device owner session is the instance of the interfaces
       // for this device/session binding
       ownerSessp = dev_OwnerSession(pdev,psess->m_pid);

       DPRINTF (UIDRV_DBG_IOCTL, ": Got owner session %p\n",ownerSessp);

       // Unbind the PIP from the session
       if(unlikely(!aaldev_pipmsgHandlerp(pdev)->unBindSession( ownerSessp ))){
          DPRINTF (UIDRV_DBG_FILE, ": uid_errnumCouldNotUnBindPipInterface\n");

       }

       // Update the ownerslist
       if(unlikely( dev_removeOwner( pdev,
                                     psess->m_pid) != 1 )){
          DPRINTF (UIDRV_DBG_IOCTL, ": Failed to remove owner\n");
       }

    } // end kosal_list_for_each_entry_safe

    // Remove from the UDDI session list
    if (kosal_sem_get_krnl_alertable( &thisDriver.m_qsem)) { /* FIXME */ }
    list_del(&psess->m_sessions);
    up( &thisDriver.m_qsem);

    kosal_sem_put(&psess->m_sem);
    kfree(psess);

    DPRINTF (UIDRV_DBG_IOCTL, ": done\n");
    return 0;
}


//=============================================================================
// Name: uidrv_poll
// Description: Called from select, poll or epoll call.
// Interface: public
// Inputs: .
// Outputs: none.
// Comments:
//=============================================================================
unsigned int uidrv_poll ( struct file *file, poll_table *wait )
{
   struct uidrv_session *psess = ( struct uidrv_session * ) file->private_data;
   unsigned int mask = 0;

   // Put session's waitq in the poll table
   poll_wait ( file, &psess->m_waitq, wait );

   // If there is a request on the queue wakeup sleeper
   if (kosal_sem_get_krnl_alertable( &psess->m_sem )) { /* FIXME */ }
   if( !_aal_q_empty(&psess->m_eventq) ){
      DPRINTF( UIDRV_DBG_FILE, ": Message available. Waking sleepers\n" );
      mask |= POLLPRI;  // Device request completion
   }
   up ( &psess->m_sem );

   return mask;
}

