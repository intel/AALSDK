//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2014-2015, Intel Corporation.
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
//  Copyright(c) 2014-2015, Intel Corporation.
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
//        FILE: cci3_afu_pip.c
//     CREATED: 09/30/2014
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          CCI3 AFU PIP
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

// Must come before mem includes for Debug tracing
#include "spl2_session.h"     // Includes spl2_pip_internal.h

#include "aalsdk/kernel/vafu2defs.h"



static int
AFUCommand(struct aaldev_ownerSession *,
           struct aal_pipmessage,
           void *);


//=============================================================================
// Name:CCIAFUpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip CCIAFUpip = {
   .m_messageHandler = {
      .sendMessage   = AFUCommand,        // Command Handler
      .bindSession   = AFUbindSession,    // Session binder
      .unBindSession = AFUunbindSession,  // Session unbinder
   },

   .m_fops = {
     .mmap = cci3_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};

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
#endif
   //============================
   //  Allocate Workspace
   //============================
   AFU_COMMAND_CASE(fappip_afucmdWKSP_VALLOC) {
      struct ahm_req          req;
      btVirtAddr krnl_virt = NULL;

      req = *(struct ahm_req *)p_localpayload;


      // Normal flow -- create the needed workspace.
      krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(req.u.wksp.m_size);
      if (NULL == krnl_virt) {
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

      //------------------------------------------------------------
      // Create the WSID object and add to the list for this session
      //------------------------------------------------------------
      wsidp = pownerSess->m_uiapi->getwsid(pownerSess->m_device, (btWSID)krnl_virt);
      if ( NULL == wsidp ) {
         PERR("Couldn't allocate task workspace\n");
         retval = -ENOMEM;
         /* send a failure event back to the caller? */
         goto ERROR;
      }

      wsidp->m_size = req.u.wksp.m_size;
      wsidp->m_type = WSM_TYPE_PHYSICAL;
      PDEBUG("Creating Physical WSID %p.\n", wsidp);

      // Add the new wsid onto the session
      aalsess_add_ws(pownerSess, wsidp->m_list);

      PINFO("CCI WS alloc wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR  " kvp=0x%" PRIx64 " size=%" PRIu64 " success!\n",
               req.u.wksp.m_wsid,
               kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
               wsidp->m_id,
               wsidp->m_size);

      // Create the event
      pafuws_evt = uidrv_event_afu_afuallocws_create(
                                            aalsess_aaldevicep(pownerSess),
                                            wsidobjp_to_wid(wsidp), // make the wsid appear page aligned for mmap
                                            NULL,
                                            kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
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
      btVirtAddr krnl_virt = NULL;

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
      if(  WSM_TYPE_PHYSICAL != wsidp->m_type ) {
         PDEBUG( "Workspace free failed due to bad WS type. Should be %d but received %d\n",WSM_TYPE_PHYSICAL,
               wsidp->m_type);

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

      krnl_virt = (btVirtAddr)wsidp->m_id;

      kosal_free_contiguous_mem(krnl_virt, wsidp->m_size);

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

