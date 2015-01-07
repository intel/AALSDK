//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
//  Copyright(c) 2011-2015, Intel Corporation.
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
//        FILE: spl2_afu_pip.c
//     CREATED: 02/06/2011
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          SPL2 AFU PIP
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_DEV

#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/fappip.h"

#include "spl2_pip_internal.h"

// Must come before mem includes for Debug tracing
#include "spl2_session.h"     // Includes spl2_pip_internal.h

#include "aalsdk/kernel/vafu2defs.h"
#include "mem-sess.h"         // Memory construct definitions
#include "xsid.h"             // xsid information


static int
AFUCommand(struct aaldev_ownerSession *,
           struct aal_pipmessage,
           void *);


//=============================================================================
// Name:SPLAFUpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip SPLAFUpip = {
   .m_messageHandler = {
      .sendMessage   = AFUCommand,        // Command Handler
      .bindSession   = AFUbindSession,    // Session binder
      .unBindSession = AFUunbindSession,  // Session unbinder
   },

   .m_fops = {
     .mmap = spl2_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};


//=============================================================================
// Name: flush_all_wsids
// Description: Frees all workspaces allocated for this session
// Interface: private
// Inputs: sessp - session
// Comments: This function should be called during cleanup.  It does not
//           protect session queue access
//=============================================================================
void
flush_all_wsids(struct spl2_session *psess)
{
   struct aaldev_ownerSession *pownerSess;
   struct spl2_device         *pdev;
   struct memmgr_session      *pmem_sess;

   struct aal_wsid            *wsidp;
   struct aal_wsid            *tmp;
   xsid_t                      xsid;

   PTRACEIN;

   ASSERT(psess);

   pownerSess = spl2_sessionp_to_ownerSession(psess);
   ASSERT(pownerSess);

   pdev = spl2_sessionp_to_spl2dev(psess);
   ASSERT(pdev);

   pmem_sess = spl2_dev_mem_sessionp(pdev);
   ASSERT(pmem_sess);

   PVERBOSE("Freeing allocated workspaces.\n");

   list_for_each_entry_safe(wsidp, tmp, &pownerSess->m_wshead, m_list) {

      if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pdev)){
         kosal_free_contiguous_mem((btAny)wsidp->m_id, wsidp->m_size);
      }else{

         xsid = (xsid_t)wsidp->m_id;

         // Destroy the workspace
         ASSERT(spl2_memsession_valid_xsid(xsid));
         if ( spl2_memsession_valid_xsid(xsid) ) {
            virtmem_destruct(&spl2_memsession_virtmem(pmem_sess,xsid));
         } else {
            PDEBUG("flush_all_wsids: Found bad xsid %d\n", (unsigned)xsid);
         }
      }
      // remove the wsid from the device and destroy
      PVERBOSE("Done Freeing PWS with id 0x%llx.\n", wsidp->m_id);
      list_del_init(&wsidp->m_list);
      pownerSess->m_uiapi->freewsid(wsidp);
   } // end list_for_each_entry
   PTRACEOUT;
}

//=============================================================================
// Name: start_spl2_transaction
// Description:
// Interface: public
// Inputs: pDevOwnerSess - Pointer to the device Owner Session context object
//         pPIPMsg - Message to driver
//         pPIPMsgContext- Context
//         pSPL2Req - SPL2 specific message
// Returns: success = 1
// Comments:
//=============================================================================
static
int
start_spl2_transaction(struct aaldev_ownerSession *pDevOwnerSess,
                       struct aal_pipmessage      *pPIPMsg,
                       void                       *pPIPMsgContext,
                       struct spl2req             *pSPL2Req)
{
   uid_errnum_e                            err;

   struct spl2_session                    *psess;
   struct spl2_device                     *pdev;
   struct aalui_AFUmessage                *pUIMsg;

   struct uidrv_event_afu_response_event  *pAFUResponseEvent = NULL;
   struct uidrv_event_afu_workspace_event *pAFUWkspcEvent    = NULL;
   struct aal_q_item                      *pQItem            = NULL;

   struct aal_wsid                        *pAFUCtxWsID       = NULL;
   struct aal_wsid                        *pAFUDSMWsID       = NULL;

   struct aalui_AFUResponse                UIResponse;

   // Verified non-NULL in AFUCommand()
   psess = (struct spl2_session  *)aalsess_pipHandle(pDevOwnerSess);

   // Verified non-NULL in AFUCommand()
   pdev = spl2_sessionp_to_spl2dev(psess);

   // Validate the UI message
   pUIMsg = (struct aalui_AFUmessage *)pPIPMsg->m_message;
   ASSERT(pUIMsg);
   ASSERT(SPL2_AFUPIP_IID == pUIMsg->pipver);
   ASSERT(sizeof(struct spl2req) == pUIMsg->size);
   if ( (NULL == pUIMsg) ||
        (SPL2_AFUPIP_IID != pUIMsg->pipver) ||
        (sizeof(struct spl2req) != pUIMsg->size) ) {
      pAFUResponseEvent = uidrv_event_afu_afuinavlidrequest_create(pDevOwnerSess->m_device,
                                                                   &pPIPMsg->m_tranID,
                                                                   pPIPMsg->m_context,
                                                                   uid_errnumInvalidRequest);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   // Attempt to claim the device. When successful, resets dev and initializes DSMs.

   err = spl2_trans_setup(psess, pdev);
   if ( uid_errnumOK != err ) {
      pAFUResponseEvent = uidrv_event_afutrancmplt_create(pDevOwnerSess->m_device,
                                                          &pPIPMsg->m_tranID,
                                                          pPIPMsg->m_context,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0,
                                                          err);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   down( spl2_sessionp_semaphore(psess) );

   spl2_sessionp_currTran(psess)       = pPIPMsg->m_tranID;
   spl2_sessionp_currContext(psess)    = pPIPMsg->m_context;
   spl2_sessionp_currMsgContext(psess) = pPIPMsgContext;

   up( spl2_sessionp_semaphore(psess) );

   if ( 0 == pSPL2Req->ahmreq.u.wksp.m_wsid ) {
      PVERBOSE("Start SPL2 Transaction without workspace.\n");
   } else {
      PVERBOSE("Start SPL2 Transaction w/ workspace.\n");

      pAFUCtxWsID = wsid_to_wsidobjp( pSPL2Req->ahmreq.u.wksp.m_wsid );

      err = spl2_trans_start(psess, pdev, pAFUCtxWsID, pSPL2Req->pollrate);
      if ( uid_errnumOK != err ) {
         pAFUWkspcEvent = uidrv_event_afu_afufreecws_create(pDevOwnerSess->m_device,
                                                            pPIPMsg->m_tranID,
                                                            pPIPMsg->m_context,
                                                            err);
         pQItem = AALQIP(pAFUWkspcEvent);
         goto ERROR;
      }
   }

   // Success

   // Allocate workspace ID for the AFU DSM.
   pAFUDSMWsID = pDevOwnerSess->m_uiapi->getwsid(pDevOwnerSess->m_device,
                                                 SPL2_AFUDSM_XSID);
   if ( NULL == pAFUDSMWsID ) {
      PERR("Could not get DSM workspace ID\n");
      /* generate a could not create message back to the requestor */
      pAFUResponseEvent = uidrv_event_afu_afuinavlidrequest_create(
         pDevOwnerSess->m_device, &pPIPMsg->m_tranID, pPIPMsg->m_context,
         uid_errnumCouldNotCreate);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   pAFUDSMWsID->m_size = spl2_dev_AFUDSM_size;
   pAFUDSMWsID->m_type = WSM_TYPE_VIRTUAL;

   // Return the parms of the AFU DSM in the response event.
   down( spl2_sessionp_semaphore(psess) );

   ASSERT(NULL == spl2_dev_AFUDSM_wsid(pdev));
   spl2_dev_AFUDSM_wsid(pdev)          = pAFUDSMWsID;
   spl2_dev_AFUDSM_WSMParms(pdev).wsid = wsidobjp_to_wid(spl2_dev_AFUDSM_wsid(pdev));

   up( spl2_sessionp_semaphore(psess) );

   ASSERT(NULL != spl2_dev_AFUDSM_wsid(pdev));

   PVERBOSE("Sending AFU DSM aalui_WSMParms wsid=0x%" PRIx64 " size=%" PRIu64 " physptr=0x%" PRIxPHYS_ADDR "\n",
               spl2_dev_AFUDSM_WSMParms(pdev).wsid,
               spl2_dev_AFUDSM_WSMParms(pdev).size,
               spl2_dev_AFUDSM_WSMParms(pdev).physptr);

   UIResponse.respID  = uid_afurespTaskStarted;
   UIResponse.evtData = 0;

   pAFUResponseEvent = uidrv_event_afutranstate_create(pDevOwnerSess->m_device,
                                                      &pPIPMsg->m_tranID,
                                                       pPIPMsg->m_context,
                                                      &UIResponse,
                                                      &spl2_dev_AFUDSM_WSMParms(pdev),
                                                       uid_errnumOK);
   pQItem = AALQIP(pAFUResponseEvent);

   pDevOwnerSess->m_uiapi->sendevent(pDevOwnerSess->m_UIHandle,
                                     pDevOwnerSess->m_device,
                                     pQItem,
                                     pPIPMsgContext);

   // If a workspace has been provided, start polling.
   if ( NULL != pAFUCtxWsID ) {

      // Will be up'ed when the transaction is done or when task_poller
      // realizes the transaction has been canceled.
      down( spl2_dev_tran_done_semp(pdev) );

      queue_delayed_work(spl2_dev_workq(pdev),
                         spl2_dev_task_handler(pdev),
                         msecs_to_jiffies(spl2_dev_pollrate(pdev)));
   }

   return 0;

ERROR:
   if ( pQItem ) {
      pDevOwnerSess->m_uiapi->sendevent(pDevOwnerSess->m_UIHandle,
                                        pDevOwnerSess->m_device,
                                        pQItem,
                                        pPIPMsgContext);
   }

   return 1;
}
//=============================================================================
// Name: stop_spl2_transaction
// Description: Stops the active transaction without releasing any resources
// Interface: public
// Inputs: pownerSess - Device Owner Session context
// Returns: success = 0
// Comments:  Host AFU need  not do anything
//=============================================================================
static
int stop_spl2_transaction(struct aaldev_ownerSession *pownerSess,
                          struct aal_pipmessage      *pPIPMsg,
                          void                       *pPIPMsgContext)
{
   struct spl2_session           *sessp = (struct spl2_session *)pownerSess->m_PIPHandle;
   struct spl2_device            *pdev  = spl2_sessionp_to_spl2dev(sessp);
   struct uidrv_event_afu_response_event  *pAFUResponseEvent = NULL;
   struct aal_q_item                      *pQItem            = NULL;
   struct aalui_AFUResponse       UIResponse;

   PDEBUG("Stopping device transaction\n");

   // Stop all on-going processing
   down( spl2_dev_semp(pdev) );

   if ( spl2_dev_activesess(pdev) ) {
      spl2_sessionp_currState(spl2_dev_activesess(pdev)) = SPL2_SESS_STATE_NONE;
   }

   up( spl2_dev_semp(pdev) );

    // Wait for the task poller to finish, if it was active.
   down( spl2_dev_tran_done_semp(pdev) );
   spl2_enable(pdev, SPL2_DISABLE);
   spl2_spl_reset(pdev);
   up( spl2_dev_tran_done_semp(pdev) );

   down( spl2_sessionp_semaphore(sessp) );

   ASSERT(NULL != spl2_dev_AFUDSM_wsid(pdev));
   // Free the DSM WSID
   if(NULL != spl2_dev_AFUDSM_wsid(pdev) ) {
      pownerSess->m_uiapi->freewsid(spl2_dev_AFUDSM_wsid(pdev));
      spl2_dev_AFUDSM_wsid(pdev) = NULL;
   }

   up( spl2_sessionp_semaphore(sessp) );

   // Generate the event
   // Success

    memset(&UIResponse, 0, sizeof(UIResponse));

    UIResponse.respID = uid_afurespTaskStopped;

    // Generate the task complete event
    pAFUResponseEvent = uidrv_event_afutranstate_create( pdev,
                                                        &spl2_sessionp_currTran(sessp),
                                                         spl2_sessionp_currContext(sessp),
                                                        &UIResponse,
                                                        &spl2_dev_AFUDSM_WSMParms(pdev),
                                                        uid_errnumOK);
    pQItem = AALQIP(pAFUResponseEvent);

    pownerSess->m_uiapi->sendevent( pownerSess->m_UIHandle,
                                    pownerSess->m_device,
                                    pQItem,
                                    pPIPMsgContext);


   return 0;
}

//=============================================================================
// Name: set_spl2_context_workspace
// Description: Sets the context workspace for the device
// Interface: public
// Inputs: pDevOwnerSess - Pointer to the device Owner Session context object
//         pPIPMsg - Message to driver
//         pPIPMsgContext- Context
//         pSPL2Req - SPL2 specific message
// Returns: success = 1
// Comments:  Host AFU need  not do anything
//=============================================================================
static
int
set_spl2_context_workspace(struct aaldev_ownerSession *pDevOwnerSess,
                           struct aal_pipmessage      *pPIPMsg,
                           void                       *pPIPMsgContext,
                           struct spl2req             *pSPL2Req)
{
   uid_errnum_e                            err;

   struct spl2_session                    *psess;
   struct spl2_device                     *pdev;
   struct aalui_AFUmessage                *pUIMsg;

   struct uidrv_event_afu_response_event  *pAFUResponseEvent = NULL;
   struct uidrv_event_afu_workspace_event *pAFUWkspcEvent    = NULL;
   struct aal_q_item                      *pQItem            = NULL;

   struct aal_wsid                        *pWsID             = NULL;

   struct aalui_AFUResponse                UIResponse;

   // Verified non-NULL in AFUCommand()
   psess = (struct spl2_session  *)aalsess_pipHandle(pDevOwnerSess);

   // Verified non-NULL in AFUCommand()
   pdev = spl2_sessionp_to_spl2dev(psess);

   // Validate the UI message
   pUIMsg = (struct aalui_AFUmessage *)pPIPMsg->m_message;
   ASSERT(pUIMsg);
   ASSERT(SPL2_AFUPIP_IID == pUIMsg->pipver);
   ASSERT(sizeof(struct spl2req) == pUIMsg->size);
   if ( (NULL == pUIMsg) ||
        (SPL2_AFUPIP_IID != pUIMsg->pipver) ||
        (sizeof(struct spl2req) != pUIMsg->size) ) {
      pAFUResponseEvent = uidrv_event_afu_afuinavlidrequest_create(pDevOwnerSess->m_device,
                                                                   &pPIPMsg->m_tranID,
                                                                   pPIPMsg->m_context,
                                                                   uid_errnumInvalidRequest);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   if ( 0 == pSPL2Req->ahmreq.u.wksp.m_wsid ) {
      // fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE, but no workspace - error.

      PDEBUG("Error: got Set Context Workspace without workspace.\n");

      pAFUResponseEvent = uidrv_event_afutrancmplt_create(pDevOwnerSess->m_device,
                                                          &pPIPMsg->m_tranID,
                                                          pPIPMsg->m_context,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0,
                                                          uid_errnumBadParameter);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   PVERBOSE("Set SPL2 Context Workspace\n");

   down( spl2_sessionp_semaphore(psess) );

   if (!spl2_dev_activesess(pdev) ) {
      // fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE, but there
      //  is no active session - protocol violation.

      up( spl2_sessionp_semaphore(psess) );

      PDEBUG("Error: got Set Context Workspace, but there is no active session.\n");

      pAFUResponseEvent = uidrv_event_afutrancmplt_create(pDevOwnerSess->m_device,
                                                          &pPIPMsg->m_tranID,
                                                          pPIPMsg->m_context,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0,
                                                          uid_errnumPermission);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   } else if ( psess != spl2_dev_activesess(pdev) ) {
      // This is not the active session - deny the request.

      up( spl2_sessionp_semaphore(psess) );

      PDEBUG("Error: got Set Context Workspace for dev 0x%p, but active session 0x%p doesn't match request 0x%p\n",
                pdev,
                spl2_dev_activesess(pdev),
                psess);

      pAFUResponseEvent = uidrv_event_afutrancmplt_create(pDevOwnerSess->m_device,
                                                          &pPIPMsg->m_tranID,
                                                          pPIPMsg->m_context,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0,
                                                          uid_errnumPermission);
      pQItem = AALQIP(pAFUResponseEvent);
      goto ERROR;
   }

   up( spl2_sessionp_semaphore(psess) );

   // State and parameter verification complete - start the transaction.

   pWsID = wsid_to_wsidobjp( pSPL2Req->ahmreq.u.wksp.m_wsid );

   err = spl2_trans_start(psess, pdev, pWsID, pSPL2Req->pollrate);
   if ( uid_errnumOK != err ) {
      pAFUWkspcEvent = uidrv_event_afu_afufreecws_create(pDevOwnerSess->m_device,
                                                         pPIPMsg->m_tranID,
                                                         pPIPMsg->m_context,
                                                         err);
      pQItem = AALQIP(pAFUWkspcEvent);
      goto ERROR;
   }

   // Success

   memset(&UIResponse, 0, sizeof(UIResponse));

   UIResponse.respID = uid_afurespSetContext;

   pAFUResponseEvent = uidrv_event_afutrancmplt_create(pDevOwnerSess->m_device,
                                                      &pPIPMsg->m_tranID,
                                                      pPIPMsg->m_context,
                                                      &UIResponse,
                                                      NULL,
                                                      (btByteArray)spl2_dev_AFUDSM(pdev),
                                                      sizeof(struct VAFU2_DSM),
                                                      uid_errnumOK);
   pQItem = AALQIP(pAFUResponseEvent);

   pDevOwnerSess->m_uiapi->sendevent(pDevOwnerSess->m_UIHandle,
                                     pDevOwnerSess->m_device,
                                     pQItem,
                                     pPIPMsgContext);

   // Success - begin polling for completion.

   // Will be up'ed when the transaction is done or when task_poller
   // realizes the transaction has been canceled.
   down( spl2_dev_tran_done_semp(pdev) );

   queue_delayed_work(spl2_dev_workq(pdev),
                      spl2_dev_task_handler(pdev),
                      msecs_to_jiffies(spl2_dev_pollrate(pdev)));

   return 0;

ERROR:
   if ( pQItem ) {
      pDevOwnerSess->m_uiapi->sendevent(pDevOwnerSess->m_UIHandle,
                                        pDevOwnerSess->m_device,
                                        pQItem,
                                        pPIPMsgContext);
   }

   return 1;
}

//=============================================================================
// Name: AFUCommand
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
//         MessageContext - used by UIDRV (TODO deprecate)
// Outputs: none.
// Comments:
//=============================================================================
int
AFUCommand(struct aaldev_ownerSession *pownerSess,
           struct aal_pipmessage       Message,
           void                       *MessageContext)
{
#if (1 == ENABLE_DEBUG)
#define AFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define AFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG
   // Private session object set at session bind time (i.e., when allocated)
   struct spl2_session *sessp = (struct spl2_session *)aalsess_pipHandle(pownerSess);
   struct spl2_device  *pdev  = NULL;

   // Generalized payload pointer. Points to locally allocated and copied
   //    version of pmsg->payload of length pmsg->size
   void *p_localpayload = NULL;

   // Overall return value for this function. Set before exiting if there is an error.
   //    retval = 0 means good return.
   int retval = 0;

   // UI Driver message
   struct aalui_AFUmessage *pmsg = (struct aalui_AFUmessage *) Message.m_message;

   // Used to point to the response event
   struct uidrv_event_afu_response_event  *pafuresponse_evt = NULL;
   struct uidrv_event_afu_workspace_event *pafuws_evt       = NULL;

   // Used by WS allocation
   struct aal_wsid                        *wsidp            = NULL;

   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error = uid_errnumInvalidRequest;

   PINFO("In SPL2 AFU message handler, AFUCommand().\n");

   // Perform some basic checks while assigning the pdev
   if ( NULL == sessp ) {
      PDEBUG("Error: No session\n");
      return -EIO;
   }

   // Get the spl2 device
   pdev = spl2_sessionp_to_spl2dev(sessp);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }
   //============================

   // Get the request
   p_localpayload = kmalloc(pmsg->size, GFP_KERNEL);
   if ( !p_localpayload ) {
      PDEBUG("Error: kmalloc failed with size=%" PRIu64 "\n", pmsg->size);
      retval = -ENOMEM;
      goto ERROR;
   }
   if ( copy_from_user( p_localpayload, (void *) pmsg->payload, pmsg->size) ) {
      PDEBUG("Error: copy_from_user(p_localpayload=%p, pmsg->payload=%p, pmsg->size=%" PRIu64 ") failed.\n",
                p_localpayload,
                pmsg->payload,
                pmsg->size);
      retval = -EFAULT;
      goto ERROR;
   }


   // check for MAFU message first. Invalid for this device,
   // Flow will fall through the switch statement which follows.
   //  and the error event will be sent in the default clause.
   if ( aalui_mafucmd == pmsg->cmd ) {
      PDEBUG("Permission denied. Not Management AFU\n");
      request_error = uid_errnumPermission;
   }

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {

      AFU_COMMAND_CASE(fappip_afucmdSTART_SPL2_TRANSACTION) {
         if ( start_spl2_transaction(pownerSess,
                                     &Message,
                                     MessageContext,
                                     (struct spl2req *)p_localpayload) ) {
            retval = -EIO;
            goto ERROR;
         }
      } break;

      AFU_COMMAND_CASE(fappip_afucmdSTOP_SPL2_TRANSACTION) {
         if ( stop_spl2_transaction( pownerSess,
                                    &Message,
                                     MessageContext) ) {
            retval = -EIO;
            goto ERROR;
         }
      }break;


      AFU_COMMAND_CASE(fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE) {
         if ( set_spl2_context_workspace(pownerSess,
                                         &Message,
                                         MessageContext,
                                         (struct spl2req *)p_localpayload) ) {
            retval = -EIO;
            goto ERROR;
         }
      } break;

      AFU_COMMAND_CASE(fappip_getCSRmap) {
         struct spl2req *preq = (struct spl2req *)p_localpayload;

         if ( !spl2_dev_allow_map_csr_space(pdev) ) {
            pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
                                                             0,
                                                             (btPhysAddr)NULL,
                                                             0,
                                                             0,
                                                             0,
                                                             Message.m_tranID,
                                                             Message.m_context,
                                                             uid_errnumPermission);
            PERR("Direct API access not permitted on this device\n");

            retval = -EPERM;
         } else {

            //------------------------------------------------------------
            // Create the WSID object and add to the list for this session
            //------------------------------------------------------------
            if ( ( WSID_CSRMAP_WRITEAREA != preq->ahmreq.u.wksp.m_wsid ) &&
                 ( WSID_CSRMAP_READAREA  != preq->ahmreq.u.wksp.m_wsid ) ) {
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                0,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on fappip_getCSRmap\n");

               retval = -EINVAL;
            } else {

               wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate CSR workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_CSR;
               PDEBUG("Getting CSR %s Aperature WSID %p using id %llx .\n",
                         ((WSID_CSRMAP_WRITEAREA == preq->ahmreq.u.wksp.m_wsid) ? "Write" : "Read"),
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                 pownerSess->m_device,
                                 wsidobjp_to_wid(wsidp),
                                 spl2_dev_phys_afu_csr(pdev),        // Return the requested aperture
                                 spl2_dev_len_afu_csr(pdev),         // Return the requested aperture size
                                 SPL2PIP_CSR_SIZE,                   // Return the CSR size in octets
                                 SPL2PIP_CSR_SPACING,                // Return the inter-CSR spacing octets
                                 Message.m_tranID,
                                 Message.m_context,
                                 uid_errnumOK);

               PVERBOSE("Sending uid_wseventCSRMap Event\n");

               retval = 0;
            }
         }

         pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;

#if 0
   //============================
   //  Get/Set CSR block function
   //============================
   case fappip_afucmdCSR_GETSET:{

      // Used by Get/Set CSR
      unsigned             u;
      unsigned             index;
      csr_read_write_blk  *pcsr_rwb;
      unsigned             num_to_set;
      csr_offset_value    *pcsr_array;

      DPRINTF(ENCODER_DBG_AFU, "Get/Set function\n");
      // Make sure we are the transaction owner
      if( !spl2_sessionp_is_tranowner(sessp){
         // Device is currently busy.
          pafuresponse_evt = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                             &Message.m_tranID,
                                                             Message.m_context,
                                                             NULL,
                                                             uid_errnumPermission);
          // Send the event
          pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                         pownerSess->m_device,
                                         AALQIP(pafuresponse_evt),
                                         MessageContext);
          goto failed;

      }
      index       = 0;
      pcsr_rwb    = (csr_read_write_blk*) p_localpayload;
      num_to_set  = pcsr_rwb->num_to_set;
      pcsr_array  = csr_rwb_setarray(pcsr_rwb);

      DPRINTF(ENCODER_DBG_AFU, "Num to set %d\n", num_to_set);

      // Execute the simulator
      for (u = 0; u < num_to_set; ++u) {

         // Invoke the PIP writecsr method. (See encoder write csr
         if( 0 != encoder_writecsr( sessp->pencoderafu, pcsr_array[u].csr_offset, pcsr_array[u].csr_value) ){
            pafuresponse_evt = uidrv_event_afu_afucsrgetset_create(  pownerSess->m_device,
                                                                     pcsr_rwb,
                                                                     u,
                                                                     &Message.m_tranID,
                                                                     Message.m_context,
                                                                     uid_errnumOK);

         }
      }

      if( NULL == pafuresponse_evt){
         // Create the response event
         pafuresponse_evt = uidrv_event_afu_afucsrgetset_create( pownerSess->m_device,
                                                                 pcsr_rwb,
                                                                 index,
                                                                 &Message.m_tranID,
                                                                 Message.m_context,
                                                                 uid_errnumOK);
      }

      if (unlikely(pafuresponse_evt == NULL)) {
         DPRINTF(ENCODER_DBG_AFU, "Exception creating event! No memory\n");
         retval = -ENOMEM;
         goto failed;
      }

      // Send the event
      pownerSess->m_uiapi->sendevent(sessp->m_pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuresponse_evt),
                                     Message.m_context);

      break;
   }
   //============================
   //  Submit the transaction
   //============================
    case fappip_afucmdDESC_SUBMIT:
   {
      struct      ahm_req req                         = *(struct ahm_req*)p_localpayload;
      struct      memmgr_session* pmem_sess           = spl2_dev_mem_sessionp(pdev);
      struct      submit_descriptors_req *submit_req  = NULL;
      btUnsigned64bitInt    uvaddr                    = 0;
      btPhyusAddr           paddr                     = 0;
      xsid_t      xsid                                = 0;

      //////////////////////////////////////////////////////////////////////////////////////////
      // Input is a submit_descriptors (defined in fappip.h), e.g. in the variable pdesc.

      // Get the user request into the pipmessage
      PVERBOSE(AHMPIP_DBG_MSGHDLR, "pmsg->payload = 0x%p: pmsg->size = %ld\n", pmsg->payload, pmsg->size );
      if (pmsg->size <= sizeof(submit_descriptors_req_t)) {
         PDEBUG("Error: No Descriptor.\n");
         // Create the event
         pafuresponse_evt
                = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                  &Message.m_tranID,
                                                  Message.m_context,
                                                  NULL,
                                                  uid_errnumDescArrayEmpty);
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                      pownerSess->m_device,
                                      AALQIP(pafuresponse_evt),
                                      MessageContext);
         return -1;
      } // if (pmsg->size <= sizeof(submit_descriptors_req_t))

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      submit_req = kmalloc(pmsg->size, GFP_KERNEL);
      if (submit_req == NULL) {
       PDEBUG("Error: Allocat desc info array failed:\n");
       // Create the event
       pafuresponse_evt = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                          &Message.m_tranID,
                                                          Message.m_context,
                                                          NULL,
                                                          uid_errnumNoMem);
       // Send the event
       pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                      pownerSess->m_device,
                                      AALQIP(pafuresponse_evt),
                                      MessageContext);
       goto failed;
      } // if (submit_req == NULL)

      if (copy_from_user(submit_req, (void *) pmsg->payload, pmsg->size)) {
       PDEBUG("Error: Copy submit_descriptors_req from user space failed:\n");
       // Create the event
       pafuresponse_evt = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                          &Message.m_tranID,
                                                          Message.m_context,
                                                          NULL,
                                                          uid_errnumCopyFromUser);
       // Send the event
       pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                      pownerSess->m_device,
                                      AALQIP(pafuresponse_evt),
                                      MessageContext);
       goto failed;
      } // if (copy_from_user(submit_req, (void *) pmsg->payload, pmsg->size))

      if (submit_req->m_nDesc != 1) {
         PDEBUG("Error: nDesc value not 1.\n");
         // Create the event
         pafuresponse_evt
                  = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                    &Message.m_tranID,
                                                    Message.m_context,
                                                    NULL,
                                                    uid_errnumBadParameter);
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuresponse_evt),
                                        MessageContext);
         goto failed;
      } // if (submit_req->m_nDesc < 1)

      // Decode the WSID object from wsid
      wsidp = wsid_to_wsidobjp(submit_req->m_arrDescInfo[0].m_wsid);

      // Get xsid from wsid object
      xsid = wsidp->m_id;

      // Get the address from the workspace
      if( !spl2_memsession_valid_xsid(xsid) ){
         PDEBUG("Error: Invalid xsid\n");
         // Create the event
         pafuresponse_evt
                   = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                     &Message.m_tranID,
                                                     Message.m_context,
                                                     NULL,
                                                     uid_errnumBadParameter);
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                         pownerSess->m_device,
                                         AALQIP(pafuresponse_evt),
                                         MessageContext);
          goto failed;

      }

      uvaddr = (btUnsigned64bitInt)spl2_memsession_virtmem(pmem_sess,xsid).m_pte_array_uvaddr;
      paddr  = (btUnsigned64bitInt)spl2_memsession_virtmem(pmem_sess,xsid).m_pte_array_physaddr;
      if((0 == uvaddr) || (0 == paddr)){
         PDEBUG("Error: Invalid address from workspace. UVaddr = %p  PhyADDR = %p\n",uvaddr,paddr );
         // Create the event
         pafuresponse_evt
                   = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                     &Message.m_tranID,
                                                     Message.m_context,
                                                     NULL,
                                                     uid_errnumBadParameter);
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                         pownerSess->m_device,
                                         AALQIP(pafuresponse_evt),
                                         MessageContext);
          goto failed;

      }

      // Setup SPL context
      spl2_dev_clrSPLCTX(devp);

      spl2_dev_SPLCTX(devp)->phys_addr_page_table  = paddr;
      spl2_dev_SPLCTX(devp)->virt_addr_afu_context = uvaddr;
      spl2_dev_SPLCTX(devp)->m_num_of_valid_pte    = spl2_memsession_virtmem(pmem_sess,xsid).m_num_of_valid_pte;
      spl2_dev_SPLCTX(devp)->page_size             = 1;
      spl2_dev_SPLCTX(devp)->control_flags         = 1;

      // Set the AFU context
      write_cci_csr64( byte_offset_AFU_CNTXT_BASE, uvaddr);

      // Set the SPL context (physical address)
      write_cci_csr64( byte_offset_SPL2_CNTXT_BASE, virt_to_phys(spl2_dev_SPLCTX(devp)));

      pafuresponse_evt
                 = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                   &Message.m_tranID,
                                                   Message.m_context,
                                                   NULL,
                                                   uid_errnumOK);
       // Send the event
       pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                       pownerSess->m_device,
                                       AALQIP(pafuresponse_evt),
                                       MessageContext);

      break;
   }
#endif
   //============================
   //  Allocate Workspace
   //============================
   AFU_COMMAND_CASE(fappip_afucmdWKSP_VALLOC) {
      struct ahm_req         req;
      struct memmgr_session *pmem_sess = spl2_dev_mem_sessionp(pdev);
      xsid_t                 xsid;

      req = *(struct ahm_req *)p_localpayload;

      // Check if virtual workspace has been initialized
      if( virtmem_is_init(&pmem_sess->m_virtmem) ) {
         // Only support one virtual workspace, so only one virtual xsid
         PDEBUG("Cannot allocate multiple virtual workspaces. Aborting EPERM\n");
         pafuws_evt = uidrv_event_afu_afuallocws_create((btObjectType) pownerSess->m_device,
                                                        (btWSID) 0,
                                                        NULL,
                                                        (btPhysAddr)NULL,
                                                        req.u.wksp.m_size,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumNoMem);

         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         retval = -EPERM;
         goto ERROR;
      } else {
         // Normal flow -- create the needed virtual workspace. Always use 2 MB buffers.
         retval = virtmem_construct(&pmem_sess->m_virtmem, req.u.wksp.m_size, 21);
         if (retval) {
            pafuws_evt = uidrv_event_afu_afuallocws_create(pownerSess->m_device,
                                                           (btWSID) 0,
                                                           NULL,
                                                           (btPhysAddr)NULL,
                                                           req.u.wksp.m_size,
                                                           Message.m_tranID,
                                                           Message.m_context,
                                                           uid_errnumNoMem);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message.m_context);

            goto ERROR;
         }
      }

      // Create the internal workspace ID.
      xsid = xsid_ctor(0, e_memtype_virtualized);

      //------------------------------------------------------------
      // Create the WSID object and add to the list for this session
      //------------------------------------------------------------
      wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, (unsigned long long)xsid);
      if ( NULL == wsidp ) {
         PERR("Couldn't allocate task workspace\n");
         retval = -ENOMEM;
         /* send a failure event back to the caller? */
         goto ERROR;
      }

      wsidp->m_size = pmem_sess->m_virtmem.m_vsize;
      wsidp->m_type = WSM_TYPE_VIRTUAL;
      PDEBUG("Creating WSID %p using id %llx .\n", wsidp, xsid);

      // Add the new wsid onto the session
      aalsess_add_ws(pownerSess, wsidp->m_list);

      PINFO("WS alloc wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR " size=%" PRIu64 " success!\n",
               req.u.wksp.m_wsid,
               pmem_sess->m_virtmem.m_pte_array_physaddr,
               req.u.wksp.m_size);

      // Create the event
      pafuws_evt = uidrv_event_afu_afuallocws_create(
                                            aalsess_aaldevicep(pownerSess),
                                            wsidobjp_to_wid(wsidp), // make the wsid appear page aligned for mmap
                                            NULL,
                                            pmem_sess->m_virtmem.m_pte_array_physaddr,
                                            req.u.wksp.m_size,
                                            Message.m_tranID,
                                            Message.m_context,
                                            uid_errnumOK);

      PVERBOSE("Sending the WKSP Alloc event.\n");
      // Send the event
      pownerSess->m_uiapi->sendevent(aalsess_uiHandle(pownerSess),
                                     aalsess_aaldevicep(pownerSess),
                                     AALQIP(pafuws_evt),
                                     Message.m_context);

   } break; // case fappip_afucmdWKSP_VALLOC

   //============================
   //  Free Workspace
   //============================
   AFU_COMMAND_CASE(fappip_afucmdWKSP_VFREE) {
      struct ahm_req         req;
      struct memmgr_session *pmem_sess = spl2_dev_mem_sessionp(pdev);
      xsid_t                 xsid;

      req = *(struct ahm_req*)p_localpayload;

      ASSERT(0 != req.u.wksp.m_wsid);
      if ( 0 == req.u.wksp.m_wsid ) {
         PDEBUG("WKSP_IOC_FREE: WS id can't be 0.\n");
         // Create the exception event
         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);

         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);
         retval = -EFAULT;
         goto ERROR;
      }

      // Get the workspace ID object
      wsidp = wsid_to_wsidobjp(req.u.wksp.m_wsid);

      ASSERT(wsidp);
      if ( NULL == wsidp ) {
         // Create the exception event
         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);

         PDEBUG("Sending WKSP_FREE Exception\n");
         // Send the event
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         retval = -EFAULT;
         goto ERROR;
      }

      // Free the buffer
      xsid = (xsid_t)wsidp->m_id;
#if 0
      // Get the address from the workspace
      if( !spl2_memsession_valid_xsid(xsid) ){
          PDEBUG("Error: Invalid xsid\n");
          // Create the event
          pafuresponse_evt
                    = uidrv_event_afutrancmplt_create(pownerSess->m_device,
                                                      &Message.m_tranID,
                                                      Message.m_context,
                                                      NULL,
                                                      uid_errnumBadParameter);
          // Send the event
          pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                          pownerSess->m_device,
                                          AALQIP(pafuresponse_evt),
                                          MessageContext);
           goto failed;

       }
#endif
      if( index_from_xsid(xsid) != 0 ) {
         PDEBUG( "Virtual free failed due to bad xsid index. Should be 0, but is %d\n",
                index_from_xsid(xsid));
         pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message.m_tranID,
                                                        Message.m_context,
                                                        uid_errnumBadParameter);
         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message.m_context);

         retval = -EFAULT;
         goto ERROR;
      }

      // Destroy the workspace
      virtmem_destruct(&spl2_memsession_virtmem(pmem_sess,xsid));

      // remove the wsid from the device and destroy
      list_del_init(&wsidp->m_list);
      pownerSess->m_uiapi->freewsid(wsidp);

      // Create the  event
      pafuws_evt = uidrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                     Message.m_tranID,
                                                     Message.m_context,
                                                     uid_errnumOK);

      PVERBOSE("Sending the WKSP Free event.\n");
      // Send the event
      pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuws_evt),
                                     Message.m_context);
   } break; // case fappip_afucmdWKSP_FREE

   default: {
      PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);

      pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                  &Message.m_tranID,
                                                                  Message.m_context,
                                                                  request_error);

      pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                     pownerSess->m_device,
                                     AALQIP(pafuresponse_evt),
                                     Message.m_context);

      retval = -EINVAL;
   } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);
   return retval;

ERROR:
   if ( NULL != p_localpayload ) {
      kfree(p_localpayload);
   }

   return retval;
}

