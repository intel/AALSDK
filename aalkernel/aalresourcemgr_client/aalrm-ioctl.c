//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2015, Intel Corporation.
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
//  Copyright(c) 2008-2015, Intel Corporation.
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
//        FILE: aalrm-ioctl.c
//     CREATED: 02/21/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE:  IOCTL operations
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02-21-08       JG       Initial version created
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
// 02/09/2009     JG       Added support for RMSS cancel transaction
// 11/09/2010     HM       Removed extraneous kernel include asm/uaccess.h
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS AALRMC_DBG_IOCTL

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalrm-int.h"
#include "aalsdk/kernel/aalrm_server-services.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalrm-events-int.h"


//----------
// Protoypes
//----------
static inline btInt aalrm_process_event(btUnsigned64bitInt arg,
                                        struct aalrm_ioctlreq *,
                                        struct aal_q_item *);

static inline btInt aalrm_RequestDevEvent(btUnsigned64bitInt arg,
                                          struct aalrm_ioctlreq *,
                                          struct reqdev_cmplt *);

static inline btInt aalrm_RegistrarEvent(btUnsigned64bitInt arg,
                                         struct aalrm_ioctlreq *preq,
                                         struct registrarreq_cmplt *pqitem);

static inline btInt aalrm_ShutdownEvent(btUnsigned64bitInt arg,
                                        struct aalrm_ioctlreq *preq,
                                        struct shutdownreq_cmplt *pqitem);



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////         RESOURCE MANAGER CLIENT CALLBACKS         ///////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrm_reqdev_cmplt
// Description: Called by the Resource Manager Server Service when the device
//              request completes
// Interface: public
// Inputs:  pretdev - response data
//          origreq - original request
//          context - pointer to the session this request belongs to
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_reqdev_cmplt( btInt errno,
                         struct rsp_device   *pretdev,
                         struct req_allocdev *origreq,
                         struct aalrms_req_tranID tranID )
{
   struct reqdev_cmplt *preqdev_cmplt = NULL;
   struct aal_device *devp = NULL;
   struct aalresmgr_session *psess = (struct aalresmgr_session *)tranID.m_context;

   // Pending transactions
   psess->m_transcnt--;
   DPRINTF (AALRMC_DBG_IOCTL, ": Outstanding transactions %d\n",psess->m_transcnt);

   // Was it cancelled?
   if(errno == rms_resultCancelled){
      // Nothing to do but delete
      DPRINTF (AALRMC_DBG_IOCTL, ": Request device Cancelled.\n");

      // No longer need the original request
      kfree(origreq);
      return;
   }

   //  Add the session to the owner list
   if( likely(pretdev->result == rms_resultOK) ){
      // Get the device from the handle and claim the device
      devp = aaldev_handle_to_devp(pretdev->devHandle);
      if(likely(devp) ){
         aaldev_AddOwner_e ret;
         if( unlikely( (ret=dev_addOwner( devp,
                                          psess->m_tgpid,
                                          NULL, // TODO MANIFEST ON OWNER NOT SUPPORTED YET
                                          &psess->m_devlist)) != aaldev_addowner_OK)  ) {

            DPRINTF (AALRMC_DBG_IOCTL, ": failed to claim the device\n");

            // Change the result to failed
            pretdev->result = ret;
            pretdev->devHandle = NULL;
         }
      } // if(likely(devp) ) 
      else{
         DPRINTF (AALRMC_DBG_IOCTL, ": Software AFU returned\n");

         // Must be SW AFU
         pretdev->devHandle = NULL;

      }
   }// if( likely(pretdev->result == rms_resultOK) )
   else{
      DPRINTF (AALRMC_DBG_IOCTL, ": Received failed rsp_device\n");
   }

   // Create the completion event
   preqdev_cmplt = reqdev_cmplt_create(pretdev,origreq);

   // Queue the device result
   _aal_q_enqueue(&AALQI(preqdev_cmplt), &psess->m_eventq);

   // Wake any sleeping clients
   wake_up_interruptible (&psess->m_waitq);

   // No longer need the original request
   kfree(origreq);
}


//=============================================================================
// Name: aalrm_registar_cmplt
// Description: Called by the RMS when the Registrar request completes
// Interface: public
// Inputs: errno - error code
//         resp - response packet
//         origreq - original request
//         context - pointer to the session this request belongs to
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_registar_cmplt(btInt errno,
                          struct req_registrar *resp,
                          struct req_registrar *origreq,
                          struct aalrms_req_tranID tranID)
{
   struct aalresmgr_session *psess = (struct aalresmgr_session *)tranID.m_context;
   struct registrarreq_cmplt *preq_cmplt;
   // Pending transactions
   psess->m_transcnt--;
   DPRINTF (AALRMC_DBG_IOCTL, ": Outstanding transactions %d\n",psess->m_transcnt);

   // Was it cancelled?
   if(errno == rms_resultCancelled){
      // decrement the reference and ignore
      DPRINTF (AALRMC_DBG_IOCTL, ": Request device Cancelled.\n");

      // No longer need the original request
      kfree(origreq);
      return;
   }

   // Create the completion event
   preq_cmplt = registrar_cmplt_create(errno,resp);

   // Queue the device result
   _aal_q_enqueue(&AALQI(preq_cmplt), &psess->m_eventq);

   // Wake any sleeping clients
   wake_up_interruptible (&psess->m_waitq);

   // No longer need the original request
   kfree(origreq);
}


//=============================================================================
// Name: aalrm_shutdown_cmplt
// Description: Called by the ioctl message reqid_Shutdown
// Interface: public
// Inputs: errno - error code
//         resp - response packet
//         origreq - original request
//         context - pointer to the session this request belongs to
// Outputs: none.
// Comments:
//=============================================================================
void aalrm_shutdown_cmplt(btInt errno, void * context)
{
   struct aalresmgr_session *psess = context;

   // Create the completion event
   struct shutdownreq_cmplt *pshutdownreq_cmplt = shutdown_cmplt_create(errno);

   // Queue the device result
   _aal_q_enqueue(&AALQI(pshutdownreq_cmplt), &psess->m_eventq);

   // Wake any sleeping clients
   wake_up_interruptible (&psess->m_waitq);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////             EVENT HANDLING FUNCTIONS              ///////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrm_process_event
// Description: Process the event. An event is defined as a message from
//              the RMS e.g., a response.
// Interface: private
// Inputs: pqitem - AAL Qitem object
// Outputs: none.
// Comments:
//=============================================================================
static btInt aalrm_process_event(btUnsigned64bitInt arg,
                                 struct aalrm_ioctlreq    *preq,
                                 struct aal_q_item *pqitem)
{
   // Switch on message type
   switch(pqitem->m_id) {
      // Request response message
      case rspid_URMS_RequestDevice:
         return aalrm_RequestDevEvent(arg, preq, qi_to_reqdev_cmplt(pqitem));
         break;

      case rspid_RS_Registrar:
         return aalrm_RegistrarEvent(arg, preq, qi_to_registrarreq_cmplt(pqitem));
         break;

      case rspid_Shutdown:
         return aalrm_ShutdownEvent(arg, preq, qi_to_shutdownreq_cmplt(pqitem));
         break;
      default:
         return -EINVAL;
   }
   return 0;
}

//=============================================================================
// Name: aalrm_RequestDevEvent
// Description: Process the RequestDev resposne event
// Interface: private
// Inputs: pitem - Request object
// Outputs: none.
// Comments:
//=============================================================================
static inline btInt aalrm_RequestDevEvent(btUnsigned64bitInt arg,
                                          struct aalrm_ioctlreq *preq,
                                          struct reqdev_cmplt *pqitem)
{
   preq->id = rspid_URMS_RequestDevice;
   preq->res_handle = pqitem->retdev.devHandle;
   preq->result_code = pqitem->retdev.result;
   preq->context = pqitem->context;
   preq->tranID = pqitem->tranID;

   if(preq->result_code != rms_resultOK){
      DPRINTF(AALRMC_DBG_IOCTL, ": About to return failure %d\n",preq->result_code);
   }

   // Copy the header portion of the request back
   if(copy_to_user ((struct aalrm_ioctlreq *) arg, preq, sizeof(struct aalrm_ioctlreq))){
      DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message \n");
      reqdev_cmplt_destroy(pqitem);
      return -EFAULT;
   }

   DPRINTF( AALRMC_DBG_IOCTL,
            ": About to copy payload[%d] to buf[%d]\n %s",
            (int)QI_LEN(AALQIP(pqitem)),
            (int)preq->size,&pqitem->retdev.buf);

   if(preq->size >= QI_LEN(AALQIP(pqitem))){
      preq->size  = QI_LEN(AALQIP(pqitem));

      //-----------------------------------------------------------------------
      // Copy the payload portion of the request back
      // Note that the preq holds the user request and thus the payload pointer
      //-----------------------------------------------------------------------
      if(copy_to_user ((struct aalrm_ioctlreq *)preq->payload, &pqitem->retdev.buf, preq->size)){
         DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message \n");
         reqdev_cmplt_destroy(pqitem);
         return -EFAULT;
      }
   }// if(preq->size >= QI_LEN(AALQIP(pqitem)))
   else{
      // Invalid size for the return payload
      reqdev_cmplt_destroy(pqitem);
      return -EINVAL;
   }

   //Destroy the request
   reqdev_cmplt_destroy(pqitem);
   return 0;
}

//=============================================================================
// Name: aalrm_RegistrarEvent
// Description: Process the Registrar response event
// Interface: private
// Inputs: pitem - Request object
// Outputs: none.
// Comments:
//=============================================================================
static inline btInt aalrm_RegistrarEvent(btUnsigned64bitInt arg,
                                         struct aalrm_ioctlreq *preq,
                                         struct registrarreq_cmplt *pqitem)
{
   preq->size  = QI_LEN(AALQIP(pqitem));
   preq->id = rspid_URMS_RequestDevice;

   // Copy the header portion of the request back
   if(copy_to_user ((struct aalrm_ioctlreq *)arg, preq, sizeof(struct aalrm_ioctlreq))){
      DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message \n");
      return -EFAULT;
   }

   //-----------------------------------------------------------------------
   // Copy the payload portion of the request back
   // Note that the preq holds the user request and thus the payload pointer
   //-----------------------------------------------------------------------
   if(copy_to_user ((struct aalrm_ioctlreq *)preq->payload, &pqitem->resp->buf, preq->size)){
      DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message \n");
      return -EFAULT;
   }

   //Destroy the request
   registrar_cmplt_destroy(pqitem);
   return 0;
}

//=============================================================================
// Name: aalrm_ShutdownEvent
// Description: Process the shutdown event
// Interface: private
// Inputs: pitem - Request object
// Outputs: none.
// Comments:
//=============================================================================
static inline btInt aalrm_ShutdownEvent(btUnsigned64bitInt arg,
                                        struct aalrm_ioctlreq *preq,
                                        struct shutdownreq_cmplt *pqitem)
{
   preq->size = AALQ_QLEN(pqitem);
   preq->id = rspid_Shutdown;

   DPRINTF(AALRMC_DBG_IOCTL, ": Shutdown Event\n");

   // Copy the header portion of the request back
   if(copy_to_user ((struct aalrm_ioctlreq *)arg, preq, sizeof(struct aalrm_ioctlreq))){
      DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message \n");
      return -EFAULT;
   }

   //Destroy the request
   shutdown_cmplt_destroy(pqitem);
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////             MESSAGE HANDLING FUNCTIONS            ///////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrm_processmsg
// Description: Processes request messages from the user
// Interface: private
// Inputs: preq - pointer to generic request header
//         psess - session
// Outputs: none.
// Comments:
//=============================================================================
btInt aalrm_processmsg(struct aalrm_ioctlreq    *preq,
                       struct aalresmgr_session *psess )
{
   // Various variables that will be used exclusively
   union {
      struct req_allocdev  *pallocreq;
      struct req_registrar *preqreg;
   }req;

   struct aalrms_req_tranID tranID;

   struct req_allocdev  *pallocmsg;
   btInt                 ret = 0;
   struct aal_device    *pdev = NULL;
   struct aaldev_ownerSession * ownerSessp = NULL;

   switch(preq->id) {
      // Registrar request
      case reqid_RS_Registrar:{
         // Allocate the message -
         // Note the message will be freed when the completion event is returned
         req.preqreg = kmalloc((preq->size + REGISTRAR_REQ_HDRSZ), GFP_KERNEL);
         if(req.preqreg == NULL){
            DPRINTF (AALRMC_DBG_IOCTL, ": CMD - reqid_Registrar failed kmalloc\n");
            return -ENOMEM;
         }
         // Copy in the request and payload
         req.preqreg->tranID = preq->tranID;
         req.preqreg->context = preq->context;
         req.preqreg->size = preq->size;

         if(copy_from_user (&req.preqreg->buf, preq->payload, preq->size)){
            DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Copy failed\n");
            kfree(req.preqreg);
            return -EFAULT;
         }
         //--------------------------------------------------------------
         // Call the RMS service Registrar Request method
         // NOTE: i_rmserver() returns the aalrms_service interface from
         //       an aal_interface to a RMS service pointer
         //--------------------------------------------------------------

         // TODO MUST HAVE A METHOD OF FLUSHING OUTSTANDING REQUESTS IN THE CASE OF A CLOSE OR ABORT
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Calling RMS reqid_Registrar\n");

         // Pending transactions
         psess->m_transcnt++;
         tranID.m_context = psess;
         ret = i_rmserver(rmssess_to_rmssrv(psess)).registrar_request( req.preqreg,
                                                                       aalrm_registar_cmplt,
                                                                       tranID);
         DPRINTF (AALRMC_DBG_IOCTL, ": Outstanding transactions %d\n",psess->m_transcnt);
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Done Calling reqid_Registrar\n");
         break;
      }// case reqid_RS_Registrar


      // Request a device be allocated
      case reqid_URMS_RequestDevice:  {
         // Allocate the message -
         // Note the message will be freed when the compeletion event is returned
         pallocmsg = kmalloc((preq->size + ALLOC_DEVHDRSZ), GFP_KERNEL);
         if(pallocmsg == NULL){
            DPRINTF (AALRMC_DBG_IOCTL, ": CMD - AALRM_IOCTL_REQDEV failed kmalloc\n");
            return -ENOMEM;
         }
         // Copy in the request and payload
         pallocmsg->tranID = preq->tranID;
         pallocmsg->context = preq->context;
         pallocmsg->size = preq->size;
         if( preq->size != 0 ){
            if(copy_from_user (&pallocmsg->buf, preq->payload, preq->size)){
               DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Copy failed\n");
               kfree(pallocmsg);
               return -EFAULT;
            }
         }

         DPRINTF(AALRMC_DBG_IOCTL, ": About to Get %d  \n %s\n",(int)pallocmsg->size,(char*)pallocmsg->buf);

         //--------------------------------------------------------------
         // Call the RMS service request Device method
         // Note: first psess is used by PM to assign ownership to device
         //       the second is the context used in the callback
         // NOTE: i_rmserver() returns the aalrms_service interface from
         //       an aal_interface to a RMS service pointer
         //---------------------------------------------------------------
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Calling RMS request_device\n");

         // Pending transactions
         tranID.m_context = psess;
         psess->m_transcnt++;
         ret = i_rmserver(rmssess_to_rmssrv(psess)).request_device( pallocmsg,
                                                                    aalrm_reqdev_cmplt,
                                                                    tranID);

         DPRINTF (AALRMC_DBG_IOCTL, ": Outstanding transactions %d\n",psess->m_transcnt);
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Done Calling RMS request_device\n");
         break;
      }// case reqid_URMS_RequestDevice

      case reqid_URMS_ReleaseDevice: {
          DPRINTF (AALRMC_DBG_IOCTL, ": reqid_URMS_ReleaseDevice - UI\n");

          // Get the device from the handle and validate
          pdev = aaldev_handle_to_devp(preq->res_handle);
          if( unlikely(pdev == NULL) ){
             DPRINTF (AALRMC_DBG_IOCTL, ": Invalid device handle %p\n", preq->res_handle);
             return -EINVAL;
          }

          // Get the device session for this device
          ownerSessp = dev_OwnerSession(pdev,psess->m_tgpid);

          // Get the default message interface
          if(ownerSessp == NULL){
                DPRINTF (AALRMC_DBG_IOCTL, ": Not owner or no message handler.\n");
                return -EACCES;
          }

          // Update the owner's list
          if(unlikely(!dev_removeOwner( pdev,
                                        psess->m_tgpid))){
             DPRINTF (AALRMC_DBG_IOCTL, ": Failed to update owner\n");
             ret = -EINVAL;
           }
           break;
      }// case reqid_URMS_ReleaseDevice
      case reqid_Shutdown:{
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Shutdown\n");
         // For now nothing much to do so complete
         aalrm_shutdown_cmplt(0, psess);
         ret = 0;
         break;
      }

      default: {
         DPRINTF (AALRMC_DBG_IOCTL, ": CMD - Unexpected\n");
         ret = -EINVAL;
         break;
      }
   } // switch(preq->id) 
   return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////                                              /////////////////
/////////////             IOCTL HANDLING FUNCTION                //////////////
////////////////                                              /////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: aalrm_ioctl
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
long aalrm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
int aalrm_ioctl(struct inode *inode, struct file *file,
                unsigned int cmd, unsigned long arg)
#endif
{
   btInt ret=0;
   struct aalrm_ioctlreq    req;              // Holds user request
   struct aal_q_item       *pqitem = NULL;    // Generic request queue item

   // Get the user session
   struct aalresmgr_session *psess = (struct aalresmgr_session *) file->private_data;

   // Get the request header first
   if (copy_from_user (&req, (void *) arg, sizeof(req))){
    return -EFAULT;
   }

   ret = -EINVAL;  //Assume failure

   // Inv Process IOCTL CMD
   switch (cmd){
      // Send message
      case AALRM_IOCTL_SENDMSG:{
         return aalrm_processmsg(&req, psess);
         break;
      }

      //---------------------------------------
      // Get next queued RMS message descriptor
      // This will contain things like its
      // size and type
      //---------------------------------------
      case AALRM_IOCTL_GETMSG_DESC: {
         // Make sure there is a message to be had
         if( _aal_q_empty( &psess->m_eventq ) ){
            DPRINTF( AALRMC_DBG_IOCTL, ": No Message available\n" );
            return -EAGAIN;
         }

         // Peek the head of the RMS message queue
         pqitem = _aal_q_peek( &psess->m_eventq );
         if(pqitem == NULL){
            DPRINTF( AALRMC_DBG_IOCTL, ": AALRM_IOCTL_GETMSG_DESC Invalid or corrupted request\n" );
            return -EFAULT;
         }

         // Return the type and total size of the message that will be returned including header
         req.id    = QI_QID(pqitem);
         req.size  = QI_LEN(pqitem);

         DPRINTF(AALRMC_DBG_MOD, ": Getting Message Descriptor - size = %d\n",(int)req.size );
         if(copy_to_user ((struct aalrm_ioctlreq *) arg, &req, sizeof(struct aalrm_ioctlreq))){
            DPRINTF(AALRMC_DBG_IOCTL, ": Failed get Message Decriptor \n");
            return -EFAULT;
         }
         ret = 0;
        break;
      } // case AALRM_IOCTL_GETMSG_DESC

      //----------------------------------------------------
      // Get the next message off of the RMS message queue
      //----------------------------------------------------
      case AALRM_IOCTL_GETMSG:{
         DPRINTF( AALRMC_DBG_IOCTL, ": Getting Message\n" );

         // Make sure there is a message to be had
         if(  _aal_q_empty( &psess->m_eventq ) ){
            DPRINTF( AALRMC_DBG_IOCTL, ": No Message available\n" );
            req.size = 0;
            if(copy_to_user ((struct aalrm_ioctlreq *) arg, &req, sizeof(struct aalrm_ioctlreq))){
               DPRINTF(AALRMC_DBG_IOCTL, ": Failed copy Message\n");
               return -EFAULT;
            }
            return -EAGAIN;
         }

         //------------------------
         // Get the request message
         //------------------------
         pqitem = _aal_q_dequeue(&psess->m_eventq);
         if(pqitem == NULL){
            DPRINTF( AALRMC_DBG_IOCTL, ": AALRM_IOCTL_GETMSG Invalid or corrupted request\n" );
            return -EFAULT;
         }

         ret = aalrm_process_event(arg,
                                   &req,
                                   pqitem);
      break;
      } // case AALRM_IOCTL_GETMSG

      default:
         DPRINTF (AALRMC_DBG_IOCTL, ": Invalid IOCTL=%x\n", cmd);
         ret = -EINVAL;
         break;

   } // switch (cmd)
   return ret;
}


//=============================================================================
// Name: aalrm_flush_eventqueue
// Description: flush the event queue
// Interface: private
// Inputs: psess - session pointer
// Outputs: none.
// Comments:
//=============================================================================
btInt aalrm_flush_eventqueue(  struct aalresmgr_session *psess)
{
   btInt ret = 0;
   struct aal_q_item *pqitem;
   DPRINTF( AALRMC_DBG_FILE, ": Flushing event queue\n" );
   while(!_aal_q_empty(&psess->m_eventq) ) {
      //------------------------
      // Get the request message
      //------------------------
      pqitem = _aal_q_dequeue(&psess->m_eventq);
      if(pqitem == NULL) {
         DPRINTF( AALRMC_DBG_FILE, ": Invalid or corrupted request on flush\n" );
         continue;
      }
      // Switch on message type
      switch(pqitem->m_id){
         // Request response message
         case rspid_URMS_RequestDevice:
            DPRINTF( AALRMC_DBG_FILE, ": rspid_URMS_RequestDevice\n" );
            reqdev_cmplt_destroy(qi_to_reqdev_cmplt(pqitem));
            break;

         case rspid_RS_Registrar:
            DPRINTF( AALRMC_DBG_FILE, ": rspid_RS_Registrar\n" );
            registrar_cmplt_destroy(qi_to_registrarreq_cmplt(pqitem));
            break;

         case rspid_Shutdown:
            DPRINTF( AALRMC_DBG_FILE, ": rspid_Shutdown\n" );
            shutdown_cmplt_destroy(qi_to_shutdownreq_cmplt(pqitem));
            break;

         default:
            DPRINTF( AALRMC_DBG_FILE, ": Encountered unknown event while flushing - leak\n" );
      }
   }
   return ret;
}
