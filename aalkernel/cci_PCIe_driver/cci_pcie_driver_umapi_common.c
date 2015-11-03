//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015, Intel Corporation.
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
//  Copyright(c) 2015, Intel Corporation.
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
//        FILE: cci_pcie_driver_umapi_common.c
//     CREATED: 10/23/2015
//      AUTHOR: Joseph Grecco
//
// PURPOSE:  This file contains the OS independent code for the Intel(R)
//           QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           User Mode Interface for the AAL CCI device driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/203/2015     JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS UIDRV_DBG_MOD


#include "cci_pcie_driver_umapi_linux.h"
#include "cciui-events.h"
#include "aalsdk/kernel/aalui-events.h"

// Prototypes
struct ccidrv_session * ccidrv_session_create(btPID );
btInt ccidrv_session_destroy(struct ccidrv_session * );
btInt ccidrv_fasync(btInt fd, struct file *, btInt );
btInt ccidrv_messageHandler( struct ccidrv_session  *,
                             btUnsigned32bitInt     ,
                             struct aalui_ioctlreq *,
                             btWSSize               ,
                             struct aalui_ioctlreq *,
                             btWSSize              *);

btInt ccidrv_sendevent( void *,
                               struct aal_device *,
                               struct aal_q_item *,
                               void *);
struct aal_wsid* ccidrv_getwsid( struct aal_device *,
                                        btUnsigned64bitInt );
btInt ccidrv_freewsid(struct aal_wsid *);
btInt ccidrv_valwsid(struct aal_wsid *);
btInt ccidrv_flush_eventqueue(  struct ccidrv_session *psess);

btInt process_send_message( struct ccidrv_session  *,
                            struct aalui_ioctlreq *);
btInt process_send_message( struct ccidrv_session  *,
                            struct aalui_ioctlreq *);
btInt process_bind_request( struct ccidrv_session  *psess,
                            struct aalui_ioctlreq *preq);
btInt ccidrv_process_message( struct aalui_ioctlreq *preq,
                              struct aal_q_item     *pqitem,
                              struct aalui_ioctlreq *resp,
                              btWSSize              *pOutbufsize);

extern struct um_APIdriver thisDriver;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              SESSION METHODS              ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: ccidrv_session_create
// Description: Create a new application session
// Interface: public
// Inputs: pid - ID of process
// Outputs: none.
// Comments: The process ID is used to identify the session uniquely.
//           The session is a context that hold information related to the
//           application process. It holds things like the list of devices
//           allocated by this process, the event queue used to communicate
//           back to the application and the signaling object for waking the
//           process.
//=============================================================================
struct ccidrv_session * ccidrv_session_create(btPID pid)
{
   // Allocate the Session object
   struct ccidrv_session * psession = (struct ccidrv_session * )kosal_kmalloc(sizeof(struct ccidrv_session));
   if(unlikely (psession == NULL) ){
      PERR(": failed to malloc session object\n");
      return NULL;
   }

   // Initialize session's lists, queues and sync objects
   //   m_sessions is used to place this session on the list of sessions
   //   held by the driver.
   kosal_list_init(&psession->m_sessions);

   //  m_devicelist is the root of the list of devices currently held by
   //  this session.

   kosal_list_init(&psession->m_devicelist);

   // m_waitq is used for asynchronous signaling (see poll)
   kosal_init_waitqueue_head(&psession->m_waitq);

   // m_eventq holds events waiting to be delivered
   aal_queue_init(&psession->m_eventq);

   kosal_mutex_init(&psession->m_sem);

   // Record the process that opened us
   psession->m_pid = pid;

   return psession;
}

//=============================================================================
// Name: ccidrv_session_destroy
// Description: Destroy the application session
// Interface: public
// Inputs: psess - session to detroy
// Outputs: none.
// Comments: responsible for flushing all queues and canceling any outstanding
//           transactions.  All devices are freed.
//=============================================================================
int ccidrv_session_destroy(struct ccidrv_session * psess)
{
   // Variables for walking the device list
    struct aaldev_owner  *devowner_itr=NULL, *tmp=NULL;
    struct aal_device *pdev=NULL;
    struct aaldev_ownerSession *ownerSessp = NULL;

    PVERBOSE("Destroying session %p\n",psess);

    // Protect the critical section
    if (kosal_sem_get_krnl_alertable(&psess->m_sem))
    {
       PERR("Failed to claim semaphore.  FATAL ERROR!\n");
       return -EIO;
    }

    // Flush the event queue and wake anything waiting
    ccidrv_flush_eventqueue(psess);
    kosal_wake_up_interruptible (&psess->m_waitq);

    PVERBOSE("Closing UI channel\n");

    // Walk through the list of ./prepdevices and free them
    kosal_list_for_each_entry_safe(devowner_itr, tmp, &psess->m_devicelist, m_devicelist, struct aaldev_owner) {

       PVERBOSE("Walking device list %p %p \n",psess, devowner_itr);

       // devowner_itr has the aaldev_owner object. The device owner object
       //  holds the context about this ownership relationship. E.g., owner PID, the device itself etc..
       pdev = devowner_itr->m_device;  // Get the device to free

       // Unbind the PIP from the session. This gives the driver a chance to cleanup
       //  the hardware if necessary
       if(unlikely(!aaldev_pipmsgHandlerp(pdev)->unBindSession( ownerSessp ))){
          PERR("uid_errnumCouldNotUnBindPipInterface\n");

       }

       // Update the ownerslist
       if(unlikely( dev_removeOwner( pdev,
                                     psess->m_pid) != 1 )){
          PERR("Failed to remove owner\n");
       }

    } // end kosal_list_for_each_entry_safe

    // Remove from the UDDI session list
    if (kosal_sem_get_krnl_alertable( &thisDriver.m_qsem)){
       PERR("Failed to claim semaphore.  FATAL ERROR!\n");
       return -EIO;
    }
    kosal_list_del(&psess->m_sessions);
    kosal_sem_put( &thisDriver.m_qsem);

    kosal_sem_put(&psess->m_sem);
    kosal_kfree(psess, sizeof(struct ccidrv_session));

    PVERBOSE("done\n");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////        MESSAGE PROCESSING METHODS         ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: ccidrv_messageHandler
// Description: Implements the OS independent message handler.
// Interface: public
// Inputs: psess - CCI Driver session
//         cmd - Command number
//         preq - Pointer to Request (input buffer)
//         InbufSize - Size of input buffer
//         presp - Pointer to response (output vuffer)
//         pOutbufSize - Pointer to output buffer size
// Outputs: number of bytes transfered to output buffer written to *pOutbufSize
// Returns: status code: 0 == success
// Comments: Entry point for all requests from user space
//=============================================================================
btInt
ccidrv_messageHandler( struct ccidrv_session *psess,
                       btUnsigned32bitInt     cmd,
                       struct aalui_ioctlreq *preq,
                       btWSSize               InbufSize,
                       struct aalui_ioctlreq *presp,
                       btWSSize              *pOutbufSize)
{
   btInt    ret            = -EINVAL; // Assume failure

   // Variables used in the Device Allocate Messages
   struct aal_q_item *pqitem = NULL; // Generic request queue item

   btWSSize OutbufSize     = *pOutbufSize;
   *pOutbufSize            = 0; // prepare for error case

#if 1
# define UIDRV_IOCTL_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_IOCTL_CASE(x) case x :
#endif

   PTRACEIN;

   UNREFERENCED_PARAMETER(InbufSize);

   ASSERT(NULL != presp);
   ASSERT(NULL != pOutbufSize);

   // Process the message
   switch ( cmd ) {

      // Get next queued message descriptor
      // This will contain things like its
      // size and type
      UIDRV_IOCTL_CASE(AALUID_IOCTL_GETMSG_DESC) {
         // Make sure there is a message to be had
         if ( _aal_q_empty(&psess->m_eventq) ) {
            PERR("No Message available\n");
            ret = -EAGAIN;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Peek the head of the message queue
         pqitem = _aal_q_peek(&psess->m_eventq);
         if ( NULL == pqitem ) {
            PERR("Corrupt event queue\n");
            ret = -EFAULT;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Copy the original header into the response then
         //   modify the parts as necessary.
         if ( OutbufSize <= sizeof(struct aalui_ioctlreq) ) {
            memcpy(presp, preq, sizeof(struct aalui_ioctlreq));
         }

         // Return the type and total size of the message that will be returned
         presp->id   = (uid_msgIDs_e)QI_QID(pqitem);
         presp->size = QI_LEN(pqitem);

         PVERBOSE("Getting Message Decriptor - size = %" PRIu64 "\n", preq->size);

         // GETMSG_DESC always returns size of ioctlreq (i.e., header)
         *pOutbufSize = sizeof(struct aalui_ioctlreq);
         ret = 0;
         PTRACEOUT_INT(ret);
      } return ret; // case AALUID_IOCTL_GETMSG_DESC:


      // Get the next message off of the queue
      // returns a copy of the message and moves the item to
      // the pending queue
      UIDRV_IOCTL_CASE(AALUID_IOCTL_GETMSG) {
         // Make sure there is a message to be had
         if ( _aal_q_empty(&psess->m_eventq) ) {
            PERR("No Message available\n");
            ret = -EAGAIN;
            PTRACEOUT_INT(ret);
            return ret;
         }

         //------------------------
         // Get the request message
         //------------------------
         pqitem = _aal_q_dequeue(&psess->m_eventq);
         if ( NULL == pqitem ) {
            PERR("Invalid or corrupted request\n");
            ret = -EFAULT;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Process the request
         *pOutbufSize = OutbufSize;
         return ccidrv_process_message(preq, pqitem, presp, pOutbufSize);
      } break; // case  AALUID_IOCTL_GETMSG:


      // Send the message to the device or PIP (SW driver)
      UIDRV_IOCTL_CASE(AALUID_IOCTL_SENDMSG) {
         return process_send_message(psess, preq);
      } break;

      // Process Bind device
      UIDRV_IOCTL_CASE(AALUID_IOCTL_BINDDEV) {
         return process_bind_request(psess, preq);
      } break;

      // Activate device - This is a framework command that
      // causes a device to "appear" in to the system
      UIDRV_IOCTL_CASE(AALUID_IOCTL_ACTIVATEDEV) {
         PERR("TODO\n");
      } break;

      // Deactivate device - This is a framework command that
      // causes a device to be removed from the system
      UIDRV_IOCTL_CASE(AALUID_IOCTL_DEACTIVATEDEV) {
         PERR("TODO\n");
      } break;

      default : {
         PERR("Invalid IOCTL = 0x%x\n", cmd);
      } break;
   } //  switch (cmd)

   ret = -1;
   PTRACEOUT_INT(ret);
   return ret;
}

//=============================================================================
// Name: process_send_message
// Description: Process a send request
// Interface: public
// Inputs: psess - session
//         preq - request header
// Outputs: negative number - transaction commplete
// Comments: This is the function that
//=============================================================================
btInt
process_send_message(struct ccidrv_session  *psess,
                     struct aalui_ioctlreq *preq)
{
   btInt                              ret = 0;
   struct aal_device                 *pdev;
   struct aaldev_ownerSession        *ownSessp;
   struct aal_pipmessage              pipMessage;
   ui_shutdownreason_e                shutdown_reason;
   btTime                             timeleft;
   struct uidrv_event_shutdown_event *newreq;
//   btWSSize                             messagesize;

#if 1
# define UIDRV_PROCESS_SEND_MESSAGE_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_SEND_MESSAGE_CASE(x) case x :
#endif

   PTRACEIN;
   ASSERT(NULL != psess);
   ASSERT(NULL != preq);

   //--------------------------------------------------------------------------
   // Process the request copying in remaining request arguments as appropriate
   //--------------------------------------------------------------------------
   switch ( preq->id ) {

      // Send a message to the AFU via the PIP message handler
      UIDRV_PROCESS_SEND_MESSAGE_CASE(reqid_UID_SendAFU) {
         // Get the handle and validate
         pdev = aaldev_handle_to_devp(preq->handle);
         if ( unlikely(NULL == pdev) ) {
            PERR("Invalid device handle %p returned %p\n", preq->handle, pdev);
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }


         // Get the ownerSession for this device - The ownerSession is
         //  an object that holds the state of a session between a device
         //  and a particular process. Since a shared device may be "owned"
         //  by multiple processes simultaneously, the device maintains a
         //  a list of ownerSessions it is currently a member of.
         //----------------------------------------------------------------
         //
         // Get ownerSession for this pid on the device
         //
         ownSessp = dev_OwnerSession(pdev,psess->m_pid);
         if ( NULL == ownSessp ) {
            PERR("Not owner or no message handler.\n");
            ret = -EACCES;
            PTRACEOUT_INT(ret);
            return ret;
         }
         PDEBUG("Got owner %p\n", ownSessp);

         // Wrap the message and transaction identification
         //  pipMessage is a generic message wrapper for all
         //  PIP message handlers
         pipMessage.m_message = aalui_ioctlPayload(preq);
         pipMessage.m_tranID = preq->tranID;
         pipMessage.m_context = preq->context;

         // Send the message on to the device specific command handler.
         //  This macro resolves to calling the low level, device specific, command
         //  handler called the Physical Interface Protocol (PIP).  This enabled devices
         //  served by this driver have custom low level drivers.
         ret = aalsess_pipSendMessage(ownSessp)(ownSessp, pipMessage);
         pipMessage.m_message = NULL;

         PTRACEOUT_INT(ret);
      } return ret; // case reqid_UID_SendAFU

      UIDRV_PROCESS_SEND_MESSAGE_CASE(reqid_UID_Shutdown) {
         // Create shutdown  request object
         shutdown_reason =((struct aalui_Shutdown*) aalui_ioctlPayload(preq))->m_reason;

         // Assume kernel shutdown takes zero time for now so return original
         //   timeout value in event which indicates how much time as actually
         //   used to shutdown (timeout-amount used(0) = timeleft
         timeleft = ((struct aalui_Shutdown*)aalui_ioctlPayload(preq))->m_timeout;

         PDEBUG("Received a shutdown ioctl reason %d\n", shutdown_reason);
         newreq = uidrv_event_shutdown_event_create(shutdown_reason,
                                                    timeleft,
                                                    &preq->tranID,
                                                    preq->context,
                                                    uid_errnumOK);
         ASSERT(NULL != newreq);
         if ( NULL == newreq ) {
            ret = -ENOMEM;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Enqueue the shutdown event
         _aal_q_enqueue(ui_evtp_uishutdown_to_qip(newreq), &psess->m_eventq);

         // Unblock select() calls.
         kosal_wake_up_interruptible( &psess->m_waitq);
      } break; // case reqid_UID_Shutdown

      default : {
         PERR("Unrecognized send message option %d\n", preq->id);
         ret = -EINVAL;
         PTRACEOUT_INT(ret);
      } return ret;

   } // switch (preq->id)

   ret = 0;
   PTRACEOUT_INT(ret);
   return ret;
} // process_send_message



//=============================================================================
// Name: process_bind_request
// Description: Process a bind request
// Interface: public
// Inputs: psess - session
//         preq - request headers
//         arg - original user input args
// Outputs: none.
// Comments: This function process the session bind and unbind requests.
//           Basically Bind/Unbind are similar to Open/Close in a traditional
//           character driver. Unlike a traditional driver we do not use device
//           nodes to represent the
//=============================================================================
btInt
process_bind_request(struct ccidrv_session  *psess,
                     struct aalui_ioctlreq *preq)
{
   struct aal_device              *pdev         = NULL;
   struct uidrv_event_bindcmplt   *bindcmplt    = NULL;
   struct uidrv_event_unbindcmplt *unbindcmplt  = NULL;
   struct aalui_extbindargs        bindevt      = {0};
   struct aaldev_ownerSession     *ownerSessp   = NULL;
   btInt                           ret          = 0;

#if 1
# define UIDRV_PROCESS_BIND_REQUEST_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_BIND_REQUEST_CASE(x) case x :
#endif

   ASSERT(NULL != psess);
   ASSERT(NULL != preq);

   switch ( preq->id ) {
      //--------------------
      //Process bind request
      //--------------------
      UIDRV_PROCESS_BIND_REQUEST_CASE(reqid_UID_Bind) {
         // Get the device from the handle and validate
         pdev = aaldev_handle_to_devp(preq->handle);
         ASSERT(NULL != pdev);
         if ( unlikely( NULL == pdev ) ) {
            PERR("Invalid device handle %p\n", preq->handle);
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Make sure the device has a PIP bound to it
         if ( unlikely( NULL == aaldev_pipp(pdev) ) ) {
            PERR("uid_errnumDeviceHasNoPIPAssigned\n");
            // Create error event
            bindcmplt = uidrv_event_bindcmplt_create(NULL, NULL, uid_errnumDeviceHasNoPIPAssigned, preq);
            goto BIND_DONE;
         }

         // Get the ownerSession for this device - The ownerSession is
         //  an object that holds the state of a session between a device
         //  and a particular process. Since a shared device may be "owned"
         //  by multiple processes simultaneously, the device maintains a
         //  a list of ownerSessions it is currently a member of.
         ownerSessp = dev_OwnerSession(pdev, psess->m_pid);
         if ( NULL == ownerSessp ) {
            PERR("Process not owner of this device.\n");
            // Create error event
            bindcmplt = uidrv_event_bindcmplt_create(NULL, NULL, uid_errnumNotDeviceOwner, preq);
            goto BIND_DONE;
         }

         // Set the owner session - Through the owner session
         //  the system can always get to the device (downstream)
         //  and the UIdrv session (upstream).
         ownerSessp->m_device   = pdev;                 // Device
         ownerSessp->m_UIHandle = psess;                // This session

         //---------------------------------------------------
         // Bind the PIP to the session
         //   Allows the PIP to do anything it needs to record
         //   the presence of this session.
         //---------------------------------------------------
         if ( unlikely( !aaldev_pipmsgHandlerp(pdev)->bindSession(ownerSessp) ) ) {
            PERR("uid_errnumCouldNotBindPipInterface\n");
            // Create error event
            bindcmplt = uidrv_event_bindcmplt_create(NULL, NULL, uid_errnumCouldNotBindPipInterface, preq);
            goto BIND_DONE;
         }
         //----------------------------------------------------------------
         // Update the ownerslist - changes owning session.
         //   Replaces the current session attributes
         //   and moves the device to this session's
         //   device list. This function only updates an existing session.
         //   The process must already be on the ownerstack for this to
         //   this to succeed. This has already been determined above
         //   by the dev_OwnerSession() call.
         //----------------------------------------------------------------
         PDEBUG("Changing owner session to UI\n");
         if ( unlikely( aaldev_addowner_OK != dev_updateOwner(pdev,                 // Device
                                                              psess->m_pid,         // Process ID
                                                              ownerSessp,           // New session attributes
                                                              &psess->m_devicelist) // Head of our session device list
                      )
            ) {
            PERR("Failed to update owner: uid_errnumCouldNotClaimDevice\n");
            // Create error event
            bindcmplt = uidrv_event_bindcmplt_create(NULL, NULL, uid_errnumCouldNotClaimDevice, preq);
         } else {
            // Fill out the extended bind parameters
            bindevt.m_apiver      = aalsess_pipmsgID(ownerSessp);
            bindevt.m_pipver      = aaldev_pipid(pdev);
            bindevt.m_mappableAPI = aaldev_mappableAPI(pdev);

            PDEBUG("Creating bind event with API ver 0x%" PRIx64 " and PIP ver 0x%" PRIx64 " MAPPABLE = 0x%x\n",
                     bindevt.m_apiver, bindevt.m_pipver, bindevt.m_mappableAPI);

            // Create the completion event
            bindcmplt = uidrv_event_bindcmplt_create(preq->handle, &bindevt, uid_errnumOK, preq);
         }

      } goto BIND_DONE; // case reqid_UID_Bind

      //----------------------
      //Process Unbind request
      //----------------------
      UIDRV_PROCESS_BIND_REQUEST_CASE(reqid_UID_UnBind) {
         // Get the device from the handle and validate
         pdev = aaldev_handle_to_devp(preq->handle);
         ASSERT(NULL != pdev);
         if ( unlikely( NULL == pdev ) ) {
            PERR("Invalid device handle %p\n", preq->handle);
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Get the device session for this device
         // Get the default message interface
         ownerSessp = dev_OwnerSession(pdev, psess->m_pid);
         if ( NULL == ownerSessp ) {
            // TODO: no error event here?
            PERR("Not owner or no message handler.\n");
            ret = -EACCES;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Unbind the PIP from the session
         if ( unlikely( !aaldev_pipmsgHandlerp(pdev)->unBindSession(ownerSessp) ) ) {
             PERR("uid_errnumCouldNotUnBindPipInterface\n");
             // Create error event
             unbindcmplt = uidrv_event_Unbindcmplt_create(uid_errnumCouldNotUnBindPipInterface,preq);
             ret = -EINVAL;
             goto UNBIND_DONE;
         }

         // Update the owner's list
         if ( unlikely( !dev_removeOwner(pdev, psess->m_pid) ) ) {
            PERR("Failed to update owner\n");
            unbindcmplt = uidrv_event_Unbindcmplt_create(uid_errnumNotDeviceOwner, preq);
            ret = -EINVAL;
         } else {
            unbindcmplt = uidrv_event_Unbindcmplt_create(uid_errnumOK, preq);
         }

      } goto UNBIND_DONE; // case reqid_UID_UnBind

      default : {
         PERR("Unrecognized Bind option %d\n", preq->id);
         ret = -EINVAL;
         PTRACEOUT_INT(ret);
         return ret;
      }

   }  // switch(preq->id)

   //-------------------
   // Generate the event
   //-------------------

BIND_DONE:
   ASSERT(NULL != bindcmplt);
   if ( NULL == bindcmplt ) {
      if ( 0 == ret ) {
         ret = -ENOMEM;
      }
      PTRACEOUT_INT(ret);
      return ret;
   }

   // Enqueue the completion event
   _aal_q_enqueue(ui_evtp_bindcmplt_to_qip(bindcmplt), &psess->m_eventq);
   kosal_wake_up_interruptible( &psess->m_waitq );

   PTRACEOUT_INT(ret);
   return ret;

UNBIND_DONE:
   ASSERT(NULL != unbindcmplt);
   if ( NULL == unbindcmplt ) {
      if ( 0 == ret ) {
         ret = -ENOMEM;
      }
      PTRACEOUT_INT(ret);
      return ret;
   }

   // Enqueue the completion event
   _aal_q_enqueue(ui_evtp_unbindcmplt_to_qip(unbindcmplt), &psess->m_eventq);

   // Unblock select() calls.
   kosal_wake_up_interruptible( &psess->m_waitq );

   PTRACEOUT_INT(ret);
   return ret;
}  // process_bind_request

//=============================================================================
// Name: ccidrv_process_message
// Description: Pre-process a queued message targeted for the application.
//              Called from AALUID_IOCTL_GETMSG, the message is unpacked,
//              any kernel level functions performed and the user mode event
//              parameters are returned.
// Interface: private
// Inputs: unsigned long arg - pointer to user space event target
//         struct aalui_ioctlreq *preq - request header
//         struct aal_q_item *pqitem - message to process
// Outputs: length of output buffer is copied to *Outbufsize.
//          return code: 0 == success
// Comments: Kernel event is destroyed
//=============================================================================
btInt
ccidrv_process_message( struct aalui_ioctlreq *preq,
                        struct aal_q_item     *pqitem,
                        struct aalui_ioctlreq *resp,
                        btWSSize              *pOutbufsize)
{
   btInt    ret = 0;
   btWSSize Outbufsize;

#if 1
# define UIDRV_PROCESS_MESSAGE_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_MESSAGE_CASE(x) case x :
#endif

   PTRACEIN;

   ASSERT(NULL != pqitem);
   ASSERT(NULL != resp);
   ASSERT(NULL != pOutbufsize);

   Outbufsize = *pOutbufsize;
   *pOutbufsize = 0; // Prepar
   // Switch on message type
   switch ( pqitem->m_id ) {
      //-------------------
      // Shutdown  Complete
      //-------------------
      UIDRV_PROCESS_MESSAGE_CASE(rspid_UID_Shutdown) {
         // Copy the header portion of the response
         resp->id      = (uid_msgIDs_e)pqitem->m_id;
         resp->errcode = qip_to_ui_evtp_uishutdown(pqitem)->m_errnum;
         resp->handle  = NULL;
         resp->context = qip_to_ui_evtp_uishutdown(pqitem)->m_context;
         resp->tranID  = qip_to_ui_evtp_uishutdown(pqitem)->m_tranID;
         resp->size    = pqitem->m_length;

         PDEBUG("Shutdown reason %d\n", ((struct aalui_Shutdown *)qip_to_ui_evtp_uishutdown(pqitem)->m_payload)->m_reason);

         // Copy the body of the message
         if ( ( resp->size + sizeof(struct aalui_ioctlreq) ) > Outbufsize ) {
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         memcpy((char *)resp + sizeof(struct aalui_ioctlreq), qip_to_ui_evtp_uishutdown(pqitem)->m_payload, (size_t)resp->size);

         // Destroy the event
         uidrv_event_shutdown_event_destroy(qip_to_ui_evtp_uishutdown(pqitem));
      } break; // case rspid_UID_Shutdown

      //--------------
      // Bind Complete
      //--------------
      UIDRV_PROCESS_MESSAGE_CASE(rspid_UID_BindComplete) {
         // Copy the header portion of the response
         resp->id        = (uid_msgIDs_e)pqitem->m_id;
         resp->errcode   = qip_to_ui_evtp_bindcmplt(pqitem)->m_errno;
         resp->handle    = qip_to_ui_evtp_bindcmplt(pqitem)->m_devhandle;
         resp->context   = qip_to_ui_evtp_bindcmplt(pqitem)->m_context;
         resp->tranID    = qip_to_ui_evtp_bindcmplt(pqitem)->m_tranID;
         resp->size      = pqitem->m_length;

         // Copy the body of the message
         if ( ( resp->size + sizeof(struct aalui_ioctlreq) ) > Outbufsize ) {
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         memcpy((char *)resp + sizeof(struct aalui_ioctlreq), &qip_to_ui_evtp_bindcmplt(pqitem)->m_extargs, (size_t)resp->size);

         //Destroy the event
         uidrv_event_bindcmplt_destroy(qip_to_ui_evtp_bindcmplt(pqitem));
      } break; // case rspid_UID_BindComplete

      //----------------
      // UnBind Complete
      //----------------
      UIDRV_PROCESS_MESSAGE_CASE(rspid_UID_UnbindComplete) {
         // Copy the header portion of the request back
         resp->id      = (uid_msgIDs_e)pqitem->m_id;
         resp->errcode = qip_to_ui_evtp_unbindcmplt(pqitem)->m_errno;
         resp->context = qip_to_ui_evtp_unbindcmplt(pqitem)->m_context;
         resp->tranID  = qip_to_ui_evtp_unbindcmplt(pqitem)->m_tranID;
         resp->handle  = NULL;
         resp->size    = 0;

         // Destroy the event
         uidrv_event_Unbindcmplt_destroy(qip_to_ui_evtp_unbindcmplt(pqitem));
      } break; // case rspid_UID_UnbindComplete

      //-------------
      // AFU Response
      //-------------
      UIDRV_PROCESS_MESSAGE_CASE(rspid_AFU_Response) {
         ASSERT(NULL != preq);

         // Copy the header portion of the request back
         resp->id      = (uid_msgIDs_e)QI_QID(pqitem);
         resp->errcode = qip_to_ui_evtp_afuresponse(pqitem)->m_errnum;
         resp->handle  = qip_to_ui_evtp_afuresponse(pqitem)->m_devhandle;
         resp->context = qip_to_ui_evtp_afuresponse(pqitem)->m_context;
         resp->tranID  = qip_to_ui_evtp_afuresponse(pqitem)->m_tranID;

         if ( preq->size < QI_LEN(pqitem) ) {
            uidrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));  //BUG in Linux version
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         resp->size    = QI_LEN(pqitem);

         // Some failures do not have response structs
         if ( NULL != qip_to_ui_evtp_afuresponse(pqitem)->m_response ) {

            if ( uid_afurespSetGetCSRComplete == qip_to_ui_evtp_afuresponse(pqitem)->m_response->respID ) {
               PDEBUG("pCSRrwb = %p\n", qip_to_ui_evtp_afuresponse(pqitem)->m_response->pcsrBlk);
               PDEBUG("Num CSR get %" PRIu64 ", Num CSR Set %" PRIu64 "\n\n",
                         qip_to_ui_evtp_afuresponse(pqitem)->m_response->pcsrBlk->num_to_get,
                         qip_to_ui_evtp_afuresponse(pqitem)->m_response->pcsrBlk->num_to_set);
            }

            // Copy the body of the message to end of message
            if ( ( resp->size + sizeof(struct aalui_ioctlreq) ) > Outbufsize ) {
               ret = -EINVAL;
               PTRACEOUT_INT(ret);
               return ret;
            }

            memcpy((char *)resp + sizeof(struct aalui_ioctlreq), qip_to_ui_evtp_afuresponse(pqitem)->m_payload, (size_t)resp->size);

         } // if ( qip_to_ui_evtp_afuresponse(pqitem)->m_response !=  NULL)

         //Destroy the event
         uidrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));
      } break; // case rspid_AFU_Response

      //-------------
      // WSM Response
      //-------------
      UIDRV_PROCESS_MESSAGE_CASE(rspid_WSM_Response) {
         ASSERT(NULL != preq);

         // Copy the header portion of the request back
         resp->id      = (uid_msgIDs_e)QI_QID(pqitem);
         resp->errcode = qip_to_ui_evtp_afuwsevent(pqitem)->m_errnum;
         resp->handle  = qip_to_ui_evtp_afuwsevent(pqitem)->m_devhandle;
         resp->context = qip_to_ui_evtp_afuwsevent(pqitem)->m_context;
         resp->tranID  = qip_to_ui_evtp_afuwsevent(pqitem)->m_tranID;

         if ( preq->size < QI_LEN(pqitem) ) {
            ret = -EFAULT;
            PTRACEOUT_INT(ret);
            return ret;
         }

         resp->size = QI_LEN(pqitem);

         // Copy the body of the message
         if ( ( resp->size + sizeof(struct aalui_ioctlreq) ) > Outbufsize ) {
            ret = -EINVAL;
            PTRACEOUT_INT(ret);
            return ret;
         }

         memcpy((char *)resp + sizeof(struct aalui_ioctlreq), qip_to_ui_evtp_afuwsevent(pqitem)->m_payload, (size_t)resp->size);

         //Destroy the event
         uidrv_event_afucwsevent_destroy(qip_to_ui_evtp_afuwsevent(pqitem));

      } break; // case rspid_WSM_Response

      default : {
         ret = -EINVAL;
         PTRACEOUT_INT(ret);
         return ret;
      } break;
   } // switch (pqitem->m_id)

   *pOutbufsize = resp->size + sizeof(struct aalui_ioctlreq);

   PTRACEOUT_INT(ret);
   return ret;
} // ccidrv_process_message

//=============================================================================
// Name: ccidrv_sendevent
// Description: Implements the PIP UI driver message handler
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
btInt
ccidrv_sendevent(btObjectType       sesHandle, // Session handle  TODO this and context may be redundant
                struct aal_device *devp,      // Send a message to the device
                struct aal_q_item *eventp,    // Event  (Will be a qitem)
                btObjectType       context)
{
   btInt                 ret   = 0;
   struct ccidrv_session *psess = (struct ccidrv_session *)sesHandle;

   UNREFERENCED_PARAMETER(context);
   UNREFERENCED_PARAMETER(devp);


   PTRACEIN;
   ASSERT(NULL != psess);

   if ( NULL != eventp ) {

      if ( kosal_sem_get_user_alertable(&psess->m_sem) ) { /* FIXME */ }
      PDEBUG("Waking Up AIA with event\n");

      // Enqueue the completion event
      _aal_q_enqueue(eventp, &psess->m_eventq);

      // Unblock select() calls.
      kosal_wake_up_interruptible( &psess->m_waitq);
      kosal_sem_put(&psess->m_sem);
   }

   PTRACEOUT_INT(ret);
   return ret;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              UTILITY METHODS              ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================


//=============================================================================
// Name: ccidrv_getwsid
// Description: Allocates a WSID object
// Interface: public
// Inputs: pdev - aal_device pointer
//         id - Workspace Manager assigned ID
// Outputs: none.
// Comments:
//=============================================================================
struct aal_wsid* ccidrv_getwsid(struct aal_device *pdev,
                               unsigned long long id)
{
   struct aal_wsid * pwsid = NULL;
   int status;

#ifdef __i386__
   pwsid = (struct aal_wsid * )__get_free_page(GFP_KERNEL);
#else
   pwsid = (struct aal_wsid * )kosal_kmalloc(sizeof(struct aal_wsid));
#endif

   if(unlikely (pwsid == NULL) ){
      DPRINTF (UIDRV_DBG_FILE, ": failed to malloc WSID object\n");
      return NULL;
   }

   DPRINTF (UIDRV_DBG_FILE, ": Created WSID %p for device %p id %llx \n", pwsid, pdev, id);
   pwsid->m_device = pdev;
   pwsid->m_id = id;
   kosal_list_init(&pwsid->m_list);
   kosal_list_init(&pwsid->m_alloc_list);

   /* get list manipulation semaphore */
   status = kosal_sem_get_krnl_alertable(&thisDriver.wsid_list_sem);
   if (0 != status) {
      DPRINTF (UIDRV_DBG_FILE, ": couldn't add WSID to alloc_list\n");
#ifdef __i386__
      free_page(pwsid);
#else
      kosal_kfree(pwsid,sizeof(struct aal_wsid));
#endif
      return NULL;
   }

   /* add to allocated list */
   kosal_list_add_head(&pwsid->m_alloc_list, &thisDriver.wsid_list_head);

   /* release semaphore */
   kosal_sem_put(&thisDriver.wsid_list_sem);

   return pwsid;
}


/** @brief free the WSID object
 * @param[in] pwsid pointer to WSID object to free.
 * @return zero if successful, -EINTR if couldn't get list manipulation
 * lock, -EINVAL if workspace ID appears to be invalid, -EBUSY if still
 * on an ownership list */
btInt ccidrv_freewsid(struct aal_wsid *pwsid)
{
   int status;

   /* cheap paranoia. */
   if (NULL == pwsid) {
      return -EINVAL;
   }

   /* check if wsid is on an ownership list */
   if (!kosal_list_is_empty(&pwsid->m_list)) {
      DPRINTF (UIDRV_DBG_FILE, ": wsid %p appears to be on an "
         "ownership list; not freeing\n", pwsid);
      return -EBUSY;
   }

   /* search for the provided wsid on the known list */
   status = ccidrv_valwsid(pwsid);
   if (0 != status) {
      return status;
   }

   status = kosal_sem_get_krnl_alertable(&thisDriver.wsid_list_sem);
   if (0 != status) {
      return status;
   }
   kosal_list_del(&pwsid->m_alloc_list);
   kosal_sem_put(&thisDriver.wsid_list_sem);

#ifdef __i386__
   free_page(pwsid);
#else
   kosal_kfree(pwsid, sizeof(struct aal_wsid *));
#endif

   return 0;
}

//=============================================================================
// Name: ccidrv_flush_eventqueue
// Description: flush the event queue
// Interface: private
// Inputs: psess - session pointer
// Outputs: none.
// Comments:
//=============================================================================
int ccidrv_flush_eventqueue(  struct ccidrv_session *psess)
{
   int ret = 0;
   struct aal_q_item *pqitem;
   DPRINTF( UIDRV_DBG_IOCTL, ": Flushing event queue\n" );
   while(!_aal_q_empty(&psess->m_eventq) ) {
      DPRINTF( UIDRV_DBG_IOCTL, ": Not Empty\n" );

      //------------------------
      // Get the request message
      //------------------------
      pqitem = _aal_q_dequeue(&psess->m_eventq);
      if(pqitem == NULL) {
         DPRINTF( UIDRV_DBG_IOCTL, ": Invalid or corrupted request on flush\n" );
         continue;
      }

      // Switch on message type
      switch (pqitem->m_id) {
         // Bind Complete
         case rspid_UID_BindComplete:  {
            uidrv_event_bindcmplt_destroy(qip_to_ui_evtp_bindcmplt(pqitem));
         }
         case rspid_UID_UnbindComplete: {
            uidrv_event_Unbindcmplt_destroy(qip_to_ui_evtp_unbindcmplt(pqitem));
            break;
         }
         case rspid_AFU_Response: {
            DPRINTF( UIDRV_DBG_IOCTL, ": Flushing Response event\n" );
            uidrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));
            break;
         }

         case rspid_WSM_Response: {
            uidrv_event_afucwsevent_destroy(qip_to_ui_evtp_afuwsevent(pqitem));
            break;
         }

         default:
            DPRINTF( UIDRV_DBG_IOCTL, ": Encountered unknown event while flushing - leak\n" );

      } // switch (pqitem->m_id)

   }
   return ret;
}


/** @brief check if a provided wsid is on the list of known allocated wsids
 * @param[in] wsid_p pointer to workspace to validate
 * @return zero on success
 * grab the list lock, walk the list, and compare pointers. */
int ccidrv_valwsid(struct aal_wsid *wsid_p)
{
   int retval = -EINVAL;
   int status;
   struct aal_wsid *listwsid_p;

   if (NULL == wsid_p) {
      PERR(": wsid was NULL\n");
      return retval;
   }

   status = kosal_sem_get_krnl_alertable(&thisDriver.wsid_list_sem);
   if (0 != status) {
      PERR(": couldn't get list semaphore\n");
      return status;
   }

   kosal_list_for_each_entry(listwsid_p, &thisDriver.wsid_list_head, m_alloc_list, struct aal_wsid) {
      if (listwsid_p == wsid_p) {
         retval = 0;
         break;
      }
   }

   kosal_sem_put(&thisDriver.wsid_list_sem);

   PINFO(": wsid %p%s on list\n", wsid_p, (retval >= 0 ? "" : " not"));

   return retval;
}

/** @brief search for a given wsid in the provided uidrv_session
 * @param[in] uidrv_sess_p pointer to uidrv session to dig through
 * @param[in] wsid_p pointer to workspace ID to check
 * @return NULL if pointer is not found, wsid_p if it is
 *
 * both input pointers are assumed already to be non-NULL.
 *
 * should this functionality be part of the workspace manager?  why are wsids
 * even leaked out of the workspace manager?  shouldn't everything out here be
 * manipulated through completely opaque workspace IDs (long long int)?
 */
struct aal_wsid *find_wsid(const struct ccidrv_session *ccidrv_sess_p,
                           struct aal_wsid *check_wsid_p)
{
   const struct aaldev_owner *owner_p;
   const struct aaldev_ownerSession *ownersess_p;
   const struct aal_wsid *cur_wsid_p = NULL;


   /* start by checking if the passed wsid is even valid */
   if (0 != ccidrv_valwsid(check_wsid_p)) {
      return NULL;
   }

   /* if this session is not associated with a device, don't bother checking
    * ownership of the wsid, since there may not be any.  */
   if (kosal_list_is_empty(&ccidrv_sess_p->m_devicelist)) {
      return check_wsid_p;
   }

   /* if this session is associated with a device, (IE m_devicelist is not
    * empty,) then any wsid we handle needs to be on one of our device's
    * ownership lists, otherwise we shouldn't be touching it.
    *
    * struct ccidrv_session contains list head of
    *   struct aaldev_owner which contains
    *     struct aaldev_ownerSession which contains the list head of
    *       struct aal_wsid */
   DPRINTF( UIDRV_DBG_MMAP, "looking at list head %p for wsid %p\n",
      &(ccidrv_sess_p->m_devicelist), check_wsid_p );
   kosal_list_for_each_entry(owner_p, &(ccidrv_sess_p->m_devicelist), m_devicelist, struct aaldev_owner) {
      DPRINTF( UIDRV_DBG_MMAP, "examining owner_p %p\n", owner_p);
      ownersess_p = &(owner_p->m_sess);

      kosal_list_for_each_entry(cur_wsid_p, &(ownersess_p->m_wshead), m_list, struct aal_wsid) {
         if (cur_wsid_p == check_wsid_p) {
            DPRINTF( UIDRV_DBG_MMAP, "  wsid ID %lld at %p found\n",
               cur_wsid_p->m_id, check_wsid_p);
            return check_wsid_p;
         }
      }
   }

   DPRINTF( UIDRV_DBG_MMAP, "wsid %p NOT found on any owner lists\n",
      check_wsid_p);
#if 0
   return NULL;
#else
   return check_wsid_p;
#endif
}

