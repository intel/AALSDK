//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
//      AUTHOR: Joseph Grecco, Intel Corporation
//
// PURPOSE:  This file contains the OS independent code for the
//           Accelerator Abstraction Layer (AAL)
//           User Mode Interface for the AAL CCI device driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/203/2015     JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS UIDRV_DBG_MOD


#include "cci_pcie_driver_umapi.h"
#include "ccipdrv-events.h"

// Prototypes
struct ccidrv_session * ccidrv_session_create(btPID );
btInt ccidrv_session_destroy(struct ccidrv_session * );
btInt ccidrv_messageHandler( struct ccidrv_session  *,
                             btUnsigned32bitInt     ,
                             struct ccipui_ioctlreq *,
                             btWSSize               ,
                             struct ccipui_ioctlreq *,
                             btWSSize              *);

btInt ccidrv_sendevent( struct aaldev_ownerSession *,
                        struct aal_q_item *);

btInt ccidrv_flush_eventqueue(  struct ccidrv_session *psess);

btInt process_send_message(struct ccidrv_session  *,
                           struct ccipui_ioctlreq *,
                           struct ccipui_ioctlreq *,
                           btWSSize               *);

btInt process_bind_request( struct ccidrv_session  *psess,
                            struct ccipui_ioctlreq *preq);
btInt ccidrv_marshal_upstream_message( struct ccipui_ioctlreq *preq,
                                       struct aal_q_item     *pqitem,
                                       struct ccipui_ioctlreq *resp,
                                       btWSSize              *pOutbufsize);
struct aal_wsid *ccidrv_valwsid(btWSID);

extern struct um_driver umDriver;

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

       // Device owner session is the instance of the interfaces
       // for this device/session binding
       ownerSessp = dev_OwnerSession(pdev,psess->m_pid);

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
    if (kosal_sem_get_krnl_alertable( &umDriver.m_qsem)){
       PERR("Failed to claim semaphore.  FATAL ERROR!\n");
       return -EIO;
    }
    kosal_list_del(&psess->m_sessions);
    kosal_sem_put( &umDriver.m_qsem);

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
//         presp - Pointer to response (output buffer)
//         pOutbufSize - Pointer to output buffer size
// Outputs: number of bytes in payload to return
// Returns: status code: 0 == success
// Comments: This function is responsible for making sure the response header
//           is updated. E.g., Just returning a non-zero OutputBufsize does
//           not update the response header size field.
//           This is because the AALUID_IOCTL_GETMSG_DESC sets the size field
//           in the header but does NOT return any payload.
//=============================================================================
btInt
ccidrv_messageHandler( struct ccidrv_session  *psess,
                       btUnsigned32bitInt      cmd,
                       struct ccipui_ioctlreq *preq,
                       btWSSize                InbufSize,
                       struct ccipui_ioctlreq *presp,
                       btWSSize               *pOutbufSize)
{

   // Variables used in the Device Allocate Messages
   struct aal_q_item *pqitem = NULL; // Generic request queue item

   // response buffer size (will be used below)
   btWSSize OutbufSize = 0;

#if 1
# define UIDRV_IOCTL_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_IOCTL_CASE(x) case x :
#endif

   PTRACEIN;

   UNREFERENCED_PARAMETER(InbufSize);

   ASSERT(NULL != presp);
   ASSERT(NULL != pOutbufSize);
   if ( presp == NULL ) return -EINVAL;
   if ( pOutbufSize == NULL ) return -EINVAL;

   // Save the response buffer size
   OutbufSize = *pOutbufSize;

   // Assume no payload to return
   *pOutbufSize = 0;

   // Process the message
   switch ( cmd ) {

      // Get next queued message descriptor
      // Returns infromation about teh upstream
      //  message on the queue without returning the actual message
      //--------------------------------------
      UIDRV_IOCTL_CASE(AALUID_IOCTL_GETMSG_DESC) {
         // Make sure there is a message to be had
         if ( _aal_q_empty(&psess->m_eventq) ) {
            PTRACEOUT_INT(-EAGAIN);
            return -EAGAIN;
         }

         // Peek the head of the message queue
         pqitem = _aal_q_peek(&psess->m_eventq);
         if ( NULL == pqitem ) {
            PERR("Corrupt event queue\n");
            PTRACEOUT_INT(-EFAULT);
            return -EFAULT;
         }

         // Return the type and total size of the message that will be returned
         //  but no paylod is returned so leave *pOutbufSize = 0
         presp->id   = (uid_msgIDs_e)QI_QID(pqitem);
         presp->size = QI_LEN(pqitem);

         PVERBOSE("Getting Message Decriptor - size = %" PRIu64 "\n", presp->size);

         PTRACEOUT_INT(0);
      } return 0; // case AALUID_IOCTL_GETMSG_DESC:


      // Get the next message off of the queue
      // returns a copy of the message and moves the item to
      // the pending queue
      //----------------------------------------------------
      UIDRV_IOCTL_CASE(AALUID_IOCTL_GETMSG) {
         // Make sure there is a message to be had
         if ( _aal_q_empty(&psess->m_eventq) ) {
            PERR("No Message available\n");
            PTRACEOUT_INT(-EAGAIN);
            return -EAGAIN;
         }

         //------------------------
         // Get the request message
         //------------------------
         pqitem = _aal_q_dequeue(&psess->m_eventq);
         if ( NULL == pqitem ) {
            PERR("Invalid or corrupted request\n");
            PTRACEOUT_INT(-EFAULT);
            return -EFAULT;
         }

         // Process the request.
         //   Function will update response header and pOutbufSize, Restore the
         //   value of pOutBufSize
         *pOutbufSize = OutbufSize;
         return ccidrv_marshal_upstream_message(preq, pqitem, presp, pOutbufSize);

      } break; // case  AALUID_IOCTL_GETMSG:


      // Send the message to the device or PIP (SW driver)
      //-------------------------------------------------
      UIDRV_IOCTL_CASE(AALUID_IOCTL_SENDMSG) {
         // Process the request. Function will update response header and pOutbufSize
         *pOutbufSize = OutbufSize;
         return process_send_message(psess, preq, presp, pOutbufSize);
      } break;

      // Process Bind device
      //--------------------
      UIDRV_IOCTL_CASE(AALUID_IOCTL_BINDDEV) {
         // Process the request. Function will update response header and pOutbufSize
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

   PTRACEOUT_INT(-1);
   return -1;
}

//=============================================================================
// Name: process_send_message
// Description: Process a send request
// Interface: public
// Inputs: psess - session
//         preq - request header
// Outputs: pOutbufSize must be set to size of payload to return or zero if none
// Comments:
//=============================================================================
btInt
process_send_message(struct ccidrv_session  *psess,
                     struct ccipui_ioctlreq *preq,
                     struct ccipui_ioctlreq *presp,
                     btWSSize               *pOutbufSize)
{
   btInt                              ret = 0;
   struct aal_device                 *pdev;
   struct aaldev_ownerSession        *ownSessp;
   struct aal_pipmessage              pipMessage;
   ui_shutdownreason_e                shutdown_reason;
   btTime                             timeleft;

//   btWSSize                             messagesize;

#if 1
# define UIDRV_PROCESS_SEND_MESSAGE_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_SEND_MESSAGE_CASE(x) case x :
#endif

   PTRACEIN;
   ASSERT(NULL != psess);
   ASSERT(NULL != preq);

   if( (NULL == psess) || (NULL == preq)) {
      PERR("Invalid Input parameter \n");
      ret = -EINVAL;
      return ret ;
   }

   // Process the request copying in remaining request arguments as appropriate
   //--------------------------------------------------------------------------
   switch ( preq->id ) {

      // Send a message to the AFU via the PIP message handler
      //------------------------------------------------------
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
            PERR("Not owner or no message handler during Process Send Message.\n");
            ret = -EACCES;
            PTRACEOUT_INT(ret);
            return ret;
         }
         PDEBUG("Got owner %p\n", ownSessp);

         // Wrap the message and transaction identification
         //  pipMessage is a generic message wrapper for all
         //  PIP message handlers
         pipMessage.m_message       = aalui_ioctlPayload(preq);
         pipMessage.m_response      = aalui_ioctlPayload(presp);
         pipMessage.m_respbufSize   = *pOutbufSize;
         pipMessage.m_tranID        = preq->tranID;
         pipMessage.m_context       = preq->context;
         pipMessage.m_errcode       = uid_errnumOK;

         // Send the message on to the device specific command handler.
         //  This macro resolves to calling the low level, device specific, command
         //  handler called the Physical Interface Protocol (PIP).  This enabled devices
         //  served by this driver have custom low level drivers.
         ret = aalsess_pipSendMessage(ownSessp)(ownSessp, &pipMessage);

         // If there is data to return the size of the response payload
         //   as long as it will fit
         if(pipMessage.m_respbufSize <= *pOutbufSize ){
            *pOutbufSize = pipMessage.m_respbufSize;
         }else{
            *pOutbufSize =0;
         }

         // It is the responsibility of this function to update the
         //   response header as the low-level driver (PIP) does not
         //   see the message header.
         presp->size             = *pOutbufSize;
         presp->errcode          = pipMessage.m_errcode;

         PTRACEOUT_INT(ret);
      } return ret; // case reqid_UID_SendAFU

      // Shutdown the driver for this process
      //-------------------------------------
      UIDRV_PROCESS_SEND_MESSAGE_CASE(reqid_UID_Shutdown) {
         // Create shutdown  request object
         struct ccipdrv_event_afu_response_event *newreq;

         shutdown_reason =((struct aalui_Shutdown*) aalui_ioctlPayload(preq))->m_reason;

         // Assume kernel shutdown takes zero time for now so return original
         //   timeout value in event which indicates how much time as actually
         //   used to shutdown (timeout-amount used(0) = timeleft
         timeleft = ((struct aalui_Shutdown*)aalui_ioctlPayload(preq))->m_timeout;

         PDEBUG("Received a shutdown ioctl reason %d\n", shutdown_reason);
         newreq = ccipdrv_event_shutdown_event_create( shutdown_reason,
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
         _aal_q_enqueue(ui_evtp_afuresponse_to_qip(newreq), &psess->m_eventq);

         // Unblock select() calls.
         kosal_wake_up_interruptible( &psess->m_waitq);

         *pOutbufSize =0;
      } break; // case reqid_UID_Shutdown

      default : {
         PERR("Unrecognized send message option %d\n", preq->id);
         PTRACEOUT_INT(-EINVAL);
         *pOutbufSize =0;
      } return -EINVAL;

   } // switch (preq->id)

   PTRACEOUT_INT(0);
   return 0;
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
                     struct ccipui_ioctlreq *preq)
{
   struct aal_device                            *pdev         = NULL;
   struct ccipdrv_event_afu_response_event      *bindcmplt    = NULL;
   struct ccipdrv_event_afu_response_event      *unbindcmplt  = NULL;
   struct ccipdrv_DeviceAttributes               bindevt      = {0};
   struct aaldev_ownerSession                   *ownerSessp   = NULL;
   btInt                                         ret          = 0;

#if 1
# define UIDRV_PROCESS_BIND_REQUEST_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_BIND_REQUEST_CASE(x) case x :
#endif

   ASSERT(NULL != psess);
   ASSERT(NULL != preq);

   if( (NULL == psess) || (NULL == preq)) {
      PERR("Invalid Input parameter \n");
      ret = -EINVAL;
      return ret ;
   }

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
            bindcmplt = ccipdrv_event_bindcmplt_create(NULL, NULL, uid_errnumDeviceHasNoPIPAssigned, preq);
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
            bindcmplt = ccipdrv_event_bindcmplt_create(NULL, NULL, uid_errnumNotDeviceOwner, preq);
            goto BIND_DONE;
         }

         // Set the owner session - Through the owner session
         //  the system can always get to the device (downstream)
         //  and the UIdrv session (upstream).
         ownerSessp->m_device   = pdev;                  // Device
         ownerSessp->m_UIHandle = psess;                 // This session
         ownerSessp->m_ownerContext =  *((btVirtAddr*)preq->payload);     // Used by AIA for routing

         PDEBUG("Owner Context %p\n",  *((btVirtAddr*)preq->payload));

         //---------------------------------------------------
         // Bind the PIP to the session
         //   Allows the PIP to do anything it needs to record
         //   the presence of this session.
         //---------------------------------------------------
         if ( unlikely( !aaldev_pipmsgHandlerp(pdev)->bindSession(ownerSessp) ) ) {
            PERR("uid_errnumCouldNotBindPipInterface\n");
            // Create error event
            bindcmplt = ccipdrv_event_bindcmplt_create(NULL, NULL, uid_errnumCouldNotBindPipInterface, preq);
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
         kosal_sem_get_user_alertable( &psess->m_sem );
         if ( unlikely( aaldev_addowner_OK != dev_updateOwner(pdev,                 // Device
                                                              psess->m_pid,         // Process ID
                                                              ownerSessp,           // New session attributes
                                                              &psess->m_devicelist) // Head of our session device list
                      )
            ) {
            PERR("Failed to update owner: uid_errnumCouldNotClaimDevice\n");
            // Create error event
            bindcmplt = ccipdrv_event_bindcmplt_create(NULL, NULL, uid_errnumCouldNotClaimDevice, preq);
         } else {
            // Fill out the extended bind parameters
            bindevt.m_mappableAPI = aaldev_mappableAPI(pdev);
            bindevt.m_size =0;

            PDEBUG("Creating bind event MAPPABLE = 0x%x\n", bindevt.m_mappableAPI);

            // Create the completion event
            bindcmplt = ccipdrv_event_bindcmplt_create(preq->handle, &bindevt, uid_errnumOK, preq);
         }
         kosal_sem_put( &psess->m_sem );
         ret = 0;
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
            PERR("Not owner or no message handler during UnBind Request.\n");
            ret = -EACCES;
            PTRACEOUT_INT(ret);
            return ret;
         }

         // Unbind the PIP from the session
         if ( unlikely( !aaldev_pipmsgHandlerp(pdev)->unBindSession(ownerSessp) ) ) {
             PERR("uid_errnumCouldNotUnBindPipInterface\n");
             // Create error event
             unbindcmplt = ccipdrv_event_Unbindcmplt_create(uid_errnumCouldNotUnBindPipInterface,preq);
             ret = -EINVAL;
             goto UNBIND_DONE;
         }

         // Update the owner's list
         if ( unlikely( !dev_removeOwner(pdev, psess->m_pid) ) ) {
            PERR("Failed to update owner\n");
            unbindcmplt = ccipdrv_event_Unbindcmplt_create(uid_errnumNotDeviceOwner, preq);
            ret = -EINVAL;
         } else {
            unbindcmplt = ccipdrv_event_Unbindcmplt_create(uid_errnumOK, preq);
         }
         ret = 0;
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
   _aal_q_enqueue(ui_evtp_afuresponse_to_qip(bindcmplt), &psess->m_eventq);
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
   _aal_q_enqueue(ui_evtp_afuresponse_to_qip(unbindcmplt), &psess->m_eventq);

   // Unblock select() calls.
   kosal_wake_up_interruptible( &psess->m_waitq );

   PTRACEOUT_INT(ret);
   return ret;
}  // process_bind_request

//=============================================================================
// Name: ccidrv_marshal_upstream_message
// Description: Pre-process a queued message targeted for the application.
//              Called from AALUID_IOCTL_GETMSG, the message is unpacked,
//              any kernel level functions performed and the user mode event
//              parameters are returned.
// Interface: private
// Inputs: unsigned long arg - pointer to user space event target
//         struct ccipui_ioctlreq *preq - request header
//         struct aal_q_item *pqitem - message to process
// Outputs: length of output buffer is copied to *Outbufsize.
//          return code: 0 == success
// Comments: Kernel event is destroyed
//=============================================================================
btInt
ccidrv_marshal_upstream_message( struct ccipui_ioctlreq *preq,
                                 struct aal_q_item     *pqitem,
                                 struct ccipui_ioctlreq *resp,
                                 btWSSize              *pOutbufsize)
{
   btInt    ret = 0;

#if 1
# define UIDRV_PROCESS_MESSAGE_CASE(x) case x : PDEBUG("%s\n", #x);
#else
# define UIDRV_PROCESS_MESSAGE_CASE(x) case x :
#endif

   PTRACEIN;

   ASSERT(NULL != pqitem);
   ASSERT(NULL != resp);
   ASSERT(NULL != pOutbufsize);
   ASSERT(NULL != preq);

   if((NULL == pqitem) || (NULL == resp) || (NULL == pOutbufsize) || (NULL == preq)){
      PERR("Invalid input argument");
      return -EINVAL;
   }

   // Copy the header portion of the request back
   resp->id      = (uid_msgIDs_e)QI_QID(pqitem);
   resp->errcode = qip_to_ui_evtp_afuresponse(pqitem)->m_errnum;
   resp->handle  = qip_to_ui_evtp_afuresponse(pqitem)->m_devhandle;
   resp->context = qip_to_ui_evtp_afuresponse(pqitem)->m_context;
   resp->tranID  = qip_to_ui_evtp_afuresponse(pqitem)->m_tranID;

   // Make sure there is room for the payload
   if ( *pOutbufsize < QI_LEN(pqitem) ) {
      PERR("No room for event payload. Outbuf payload size = %d Event Payload = %d\n",(int) *pOutbufsize,  (int) QI_LEN(pqitem));
      ccipdrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));
      *pOutbufsize = 0;
      PTRACEOUT_INT(ret);
      return -EINVAL;
   }

   // Payload size
   *pOutbufsize = resp->size = QI_LEN(pqitem);

   // Copy the payload
   memcpy(resp->payload, qip_to_ui_evtp_afuresponse(pqitem)->m_payload, (size_t)resp->size);

   PVERBOSE("Sending Event Event ID = %d\n",((struct aalui_WSMEvent*)(resp->payload))->evtID );

   //Destroy the event
   ccipdrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));

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
ccidrv_sendevent(struct aaldev_ownerSession * pOwnerSession,
                 struct aal_q_item *eventp)
{
   btInt                 ret   = 0;
   struct ccidrv_session *psess = (struct ccidrv_session *)pOwnerSession->m_UIHandle;

   PTRACEIN;
   ASSERT(NULL != psess);
   if( NULL == psess) {
      PERR("Invalid Input parameter \n");
      ret = -EINVAL;
      return ret ;
   }

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
   static btUnsigned64bitInt     nextWSID = 1;

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

   // Check the WSID for roll over. This gives us a very large number of WSIDs before
   //   roll over.
   if( 0 == wsid_to_wsidHandle(nextWSID) ){
      nextWSID = 1;
   }

   pwsid->m_device = pdev;
   pwsid->m_handle = wsid_to_wsidHandle(nextWSID);
   pwsid->m_id = id;
   kosal_list_init(&pwsid->m_list);
   kosal_list_init(&pwsid->m_alloc_list);

   PDEBUG(": Created WSID %llu [Handle %llx] for device id %llx \n", nextWSID, pwsid->m_handle, id);

   /* get list manipulation semaphore */
   status = kosal_sem_get_krnl_alertable(&umDriver.wsid_list_sem);
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
   kosal_list_add_head(&pwsid->m_alloc_list, &umDriver.wsid_list_head);

   /* release semaphore */
   kosal_sem_put(&umDriver.wsid_list_sem);

   nextWSID++;

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
   if (NULL ==  ccidrv_valwsid( pwsid_to_wsidHandle(pwsid) )) {
      return -EINVAL;
   }

   status = kosal_sem_get_krnl_alertable(&umDriver.wsid_list_sem);
   if (0 != status) {
      return status;
   }
   kosal_list_del(&pwsid->m_alloc_list);
   kosal_sem_put(&umDriver.wsid_list_sem);

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
      DPRINTF( UIDRV_DBG_IOCTL, ": Flushing Response event\n" );
      ccipdrv_event_afuresponse_destroy(qip_to_ui_evtp_afuresponse(pqitem));
   }
   return ret;
}

//=============================================================================
// Name: ccidrv_valwsid
/** @brief check if a provided wsid is on the list of known allocated wsids
 * @param[in] wsidHandle handle to workspace to validate
 * @return zero on success
 * grab the list lock, walk the list, and compare pointers. */
//=============================================================================
struct aal_wsid *ccidrv_valwsid(btWSID wsidHandle)
{
   int status;
   struct aal_wsid *listwsid_p;

   if( 0 == wsidHandle) {
      PERR(": wsid was 0\n");
      return NULL;
   }

   status = kosal_sem_get_krnl_alertable(&umDriver.wsid_list_sem);
   if (0 != status) {
      PERR(": couldn't get list semaphore\n");
      kosal_sem_put(&umDriver.wsid_list_sem);
      return NULL;
   }

   kosal_list_for_each_entry(listwsid_p, &umDriver.wsid_list_head, m_alloc_list, struct aal_wsid) {
      if (listwsid_p->m_handle == wsidHandle) {
         kosal_sem_put(&umDriver.wsid_list_sem);
         return listwsid_p;
      }
   }

   kosal_sem_put(&umDriver.wsid_list_sem);

   PINFO(": wsid %llu not on list\n", wsidHandle);

   return NULL;
}

/** @brief search for a given wsid in the provided uidrv_session
 * @param[in] ccidrv_sess_p pointer to uidrv session to dig through
 * @param[in] wsidHandle to workspace ID to check
 * @return NULL if pointer is not found, wsid_p if it is
 *
 * both input pointers are assumed already to be non-NULL.
 *
 * should this functionality be part of the workspace manager?  why are wsids
 * even leaked out of the workspace manager?  shouldn't everything out here be
 * manipulated through completely opaque workspace IDs (long long int)?
 */
struct aal_wsid *find_wsid( const struct ccidrv_session *ccidrv_sess_p,
                            btWSID wsidHandle)
{
   const struct aaldev_owner *owner_p;
   const struct aaldev_ownerSession *ownersess_p;
   struct aal_wsid *cur_wsid_p = NULL;

   PDEBUG("Looking for WSID Handle %llx\n", wsidHandle);

   /* start by checking if the passed wsid is even valid */
   if (NULL == ccidrv_valwsid(wsidHandle)) {
      PERR("WSID Invalid\n");
      return NULL;
   }

   /* if this session is not associated with a device, don't bother checking
    * ownership of the wsid, since there may not be any.  */
   if (kosal_list_is_empty(&ccidrv_sess_p->m_devicelist)) {
      return ccidrv_valwsid(wsidHandle);
   }

   /* if this session is associated with a device, (IE m_devicelist is not
    * empty,) then any wsid we handle needs to be on one of our device's
    * ownership lists, otherwise we shouldn't be touching it.
    *
    * struct ccidrv_session contains list head of
    *   struct aaldev_owner which contains
    *     struct aaldev_ownerSession which contains the list head of
    *       struct aal_wsid */
  PVERBOSE("looking at list head %p for wsid %llx\n", &(ccidrv_sess_p->m_devicelist), wsidHandle );
   kosal_list_for_each_entry(owner_p, &(ccidrv_sess_p->m_devicelist), m_devicelist, struct aaldev_owner) {
      PVERBOSE("examining owner_p %p\n", owner_p);
      ownersess_p = &(owner_p->m_sess);

      kosal_list_for_each_entry(cur_wsid_p, &(ownersess_p->m_wshead), m_list, struct aal_wsid) {
         if (cur_wsid_p->m_handle == wsidHandle) {
            PVERBOSE("  wsid ID %lld at %p found\n",cur_wsid_p->m_handle, cur_wsid_p);
            return cur_wsid_p;
         }
      }
   }

   PVERBOSE("wsid %llu NOT found on any owner lists\n", wsidHandle);

   return NULL;
}

