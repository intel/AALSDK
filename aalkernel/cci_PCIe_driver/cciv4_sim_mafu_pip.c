//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
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
//  Copyright(c) 2012-2015, Intel Corporation.
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
//        FILE: cciv4_sim_mafu_pip.c
//     CREATED: 07/29/2015
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          CCIV4 MAFU for simulated devices
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIV4_DBG_MOD

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/spl2defs.h"
#include "aalsdk/kernel/aalmafu-events.h"
#include "aalsdk/kernel/fappip.h"

#include "cciv4_PIPsession.h"

int MAFUCommandHandler(struct aaldev_ownerSession * ,
                       struct aal_pipmessage ,
                       void * );

int CMAFUCommandHandler(struct aaldev_ownerSession * ,
                        struct aal_pipmessage ,
                        void * );

struct uidrv_event_afu_response_event *
                  do_mafucmdCreateAFU( struct aaldev_ownerSession *,
                                       struct aal_pipmessage *,
                                       struct mafu_request *);

struct uidrv_event_afu_response_event *
                  do_mafucmdDestroyAFU( struct aaldev_ownerSession *,
                                       struct aal_pipmessage *,
                                       struct mafu_request *);


//=============================================================================
// Name: MAFUpip
// Description: Physical Interface Protocol for the SPL2 MAFU
//=============================================================================
struct aal_ipip cciv4_simMAFUpip =
{
   .m_messageHandler = {
      .sendMessage   = MAFUCommandHandler,   // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
      .mmap = cciv4_sim_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device

};

//=============================================================================
// Name: MAFUCommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
//         MessageContext - used by UIDRV (TODO deprecate)
// Outputs: none.
// Comments:
//=============================================================================
int
MAFUCommandHandler(struct aaldev_ownerSession *pownerSess,
                   struct aal_pipmessage       Message,
                   void                       *MessageContext)
{
#if (1 == ENABLE_DEBUG)
#define MAFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define MAFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG

   struct uidrv_event_afu_response_event *pafuresponse_evt = NULL;
   struct uidrv_event_afu_workspace_event *pafuws_evt       = NULL;
   struct mafu_request                    req;
   struct cciv4_device  *pdev  = NULL;

   // Used by WS allocation
   struct aal_wsid                        *wsidp            = NULL;

   struct cciv4_PIPsession *pSess = (struct cciv4_PIPsession *)aalsess_pipHandle(pownerSess);

   // Generalized payload pointer. Points to locally allocated and copied
   //    version of pmsg->payload of length pmsg->size
   void *p_localpayload = NULL;

   // Get the UI Driver command
   struct aalui_AFUmessage *pmsg = (struct aalui_AFUmessage *)Message.m_message;

   int retval = 0;
   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cciv4 device
   pdev = cciv4_PIPsessionp_to_cciv4dev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }

   PDEBUG("In CCIV4 AHM MAFU command handler.\n");

   // Check for MAFU message first. Invalid for this device.
   // Flow will fall through the switch statement which follows.
   //  and the error event will be sent in the default clause.
   if ( aalui_mafucmd == pmsg->cmd ) {
      // Get the user MAFU request into the pipmessage
      if ( copy_from_user(&req, (void *) pmsg->payload, sizeof(struct mafu_request)) ) {
         return -EFAULT;
      }
      // MAFU Command Handler
      switch (req.cmd) {

         MAFU_COMMAND_CASE(aalui_mafucmdCreateAFU) {

            pafuresponse_evt = do_mafucmdCreateAFU(pownerSess, &Message, &req);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);

         } break;

         MAFU_COMMAND_CASE(aalui_mafucmdDestroyAFU) {

            pafuresponse_evt = do_mafucmdDestroyAFU(pownerSess, &Message, &req);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
          } break;

         default: {
            ASSERT(0);
            pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                       &Message.m_tranID,
                                                                        Message.m_context,
                                                                        uid_errnumBadParameter);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
            retval = -EINVAL;
         } break;
      }//switch (req.cmd)

   }else{
      // Get the general request copied into local memory
      p_localpayload = kosal_kzmalloc(pmsg->size);
      if( !p_localpayload ) {
         PDEBUG("Error: kosal_kzmalloc failed with size=%" PRIu64 "\n", pmsg->size);
         retval = -ENOMEM;
         goto ERROR;
      }
      if ( copy_from_user( p_localpayload, (void *) pmsg->payload, pmsg->size) ) {
         PDEBUG("Error: copy_from_user(p_localpayload=%p, pmsg->payload=%p, pmsg->size=%" PRIu64 ") failed.\n", p_localpayload,
                                                                                                                pmsg->payload,
                                                                                                                pmsg->size);
         retval = -EFAULT;
         goto ERROR;
      }

      // AFU Command Handler
      switch (pmsg->cmd) {
          // Returns a workspace ID for the Config Space
          MAFU_COMMAND_CASE(fappip_getCSRmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_csr_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_cci_csr(pdev), (int)cciv4_dev_len_cci_csr(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_cci_csr(pdev),        // Return the requested aperture
                                     cciv4_dev_len_cci_csr(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

          // Returns a workspace ID for the MMIO-R Space
          MAFU_COMMAND_CASE(fappip_getMMIORmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_mmior_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_afu_mmio(pdev), (int)cciv4_dev_len_afu_mmio(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_afu_mmio(pdev),        // Return the requested aperture
                                     cciv4_dev_len_afu_mmio(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

          MAFU_COMMAND_CASE(fappip_getuMSGmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_umsg_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_afu_umsg(pdev), (int)cciv4_dev_len_afu_umsg(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_afu_umsg(pdev),        // Return the requested aperture
                                     cciv4_dev_len_afu_umsg(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

         default: {
            ASSERT(0);
            pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                       &Message.m_tranID,
                                                                        Message.m_context,
                                                                        uid_errnumBadParameter);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
            retval = -EINVAL;
         } break;
      }//switch (pmsg->cmd)
   }// if ( aalui_mafucmd == pmsg->cmd )
   ASSERT(0 == retval);

ERROR:
   if ( NULL != p_localpayload ) {
      kfree(p_localpayload);
   }

   return retval;
}


//=============================================================================
// Name: MAFUpip
// Description: Physical Interface Protocol for the SPL2 MAFU
//=============================================================================
struct aal_ipip cciv4_simCMAFUpip =
{

   .m_messageHandler = {
      .sendMessage   = CMAFUCommandHandler,  // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
     .mmap = cciv4_sim_mmap,
   },

  // Methods for binding and unbinding PIP to generic aal_device
  //  Unused in this PIP
  .binddevice    = NULL,      // Binds the PIP to the device
  .unbinddevice  = NULL,      // Binds the PIP to the device

};

//=============================================================================
// Name: CMAFUCommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
//         MessageContext - used by UIDRV (TODO deprecate)
// Outputs: none.
// Comments:
//=============================================================================
int
CMAFUCommandHandler(struct aaldev_ownerSession *pownerSess,
                    struct aal_pipmessage       Message,
                    void                       *MessageContext)
{
#if (1 == ENABLE_DEBUG)
#define MAFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define MAFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG

   struct uidrv_event_afu_response_event *pafuresponse_evt = NULL;
   struct uidrv_event_afu_workspace_event *pafuws_evt       = NULL;
   struct mafu_request                    req;
   struct cciv4_device  *pdev  = NULL;

   // Used by WS allocation
   struct aal_wsid                        *wsidp            = NULL;

   struct cciv4_PIPsession *pSess = (struct cciv4_PIPsession *)aalsess_pipHandle(pownerSess);

   // Generalized payload pointer. Points to locally allocated and copied
   //    version of pmsg->payload of length pmsg->size
   void *p_localpayload = NULL;

   // Get the UI Driver command
   struct aalui_AFUmessage *pmsg = (struct aalui_AFUmessage *)Message.m_message;

   int retval = 0;
   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cciv4 device
   pdev = cciv4_PIPsessionp_to_cciv4dev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }

   PDEBUG("In CCIV4 AHM MAFU command handler.\n");

   // Check for MAFU message first. Invalid for this device.
   // Flow will fall through the switch statement which follows.
   //  and the error event will be sent in the default clause.
   if ( aalui_mafucmd == pmsg->cmd ) {
      // Get the user MAFU request into the pipmessage
      if ( copy_from_user(&req, (void *) pmsg->payload, sizeof(struct mafu_request)) ) {
         return -EFAULT;
      }
      // MAFU Command Handler
      switch (req.cmd) {

         MAFU_COMMAND_CASE(aalui_mafucmdCreateAFU) {

            pafuresponse_evt = do_mafucmdCreateAFU(pownerSess, &Message, &req);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);

         } break;

         MAFU_COMMAND_CASE(aalui_mafucmdDestroyAFU) {

            pafuresponse_evt = do_mafucmdDestroyAFU(pownerSess, &Message, &req);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
          } break;

         default: {
            ASSERT(0);
            pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                       &Message.m_tranID,
                                                                        Message.m_context,
                                                                        uid_errnumBadParameter);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
            retval = -EINVAL;
         } break;
      }//switch (req.cmd)

   }else{
      // Get the general request copied into local memory
      p_localpayload = kosal_kzmalloc(pmsg->size);
      if( !p_localpayload ) {
         PDEBUG("Error: kosal_kzmalloc failed with size=%" PRIu64 "\n", pmsg->size);
         retval = -ENOMEM;
         goto ERROR;
      }
      if ( copy_from_user( p_localpayload, (void *) pmsg->payload, pmsg->size) ) {
         PDEBUG("Error: copy_from_user(p_localpayload=%p, pmsg->payload=%p, pmsg->size=%" PRIu64 ") failed.\n", p_localpayload,
                                                                                                                pmsg->payload,
                                                                                                                pmsg->size);
         retval = -EFAULT;
         goto ERROR;
      }

      // AFU Command Handler
      switch (pmsg->cmd) {
          // Returns a workspace ID for the Config Space
          MAFU_COMMAND_CASE(fappip_getCSRmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_csr_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_cci_csr(pdev), (int)cciv4_dev_len_cci_csr(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_cci_csr(pdev),        // Return the requested aperture
                                     cciv4_dev_len_cci_csr(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

          // Returns a workspace ID for the MMIO-R Space
          MAFU_COMMAND_CASE(fappip_getMMIORmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_mmior_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_afu_mmio(pdev), (int)cciv4_dev_len_afu_mmio(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_afu_mmio(pdev),        // Return the requested aperture
                                     cciv4_dev_len_afu_mmio(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

          MAFU_COMMAND_CASE(fappip_getuMSGmap) {
             struct cciv4req *preq = (struct cciv4req *)p_localpayload;

             if ( !cciv4_dev_allow_map_umsg_space(pdev) ) {
                PERR("Failed getCSR map Permission\n");
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
                   PERR("Failed getCSR map Parameter\n");
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

                   PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cciv4_dev_phys_afu_umsg(pdev), (int)cciv4_dev_len_afu_umsg(pdev));

                   // Return the event with all of the appropriate aperture descriptor information
                   pafuws_evt = uidrv_event_afu_afugetcsrmap_create(
                                     pownerSess->m_device,
                                     wsidobjp_to_wid(wsidp),
                                     cciv4_dev_phys_afu_umsg(pdev),        // Return the requested aperture
                                     cciv4_dev_len_afu_umsg(pdev),         // Return the requested aperture size
                                     CCIV4_CSR_SIZE,                      // Return the CSR size in octets
                                     CCIV4_CSR_SPACING,                   // Return the inter-CSR spacing octets
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

         default: {
            ASSERT(0);
            pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                       &Message.m_tranID,
                                                                        Message.m_context,
                                                                        uid_errnumBadParameter);

            pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuresponse_evt),
                                           Message.m_context);
            retval = -EINVAL;
         } break;
      }//switch (pmsg->cmd)
   }// if ( aalui_mafucmd == pmsg->cmd )
   ASSERT(0 == retval);

ERROR:
   if ( NULL != p_localpayload ) {
      kfree(p_localpayload);
   }

   return retval;
}


//=============================================================================
// Name: do_mafucmdCreateAFU
// Description: Implements the create AFU command
// Interface: public
// Inputs: pownerSess - Session between App and device
//         pMessage - Message to process
//         preq - request
// Outputs: Response event.
// Comments: The device is created with the parameters specified. This
//           initiates the Linux match/probe protocol where eventually the
//           PIP will get bound to the device.  At that point device
//           initialization and uevent processing will occur.
//=============================================================================
struct uidrv_event_afu_response_event *
                  do_mafucmdCreateAFU( struct aaldev_ownerSession *pownerSess,
                                       struct aal_pipmessage *pMessage,
                                       struct mafu_request *preq)
{

   // The PIP handle (aka context) is the cciv4_session. This was setup at
   //  afuBindSession time.
   struct cciv4_PIPsession  *pSess   = (struct cciv4_PIPsession  *)aalsess_pipHandle(pownerSess);
   struct cciv4_device      *pdev    = NULL;
//   afu_descriptor           afu_desc;
//   struct mafu_CreateAFU    CreateRequest;
   uid_errnum_e             res     = uid_errnumCouldNotCreate;

   PVERBOSE("Creating a simulated SPL2 device.\n");

   ASSERT(pSess);
   if ( NULL == pSess ) {
      PERR("Error: No session\n");
      goto ERR;
   }

   // Get the spl2 device
   pdev = cciv4_PIPsessionp_to_cciv4dev(pSess);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PERR("Error: No SPL2 device in session\n");
      goto ERR;
   }
#if 0
   // Cannot allocate more than one
   if( NULL != cciv4_dev_to_aaldev(pdev) ){
      PERR("Device already allocated for this spl2 simulation device\n");
      goto ERR;
   }

   // Copy the request header
   if ( copy_from_user(&CreateRequest,preq->payload,preq->size) ) {
      PERR("Copy req payload from user space failed:\n");
      res = uid_errnumCopyFromUser;
      goto ERR;
   }

   if ( 0 == CreateRequest.maxshares ) {
      CreateRequest.maxshares = 1;
   }

   // Set the internal deviID parameters so that the probe finds them.
   //   This is done via some slight of hand.  The AFU ID is passed down
   //   in the devid of the message.  This will be used by the simulator
   //   write_csr() when the AFU DSM is probed.

   PDEBUG("AFUID L 0x%Lx   H 0x%Lx",CreateRequest.device_id.m_afuGUIDl,CreateRequest.device_id.m_afuGUIDh );

   PVERBOSE("Making the AFU ID  0x%Lx  0x%Lx\n",
            aaldevid_afuguidh(cciv4_dev_to_sim_devid(pdev)),
            aaldevid_afuguidl(cciv4_dev_to_sim_devid(pdev)) );


   // Probe the hardware
   if ( cciv4_sim_internal_probe(pdev, &CreateRequest.device_id) ) {
      PERR("Probe failed\n");
      goto ERR;
   }

   CreateRequest.actionflags = 0;
   PVERBOSE("Action flags are %x\n",CreateRequest.actionflags);

   // Create the kernel device and register it with the platform
   if ( 0 != cciv4_create_discovered_afu( pdev, &CreateRequest.device_id, &SPLAFUpip) ) {
      PERR("Failed to create new device\n");
      goto ERR;
   }

   afu_desc.devid = CreateRequest.device_id;
   strncpy(afu_desc.basename, CreateRequest.basename, MAFU_MAX_BASENAME_LEN);

   return uidrv_event_mafu_ConfigureAFU_create(pownerSess->m_device,
                                              &afu_desc,
                                               uid_afurespAFUCreateComplete,
                                              &pMessage->m_tranID,
                                               pMessage->m_context,
                                               uid_errnumOK);
#endif
ERR:
   return uidrv_event_mafu_ConfigureAFU_create(pownerSess->m_device,
                                               NULL,
                                               uid_afurespAFUCreateComplete,
                                              &pMessage->m_tranID,
                                               pMessage->m_context,
                                               res);
}


//=============================================================================
// Name: do_mafucmdDestroyAFU
// Description: Implements destroy AFU command
// Interface: public
// Inputs: pownerSess - Session between App and device
//         pMessage - Message to process
//         preq - request
// Outputs: Response event.
// Comments: The device can only be destroyed if it was created by this driver.
//           The actual device is not destroyed here but rather the unregister
//           process started.  The final cleanup is handled in the device's
//           Release() method called by Linux.
//=============================================================================
struct uidrv_event_afu_response_event *
                  do_mafucmdDestroyAFU( struct aaldev_ownerSession *pownerSess,
                                       struct aal_pipmessage *pMessage,
                                       struct mafu_request *preq)
{
#if 0
   struct cciv4_session     *pSess   = (struct cciv4_session  *)aalsess_pipHandle(pownerSess);
   struct cciv4_device      *pdev    = NULL;
   struct mafu_DestroyAFU  DestroyRequest;

   memset(&DestroyRequest, 0, sizeof(struct mafu_DestroyAFU));

   if ( NULL == pSess ) {
        PDEBUG("Error: No session\n");
        return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                      NULL,
                                                      uid_afurespAFUDestroyComplete,
                                                      &pMessage->m_tranID,
                                                      pMessage->m_context,
                                                      uid_errnumCouldNotCreate);
   }

   // Get the cciv4 device
   pdev = cciv4_PIPsessionp_to_cciv4dev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No SPL2 device in session\n");
      return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                    NULL,
                                                    uid_afurespAFUDestroyComplete,
                                                    &pMessage->m_tranID,
                                                    pMessage->m_context,
                                                    uid_errnumCouldNotDestroy);
   }

   // Cannot allocate more than one
   if( NULL == cciv4_dev_to_aaldev(pdev) ){
      PDEBUG("Device already allocated for this spl2 simulation device\n");
      return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                    NULL,
                                                    uid_afurespAFUDestroyComplete,
                                                    &pMessage->m_tranID,
                                                    pMessage->m_context,
                                                    uid_errnumCouldNotDestroy);
   }

   // Read the reques but currently ignored as there is only one AFU to destroy...
   //  the front end one.  Could check parameters but not now TODO
   if ( copy_from_user(&DestroyRequest,preq->payload,preq->size) ) {
       PDEBUG(" Copy req payload from user space failed:\n");
       // Create the event
       return uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                    NULL,
                                                    uid_afurespAFUDestroyComplete,
                                                   &pMessage->m_tranID,
                                                    pMessage->m_context,
                                                    uid_errnumCopyFromUser);

   }

   // Unregistering the device will cause the Linux release protocol
   //   to kick in.  This will result in the device being unregistered
   //   from the driver and the devices Release() method being called.
   aalbus_get_bus()->unregister_device(cciv4_dev_to_aaldev(pdev));
   cciv4_dev_to_aaldev(pdev)=NULL;

   // Device destroy started.
   return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                &DestroyRequest.dev_desc,
                                                 uid_afurespAFUDestroyComplete,
                                                &pMessage->m_tranID,
                                                 pMessage->m_context,
                                                 uid_errnumOK);
#endif
   return NULL;
}  // do_mafucmdDestroyAFU()


