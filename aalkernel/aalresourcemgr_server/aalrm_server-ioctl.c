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
//        FILE: aalrm_server-ioctl.c
//     CREATED: 02/21/2008
//      AUTHOR: Joseph Grecco 	- Intel
//      		Alvin Chen		- Intel
//
// PURPOSE:  IOCTL operations
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-21-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/23/2009     JG       Initial code clean up
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 06/30/2009     AC       Copy all user space memory to local instead of using
//                            user space pointer directly
// 10/12/2009     JG       Major changes to processDeviceRequest began
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMS_DBG_IOCTL

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalrm_server-int.h"
#include "aalsdk/kernel/aalrm_server-services.h"
#include "aalsdk/kernel/aaldevice.h"
#include "aalrms-events-int.h"



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////      RESOURCE MANAGER SERVER CALLBACK METHODS       /////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrms_event_config_update
// Description: Process a config update event
// Interface: private
// Inputs:   pdev - AAL device
//           updateType - Type of update to the device state
//           pid - Process ID if applicable
//           context - User context
// Outputs: none.
// Comments:
//=============================================================================
void aalrms_event_config_update(struct aal_device *pdev,
                                krms_cfgUpDate_e   updateType,
                                btPID              pid,
                                void              *context)
{
   struct rms_reqq_config_update_event* newreq;
   struct aaldev_owner *itr, *tmp;
   btPID *nextowner;

   //-------------------------------------------------------------
   // Create queue request object -
   //  Includes size of constant structure and array of owner pids
   //-------------------------------------------------------------
   struct aalrms_configUpDateEvent * pcfgUpdate = (struct aalrms_configUpDateEvent *)
		   kosal_kmalloc(( sizeof(struct aalrms_configUpDateEvent) + (pdev->m_numowners * sizeof(btPID)) ));

   DPRINTF (AALRMS_DBG_IOCTL, ": Update Event\n");

   //--------------------------------------
   // Set the constant portion of the event
   //--------------------------------------
   pcfgUpdate->id  = updateType;
   pcfgUpdate->pid = pid;

   pcfgUpdate->devattrs.state       = pdev->m_devstate;     // Device state
   pcfgUpdate->devattrs.Handle      = pdev;                 // Device Handle
   pcfgUpdate->devattrs.devid       = pdev->m_devid;        // Copy over the device ID
   pcfgUpdate->devattrs.maxOwners   = pdev->m_maxowners;    // Max mumber of owners

   //---------------------------------------------
   // Set the variable length portion of the event
   //   this consists  of an array of PIDs
   //---------------------------------------------
   pcfgUpdate->devattrs.numOwners   = pdev->m_numowners;    // Number of owners


   nextowner = pcfgUpdate->devattrs.ownerlist;  // The start of the variable length portion
   DPRINTF (AALRMS_DBG_IOCTL, ": Walkinglist %p, NumOwners %d, Head %p, Next %p\n", nextowner, pdev->m_numowners, &pdev->m_ownerlist, pdev->m_ownerlist.next);

   if(0 != pdev->m_numowners){
      // Fill in the owner list - Walk the ownerlist and copy the pid for each
      kosal_list_for_each_entry_safe(itr, tmp, &pdev->m_ownerlist, m_ownerlist, struct aaldev_owner) {
         DPRINTF(AALRMS_DBG_IOCTL, "About to look at itr %p, nextowner %p\n", itr, nextowner);
         if (itr) {
            *nextowner++ = itr->m_pid; // The owner is identified by pid. Save in increment the pointer
         }
         else {
            DPRINTF(AALRMS_DBG_IOCTL, "ITR IS NULL\n");
            break;   // GET OUT, otherwise maybe infinite
         }
      }
   }

   //---------------
   // Debug log info
   //---------------
   DPRINTF(AALRMS_DBG_IOCTL,"ID %d and %d\n",  updateType,pcfgUpdate->id );

   DPRINTF(AALRMS_DBG_IOCTL,"Handle: %p\n", pcfgUpdate->devattrs.Handle);                    // Device Handle
   DPRINTF(AALRMS_DBG_IOCTL,"vendor: %x\n", pcfgUpdate->devattrs.devid.m_vendor);            // Vendor code
   DPRINTF(AALRMS_DBG_IOCTL,"devicetype: %x\n", pcfgUpdate->devattrs.devid.m_devicetype);    // Device type

   DPRINTF(AALRMS_DBG_IOCTL,"\nADDRESS:\n");
   DPRINTF(AALRMS_DBG_IOCTL,"bustype: %x\n", pcfgUpdate->devattrs.devid.m_devaddr.m_bustype);
   DPRINTF(AALRMS_DBG_IOCTL,"busnum: %x\n", pcfgUpdate->devattrs.devid.m_devaddr.m_busnum);
   DPRINTF(AALRMS_DBG_IOCTL,"devicenum: %x\n", pcfgUpdate->devattrs.devid.m_devaddr.m_devicenum);
   DPRINTF(AALRMS_DBG_IOCTL,"funcnum: %x\n", pcfgUpdate->devattrs.devid.m_devaddr.m_functnum);
   DPRINTF(AALRMS_DBG_IOCTL,"subdevnum: %x\n\n", pcfgUpdate->devattrs.devid.m_devaddr.m_subdevnum);
   DPRINTF(AALRMS_DBG_IOCTL,"instancenum: %x\n\n", pcfgUpdate->devattrs.devid.m_devaddr.m_instanceNum);
   DPRINTF(AALRMS_DBG_IOCTL,"socketnum: %x\n\n", pcfgUpdate->devattrs.devid.m_devaddr.m_socketnum);

   DPRINTF(AALRMS_DBG_IOCTL,"pipGUID: %llx\n", pcfgUpdate->devattrs.devid.m_pipGUID); // PIP GUID
   DPRINTF(AALRMS_DBG_IOCTL,"ahmGUID: %llx\n", pcfgUpdate->devattrs.devid.m_ahmGUID); // AHM GUID
   DPRINTF(AALRMS_DBG_IOCTL,"afuGUIDH: %llx\n", pcfgUpdate->devattrs.devid.m_afuGUIDh); // AFU GUID
   DPRINTF(AALRMS_DBG_IOCTL,"afuGUIDL: %llx\n", pcfgUpdate->devattrs.devid.m_afuGUIDl); // AFU GUID

   DPRINTF(AALRMS_DBG_IOCTL,"Owners: %d\n", pcfgUpdate->devattrs.numOwners);
   DPRINTF(AALRMS_DBG_IOCTL,"MaxOwners: %d\n",pcfgUpdate->devattrs.maxOwners);


   // Create the event object
   newreq = rms_reqq_config_update_event_create(pcfgUpdate);
   if(newreq == NULL){
      DPRINTF (AALRMS_DBG_IOCTL, ": Failed to create queue object\n");
      return;
   }

   // Queue the item  - TODO Need a separate queue to make a priority queuing scheme
   aalrms_queue_req(RMSSQIP(newreq));
   return;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////         RESOURCE MANAGER SERVER  METHODS            /////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrms_process_message
// Description: Process a request message
// Interface: private
// Inputs:struct psess - session
//               arg - user args from ioctl
//               preq - aalrm_ioctlreq header
// Outputs: Message copied back to user sapce.
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
int aalrms_process_message( struct aalrm_server_session *psess,
                            unsigned long arg,
                            struct aalrm_ioctlreq  *preq)
{
   union {
      struct rms_reqq_reqdev     *preqdev;   // "Request Device" message which is of type qitem
      struct rms_reqq_registrar  *pregreq;   // "Registrar Request" message which is of type qitem
      struct rms_reqq_config_update_event *pupdate;
   }u;

   struct aal_q_item    *pqitem = NULL;
   int ret = 0;

   DPRINTF(AALRMS_DBG_IOCTL, ": Processing message\n");

   // Lock-out cancels. Do not make this interruptible - TODO perhaps shold be changed to down_killable
   kosal_sem_get_user(&rmserver.m_sem);

   //--------------------------------------------------------
   // Check the request message
   // Must use peek() so that it remains on a trackable queue
   //--------------------------------------------------------
   pqitem = aalrms_peek_req();
   if(pqitem == NULL) {
      DPRINTF( AALRMS_DBG_IOCTL, ": Invalid or corrupted request\n" );
      ret =  -EFAULT;
      goto done;
   }

   //-----------------------------------------------------------------------------
   // Update the ioctl header
   //  Since most RMSS requests are forwarded to the user mode service application
   //  the user request header is prepared up front
   //-----------------------------------------------------------------------------
   preq->id = QI_QID(pqitem);
   preq->req_handle = pqitem;

   //----------------------------------------------------------------
   // Process the request message
   // - Note the ID has already been copied into the preq structure
   //   but originally comes from the queued request message in qitem
   //----------------------------------------------------------------
   switch(preq->id) {
      //------------------
      // Registrar Request
      //------------------
      case reqid_RS_Registrar: {
         DPRINTF (AALRMS_DBG_IOCTL, ": MSG - reqid_Registrar  reqhandle %p\n",preq->req_handle);

         // Get the request from the queue item
         u.pregreq = qi_to_regreq(pqitem);

         // User should have passed the payload buffer size.  Do not exceed buffer
         if(preq->size < QI_LEN(pqitem)){
            // TODO the message should be pushed back on the request queue or at least an error response sent to the requestor
            rms_reqq_regreq_destroy(u.pregreq);
            DPRINTF(AALRMS_DBG_IOCTL, ": Buffer not large enough to accept command\n");
            ret = -EINVAL;
            goto done;
         }

         //------------------------------------------------------
         // Place the item on the pending queue
         //  Requests that have a response must be
         //  saved so that the response can be routed back to the
         //  originator. This is an atomic operation.
         //------------------------------------------------------
         aalrms_queue_move_to_pend(pqitem);

         // Copy the header portion of the request back
         if(copy_to_user ((struct aalrm_ioctlreq *)arg, preq, sizeof(struct aalrm_ioctlreq))){
            DPRINTF(AALRMS_DBG_IOCTL, ": Failed copy Message \n");
            ret = -EFAULT;
            goto done;
         }

         // Copy the payload portion of the request back
         if(QI_LEN(pqitem) !=0){
            if(copy_to_user(preq->payload,
                              &REGREQ_REGREQP(u.pregreq)->buf,
                              REGREQ_REGREQP(u.pregreq)->size) ){
               DPRINTF(AALRMS_DBG_IOCTL, ": Failed get Message \n");
               ret = -EFAULT;
               goto done;
            }
         }
         preq->size  = QI_LEN(pqitem);
         break;
      } // case reqid_RS_Registrar


      //------------------------------
      // Request a device be allocated
      //------------------------------
      case reqid_URMS_RequestDevice: {
         DPRINTF (AALRMS_DBG_IOCTL, ": MSG - reqid_URMS_RequestDevice reqhandle %p\n",preq->req_handle);

         // Get the request
         u.preqdev = qi_to_reqdev(pqitem);

         // User should have passed the payload buffer size.  Do not exceed buffer
         if(preq->size < QI_LEN(pqitem)){
            // TODO SHOULD SEND A FAILURE RESOPONSE TO REQUESTOR NOT DELETE MYSELF AND REMOVE FROM PENDING
            kosal_kfree(u.preqdev->m_reqdev, sizeof(struct req_allocdev)); rms_reqq_reqdev_destroy(u.preqdev);
            DPRINTF(AALRMS_DBG_IOCTL, ": DEBUG FREEING THE QITEM AND ITEM ITSELF\n");
            ret = -EINVAL;
            goto done;
         }

         // Place the item on the pending queue atomically
         aalrms_queue_move_to_pend(pqitem);

         // Copy the header portion of the request back
         if(copy_to_user((struct aalrm_ioctlreq *) arg, preq, sizeof(struct aalrm_ioctlreq))){
              DPRINTF(AALRMS_DBG_IOCTL, ": Failed get Message \n");
              ret = -EFAULT;
              goto done;
         }

         // Copy the payload section if there is one
         if(QI_LEN(pqitem) != 0){
            if( copy_to_user( preq->payload, &REQDEV_ALLOCDEV(u.preqdev).buf, QI_LEN(pqitem)  )    ){
               DPRINTF(AALRMS_DBG_IOCTL, ": Failed copy Message\n");
               ret = -EFAULT;
               goto done;
            }
         }
         preq->size  = QI_LEN(pqitem);
         break;
      } // case reqid_URMS_RequestDevice

      //---------------------------
      // Configuration Update event
      //---------------------------
      case evtid_KRMS_ConfigUpdate: {
         pqitem = aalrms_dequeue_req();

         u.pupdate = qi_to_updateevent(pqitem);
         DPRINTF (AALRMS_DBG_IOCTL, ": CMD - rspid_KRMS_ConfigUpdate ID = %d\n",CONF_EVTP_EVENT(u.pupdate).id);
         // User should have passed the payload buffer size.  Do not exceed buffer
         if(preq->size < QI_LEN(pqitem)){
             rms_reqq_config_update_event_destroy(u.pupdate);
             DPRINTF(AALRMS_DBG_IOCTL, ": No room to return the event\n");
             ret = -EINVAL;
             goto done;
         }

         // Copy the payload section
         if(copy_to_user(  preq->payload, &CONF_EVTP_EVENT(u.pupdate), QI_LEN(pqitem)) ){
            DPRINTF(AALRMS_DBG_IOCTL, ": Failed copy Message\n");
            rms_reqq_config_update_event_destroy(u.pupdate);
            ret = -EFAULT;
            goto done;
         }
         DPRINTF (AALRMS_DBG_IOCTL, ": CMD - rspid_KRMS_ConfigUpdate - Event Destroyed\n");
         rms_reqq_config_update_event_destroy(u.pupdate);
         preq->size  = QI_LEN(pqitem);
         break;
      } // case evtid_KRMS_ConfigUpdate

      //-----------------
      // Shutdown request
      //-----------------
      case reqid_Shutdown: {
         // Copy the header portion of the request back
         DPRINTF (AALRMS_DBG_IOCTL, ": CMD - reqid_Shutdown\n");

         pqitem = aalrms_dequeue_req();

         // User should have passed the payload buffer size.  Do not exceed buffer
         if(preq->size < QI_LEN(pqitem)){
            rms_reqq_Shutdown_destroy(qi_to_shutdownevent(pqitem));
             DPRINTF(AALRMS_DBG_IOCTL, ": No room to return the event\n");
             ret = -EINVAL;
             goto done;
         }

         if(copy_to_user ((struct aalrm_ioctlreq *)arg, preq, sizeof(struct aalrm_ioctlreq))){
            DPRINTF(AALRMS_DBG_IOCTL, ": Failed copy Message \n");
            rms_reqq_Shutdown_destroy(qi_to_shutdownevent(pqitem));
            ret = -EFAULT;
            goto done;
         }
         rms_reqq_Shutdown_destroy(qi_to_shutdownevent(pqitem));
         preq->size  = QI_LEN(pqitem);
         break;
      } // case reqid_Shutdown


      default: {
         DPRINTF (AALRMS_DBG_IOCTL, ": CMD - Unexpected\n");
         pqitem = aalrms_dequeue_req();
         ret = -EINVAL;
         goto done;
         break;
      }

   } // switch(preq->id)

   done:
      kosal_sem_put(&rmserver.m_sem);
      return ret;
}

//=============================================================================
// Name: aalrms_processDeviceRequest
// Description: Process a request to be forwarded to a device such as activate
//              and deactivate
// Interface: public
// Inputs:  psess- pointer to session
//          preq - pointer to user request block
// Outputs: none.
// Comments:
//=============================================================================
int aalrms_processDeviceRequest(struct aalrm_server_session *psess,
                                struct aalrm_ioctlreq       *preq)
{
   struct aaldev_ownerSession     ownerSess;
   struct aal_pipmessage          pipMessage;
   struct rms_event_devreq_cmplt *eventp = NULL;
   struct aal_device             *pdev   = NULL;
   int                            ret    = -EINVAL;

   // Get the handle and validate
   DPRINTF(AALRMS_DBG_IOCTL, ": Getting handle %p %p\n",preq,preq->res_handle );

   pdev = aaldev_handle_to_devp(preq->res_handle);
   if ( unlikely( NULL == pdev ) ) {
      DPRINTF(AALRMS_DBG_IOCTL, ": Invalid device handle %p\n", preq->res_handle);

      eventp = rms_event_devreq_cmplt_create(rms_resultInvalidDevice,
                                             preq->tranID,
                                             preq->context);
      goto DONE;
   }

   // ------------------------------------
   // Get the default PIP message handler
   // interface and bind it to our session
   // ------------------------------------
   DPRINTF(AALRMS_DBG_IOCTL, ": Checking Interface\n");
   if ( unlikely( NULL == aaldev_pipp(pdev) ) ) {
      DPRINTF(AALRMS_DBG_IOCTL, ": rms_resultDeviceHasNoPIPAssigned\n");

      eventp = rms_event_devreq_cmplt_create(rms_resultDeviceHasNoPIPAssigned,
                                             preq->tranID,
                                             preq->context);
      goto DONE;
   }

   //--------------------------------------------
   // Initialize the UI side of the owner session
   //--------------------------------------------
   ownerSess.m_device = pdev;                                   // Device

   // Wrap the message and transaction identification
   //  pipMessage is a generic message wrapper for all
   //  PIP message handlers
   pipMessage.m_message = preq->payload;
   pipMessage.m_tranID  = preq->tranID;
   pipMessage.m_context = preq->context;

   DPRINTF(AALRMS_DBG_IOCTL, ": Sending message\n");

   // Send the message on its way.

   ret = aalsess_pipSendMessage(&ownerSess)(&ownerSess,
                                            &pipMessage);
   if ( 0 == ret ) {
      ASSERT(NULL == eventp); // memory leak otherwise.
      goto DONE;
   }

   eventp = rms_event_devreq_cmplt_create(rms_resultOK,
                                          preq->tranID,
                                          preq->context);

DONE:
   if ( NULL != eventp ) {
      aalrms_queue_req(AALQIP(eventp));
   }

   return ret;
}

//=============================================================================
// Name: aalrms_process_sendmessage
// Description: Process a send message command
// Interface: public
// Inputs:  psess- pointer to session
//          preq - pointer to user request block
// Outputs: none.
// Comments:
//=============================================================================
int aalrms_process_sendmessage(struct aalrm_server_session *psess,
                               struct aalrm_ioctlreq  *preq)
{
   struct rms_reqq_Shutdown*  newreq;

   // Process an RMSS message
   switch (preq->id) {
      //---------------------------------------
      // Enable or disable config update events
      //---------------------------------------
      case reqid_KRMS_SetConfigUpdates: {
         DPRINTF(AALRMS_DBG_IOCTL, ": Enabling Update Events\n");
         return psess->m_aalbus->register_config_update_handler(
                                             aalrms_event_config_update, NULL);
      }
      break;

      //-------------------------
      // Issue a shutdown request
      //-------------------------
      case reqid_Shutdown: {
         //-------------------------------
         // Create shutdown  request object
         //-------------------------------
         DPRINTF(AALRMS_DBG_IOCTL, ": Received a shutdown ioctl\n");
         newreq = rms_reqq_Shutdown_create((rms_shutdownreason_e)preq->data);
         if ( NULL == newreq ) {
            return -ENOMEM;
         }

         // Queue the item
         aalrms_queue_req(RMSSQIP(newreq));
         break;
      }

      //----------------
      // Device request
      //---------------
      case reqid_RM_DeviceRequest:
      {
         DPRINTF(AALRMS_DBG_IOCTL, ": Received a device request ioctl\n");
         return aalrms_processDeviceRequest(psess,
                                            preq);
      }

      default:
      {
         DPRINTF(AALRMS_DBG_IOCTL, ": Unknown message\n");
         return -EINVAL;
         break;
      }
   } // switch (preq->id)
   return 0;
}

//=============================================================================
// Name: aalrms_process_sendresponse
// Description: Process a response message
// Interface: public
// Inputs:  psess- pointer to session
//          preq - pointer to user request block
//          pqitem - Pointer to the qitem holding original request
// Outputs: Response copied to user space..
// Comments:
//=============================================================================
int aalrms_process_sendresponse(struct aalrm_server_session *psess,
                                struct aalrm_ioctlreq  *preq,
                                struct aal_q_item    *pqitem)
{
   struct req_registrar    *pregresp;        // Response from registrar
   struct rsp_device       *presresp;        // Response from RMS
   union {
      struct rms_reqq_reqdev     *preqdev;   // "Request Device" message which is of type qitem
      struct rms_reqq_registrar  *pregreq;   // "Registrar Request" message which is of type qitem
   }u;

   int ret = 0;

   // Lock-out cancels. Do not make this interruptible
   kosal_sem_get_user(&rmserver.m_sem);

   // Make sure the qitem is valid
   if(likely(aalrms_pendq_find(pqitem))){
      //Remove it from the pending queue
      aalrms_pendq_remove(pqitem);

   }
   else{
      DPRINTF (AALRMS_DBG_IOCTL, ": Invalid qitem  %p\n",pqitem);

      ret = -EINVAL;
      goto done;
   }

   //-----------------
   // Process response
   //-----------------
   switch (preq->id) {
      //------------------
      // Registar response  //TODO make sure the response is correct for the request
      //------------------
      case rspid_RS_Registrar: {
         DPRINTF(AALRMS_DBG_IOCTL, ": rspid_RS_Registrar - Payload len = %d\n",(int)preq->size);

         u.pregreq = qi_to_regreq(pqitem);

         //--------------------------------
         // Make a copy of the user message
         //  so that it can be sent in a
         //  response event back to RMCS
         //--------------------------------

         // Allocates space for the message + payload
         pregresp = (struct req_registrar    *)kosal_kmalloc((preq->size + REGISTRAR_REQ_HDRSZ));
         if (pregresp == NULL) {
            DPRINTF(AALRMS_DBG_IOCTL,
                  ": CMD - Registrar response failed kmalloc\n");
            ret = -ENOMEM;
            goto done;
         }

         //---------------------------------------------------
         // Copy the header from original request. It includes
         //   the TranID and Context from original request.
         //   These need to be returned in the response.
         //---------------------------------------------------
         *pregresp = REGREQ_REGREQ(u.pregreq);

         //Update the Payload information with Registrar response data
         pregresp->size = preq->size;

         DPRINTF(AALRMS_DBG_IOCTL,
               ": CMD - Queuing Registrar response size = %d\n",
               (int) pregresp->size);

         // Copy in the response payload
         if((pregresp->size != 0) && preq->payload ){
            //Copy the payload
            if (copy_from_user(&pregresp->buf, preq->payload, preq->size)) {
               DPRINTF(AALRMS_DBG_IOCTL, ": CMD - Copy failed\n");
               kosal_kfree(pregresp, (preq->size + REGISTRAR_REQ_HDRSZ));
               ret = -EFAULT;
               goto done;
            }
         }

         // Call the RMC completion callback
         CALL_COMPLETION(u.pregreq)(preq->result_code,
                                    pregresp,
                                    REGREQ_REGREQP(u.pregreq),
                                    RMSSQ_TRANID(u.pregreq));

         // Free the copy of the response
         kosal_kfree(pregresp, (preq->size + REGISTRAR_REQ_HDRSZ));

         //Destroy original request message
         rms_reqq_regreq_destroy(u.pregreq);
         
         break;
      } // case rspid_RS_Registrar

      //-----------------------
      // RequestDevice response
      //-----------------------
      case rspid_URMS_RequestDevice: {
         DPRINTF(AALRMS_DBG_IOCTL, ": rspid_URMS_RequestDevice - Payload len = %d\n",(int)preq->size);

         // Get the original request from queue item
         u.preqdev = qi_to_reqdev(pqitem);

         //--------------------------------
         // Make a copy of the user message
         //  so that it can be sent in a
         //  response event back to RMCS
         //--------------------------------

         // Allocates space for the message + payload
         presresp = (struct rsp_device *)kosal_kmalloc((preq->size + REQDEVICE_RSP_HDRSZ));
         if (presresp == NULL) {
            DPRINTF(AALRMS_DBG_IOCTL,
                  ": CMD - Registrar response failed kmalloc\n");
            ret = -ENOMEM;
            goto done;
         }

         presresp->devHandle = preq->res_handle;
         presresp->result = preq->result_code;
         if(unlikely(presresp->result != 0 ) ){
            DPRINTF(AALRMS_DBG_IOCTL, ": Failure response sent from uRMS\n");
         }

         //Update the Payload information with response data
         presresp->size = preq->size;

         if((presresp->size != 0) && preq->payload ){
            //Copy the payload
            DPRINTF(AALRMS_DBG_IOCTL, ": Copying payload %p[%d]\n",preq->payload,(int)presresp->size);
            if (copy_from_user(&presresp->buf, preq->payload, preq->size)) {
               DPRINTF(AALRMS_DBG_IOCTL, ": CMD - Copy failed\n");
               kosal_kfree(presresp, (preq->size + REQDEVICE_RSP_HDRSZ));
               ret = -EFAULT;
               goto done;
            }
         }

         // Send the response event back
         CALL_COMPLETION( u.preqdev)(presresp->result,
                                     presresp,
                                     REQDEV_ALLOCDEVP(u.preqdev),
                                     RMSSQ_TRANID(u.preqdev));

         // Done so destroy the request
         rms_reqq_reqdev_destroy(u.preqdev);

         // Destroy the response
         kosal_kfree(presresp, (preq->size + REQDEVICE_RSP_HDRSZ));
         
         break;
      } // rspid_URMS_RequestDevice

      default:  {
         DPRINTF(AALRMS_DBG_IOCTL, ": CMD - Unknown response\n");
         ret = -EINVAL;
         goto done;
         break;
      }
   } // switch (preq->id)

done:
   kosal_sem_put(&rmserver.m_sem);
   return ret;
}


//=============================================================================
// Name: aalrm_server_ioctl
// Description: Implements the ioctl system call
// Interface: public
// Inputs: inode - pointer to inode of special device file
//         file - open file instance
//         cmd - from command argument of ioctl
//         arg - arg argument of ioctl
// Outputs: none.
// Comments:
//=============================================================================
#if HAVE_UNLOCKED_IOCTL
long aalrm_server_ioctl(struct file *file,
                        unsigned int cmd,
                        unsigned long arg)
#else
int aalrm_server_ioctl(struct inode *inode,
                       struct file *file,
                       unsigned int cmd,
                       unsigned long arg)
#endif

{
   // Get the session pointer from file instance
   struct aalrm_server_session *psess =
                           (struct aalrm_server_session *) file->private_data;

   // Generic variables
   int                           ret=0;
   struct aalrm_ioctlreq         req;              // User IOCTL request structure

   // Variables used in the Device Allocate Messages
   struct aal_q_item             *pqitem = NULL;   // Generic request queue item

   //---------------------
   // Get the user request
   //---------------------
   if(copy_from_user (&req, (void *) arg, sizeof(req))){
      return -EFAULT;
   }

   if ( ( req.size + REGISTRAR_REQ_HDRSZ ) > KMALLOC_MAX_SIZE ) {
      PERR("Request size too large: %" PRIu64 "\n", req.size + REGISTRAR_REQ_HDRSZ);
      return -EINVAL;
   }

   //------------------
   // Process the IOCTL
   //------------------
   DPRINTF (AALRMS_DBG_IOCTL, "cmd=%x\n", cmd);
   ret = -EINVAL;  //Assume failure

   switch (cmd) {
   //-----------------------------------
   // Get next queued message descriptor
   // This will contain things like its
   // size and type
   //---------------------------------
      case AALRM_IOCTL_GETMSG_DESC: {
         // Make sure there is a message to be had
         if( aalrms_reqq_empty() ) {
            DPRINTF( AALRMS_DBG_IOCTL, ": No Message available\n" );
            return -EAGAIN;
         }

         // Peek the head of the message queue
         pqitem = aalrms_peek_req();
         if(pqitem == NULL) {
            DPRINTF( AALRMS_DBG_IOCTL, ": Invalid or corrupted request\n" );
            return -EFAULT;
         }

         // Return the type and total size of the message that will be returned
         req.id   = QI_QID(pqitem);
         req.size = QI_LEN(pqitem);

         DPRINTF(AALRMS_DBG_MOD, ": Getting Message Decriptor - size = %d\n",(int)req.size );

         // Return the message header
         if(copy_to_user ((struct aalrm_ioctlreq *) arg, &req, sizeof(struct aalrm_ioctlreq))) {
            DPRINTF(AALRMS_DBG_IOCTL, ": Failed get Message Decriptor \n");
            return -EFAULT;
         }
         return 0;
      } // case AALRM_IOCTL_GETMSG_DESC: 

   //-----------------------------------------------
   // Get the next message off of the request queue
   // and process it.  The message is processed and
   // may be forwarded to a user mode service
   //-----------------------------------------------
      case  AALRM_IOCTL_GETMSG: {
         // Make sure there is a message to be had
         if( aalrms_reqq_empty() ) {
            DPRINTF( AALRMS_DBG_IOCTL, ": No Message available\n" );
            return -EAGAIN;
         }


         // Process the request
         return aalrms_process_message(psess, arg, &req);
      }

      //-----------------------------------------
      // Send a message
      //  This may be a response to a request or
      //  simply a message
      //-----------------------------------------
      case  AALRM_IOCTL_SENDMSG: {
         if(req.req_handle == NULL) {
            DPRINTF(AALRMS_DBG_IOCTL, ": Sending message\n");
            return aalrms_process_sendmessage(psess, &req);
         }else {
            DPRINTF(AALRMS_DBG_IOCTL, ": Sending response\n");

            /* a straight cast from (void *) is performed here.  the pointer
             * value is validated before use in aalrms_process_sendresponse(),
             * so it is not checked here.  */
            pqitem = req.req_handle;

            return aalrms_process_sendresponse(psess, &req, pqitem);
         }
         break;
      }
      default:
      {
         DPRINTF(AALRMS_DBG_IOCTL, ": Invalid IOCTL=%x\n", cmd);
         break;
      }
   } // switch (cmd)
   return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////      RESOURCE MANAGER SERVER UTILITY METHODS       /////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
