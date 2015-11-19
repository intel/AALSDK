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
//        FILE: spl2_mafu_pip.c
//     CREATED: 02/09/2012
//      AUTHOR: Joseph Grecco, Intel
// PURPOSE: This file implements the Intel(R) QuickAssist Technology AAL
//          SPL2 MAFU for simualted devices
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_MAFU

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/spl2defs.h"
#include "aalsdk/kernel/aalmafu-events.h"
#include "spl2_session.h"  // Includes spl2_pip_internal.h

int
MAFUCommand(struct aaldev_ownerSession * ,
            struct aal_pipmessage *);

static void    spl2_sim_read_cci_csr(struct spl2_device * , btCSROffset );
static void spl2_sim_write_cci_csr32(struct spl2_device * , btCSROffset , bt32bitCSR );

//=============================================================================
// Name: MAFUpip
// Description: Physical Interface Protocol for the SPL2 MAFU
//=============================================================================
struct spl2_MAFUpip MAFUpip =
{
   .m_ipip = {
      .m_messageHandler = {
         .sendMessage   = MAFUCommand,       // Command Handler
         .bindSession   = AFUbindSession,    // Session binder
         .unBindSession = AFUunbindSession,  // Session unbinder
      },
#if 0
   .m_fops = {
     .mmap = encoder_mmap,
   },
#endif
     // Methods for binding and unbinding PIP to generic aal_device
     //  Unused in this PIP
     .binddevice    = NULL,      // Binds the PIP to the device
     .unbinddevice  = NULL,      // Binds the PIP to the device
   },

   .read_cci_csr     = spl2_sim_read_cci_csr,
   .write_cci_csr32  = spl2_sim_write_cci_csr32,
};
//=============================================================================
// Name: spl2_sim_read_cci_csr
// Description: Called when a simulated device gets a read csr call
// Input: pdev - device
//        offset - to read
// Comment:
// Returns: value
// Comments:
//=============================================================================
void spl2_sim_read_cci_csr(struct spl2_device *pdev, btCSROffset offset)
{
   PVERBOSE("spl2_sim_read_cci_csr got offset 0x%x",offset);
   if ( NULL != spl2_dev_to_simsession(pdev) ) {
      PVERBOSE("Processing by simulator application\n");
   }
   //
    // Process special offsets
    switch(offset){
       case byte_offset_CCI_CH_STAT0:
       case byte_offset_CCI_CH_STAT1:
          {

             char       volatile *p  = ((char volatile *)spl2_dev_kvp_cci_csr(pdev)) + offset; // offset is in bytes
             bt32bitCSR volatile *up = (bt32bitCSR volatile *)p;
             PVERBOSE("Setting CH_STATx to 0x80000000\n");
             *up=0x80000000;
             break;
          }

       default:
          break;
    }
}  // read_cci_csr


//=============================================================================
// Name: spl2_sim_write_cci_csr32
// Description: called when a simulated device gets a write to a csr
// Input: pdev - device
//        offset - to write to
//        value - value
// Comment:
// Returns: value
// Comments:
//=============================================================================
static void spl2_sim_write_cci_csr32(struct spl2_device *pdev,
                                     btCSROffset         offset,
                                     bt32bitCSR          value)
{
   static bt64bitCSR valueh = 0;

   // If we are attached to an application defer processing to application
   // TBD
   PVERBOSE("spl2_sim_write_cci_csr32 got offset 0x%x  value 0x%x",offset,value);
   if ( NULL != spl2_dev_to_simsession(pdev) ) {
      PVERBOSE("Processing by simulator application\n");
   }

   //
   // Process special offsets
   switch(offset){
      case byte_offset_SPL_DSM_BASE:
         {
            ((struct CCIAFU_DSM *)pdev->m_SPL2DSM)->cci_afu_id = SPL2_ID;
            break;
         }
      case byte_offset_CSR_AFU_DSM_BASE+4:
      {
         valueh = (bt64bitCSR)value << 32;
         break;
      }
      case byte_offset_CSR_AFU_DSM_BASE:
         {
            spl2_dev_AFUDSM_type volatile *pAFU_DSM = NULL;
            bt64bitCSR physaddr = valueh + value;
            valueh = 0;

            pAFU_DSM = (spl2_dev_AFUDSM_type *)phys_to_virt(physaddr);
            // Set DSM and set the AFUID

            pAFU_DSM->vafu2.AFU_ID[1] = 0xC000C9660D824272L;
            pAFU_DSM->vafu2.AFU_ID[0] = 0x9AEFFE5F84570612L;
            PVERBOSE("Setting Simulated DSM @ 0x%p [phys 0x%" PRIx64 "] and setting AFUID to 0x%" PRIx64 " 0x%" PRIx64 "\n",
                                                                                       pAFU_DSM,
                                                                                       physaddr,
                                                                                       pAFU_DSM->vafu2.AFU_ID[1],
                                                                                       pAFU_DSM->vafu2.AFU_ID[0]);
            break;
         }

      case byte_offset_AFU_DSM_BASE:
         {
            // Set the DSM AFU_ID to the value set in the simulator device ID.
            spl2_dev_AFUDSM(pdev)->vafu2.AFU_ID[0] = aaldevid_afuguidl( spl2_dev_to_sim_devid(pdev)  );
            spl2_dev_AFUDSM(pdev)->vafu2.AFU_ID[1] = aaldevid_afuguidh( spl2_dev_to_sim_devid(pdev)  );
         }

      default:
         break;
   }

}  // driver_write_cci_csr

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
   // The PIP handle (aka context) is the spl2_session. This was setup at
   //  afuBindSession time.
   struct spl2_session     *sessp   = (struct spl2_session  *)aalsess_pipHandle(pownerSess);
   struct spl2_device      *pdev    = NULL;
   afu_descriptor           afu_desc;
   struct mafu_CreateAFU    CreateRequest;
   uid_errnum_e             res     = uid_errnumCouldNotCreate;

   PVERBOSE("Creating a simulated SPL2 device.\n");

   ASSERT(sessp);
   if ( NULL == sessp ) {
      PERR("Error: No session\n");
      goto ERR;
   }

   // Get the spl2 device
   pdev = spl2_sessionp_to_spl2dev(sessp);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PERR("Error: No SPL2 device in session\n");
      goto ERR;
   }

   // Cannot allocate more than one
   if( NULL != spl2_dev_to_aaldev(pdev) ){
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
   spl2_dev_to_sim_devid(pdev) = CreateRequest.device_id;

   PVERBOSE("Making the AFU ID  0x%Lx  0x%Lx\n",
            aaldevid_afuguidh(spl2_dev_to_sim_devid(pdev)),
            aaldevid_afuguidl(spl2_dev_to_sim_devid(pdev)) );


   // Probe the hardware
   if ( spl2_sim_internal_probe(pdev, &CreateRequest.device_id) ) {
      PERR("Probe failed\n");
      goto ERR;
   }

   CreateRequest.actionflags = 0;
   PVERBOSE("Action flags are %x\n",CreateRequest.actionflags);

   // Create the kernel device and register it with the platform
   if ( 0 != spl2_create_discovered_afu( pdev, &CreateRequest.device_id, &SPLAFUpip) ) {
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
   struct spl2_session     *sessp   = (struct spl2_session  *)aalsess_pipHandle(pownerSess);
   struct spl2_device      *pdev    = NULL;
   struct mafu_DestroyAFU  DestroyRequest;

   memset(&DestroyRequest, 0, sizeof(struct mafu_DestroyAFU));

   if ( NULL == sessp ) {
        PDEBUG("Error: No session\n");
        return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                      NULL,
                                                      uid_afurespAFUDestroyComplete,
                                                      &pMessage->m_tranID,
                                                      pMessage->m_context,
                                                      uid_errnumCouldNotCreate);
   }

   // Get the spl2 device
   pdev = spl2_sessionp_to_spl2dev(sessp);
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
   if( NULL == spl2_dev_to_aaldev(pdev) ){
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
   aalbus_get_bus()->unregister_device(spl2_dev_to_aaldev(pdev));
   spl2_dev_to_aaldev(pdev)=NULL;

   // Device destroy started.
   return  uidrv_event_mafu_ConfigureAFU_create( pownerSess->m_device,
                                                &DestroyRequest.dev_desc,
                                                 uid_afurespAFUDestroyComplete,
                                                &pMessage->m_tranID,
                                                 pMessage->m_context,
                                                 uid_errnumOK);

   return NULL;
}  // do_mafucmdDestroyAFU()


//=============================================================================
// Name: MAFUCommand
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
//         MessageContext - used by UIDRV (TODO deprecate)
// Outputs: none.
// Comments:
//=============================================================================
int
MAFUCommand(struct aaldev_ownerSession *pownerSess,
            struct aal_pipmessage       *Message)
{
#if (1 == ENABLE_DEBUG)
#define MAFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define MAFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG
   struct uidrv_event_afu_response_event *pafuresponse_evt = NULL;
   struct mafu_request                    req;

   // Get the UI Driver command
   struct aalui_AFUmessage *pmsg = (struct aalui_AFUmessage *)Message->m_message;

   int retval = 0;

   PDEBUG("In SPL2 MAFU message handler.\n");

   // Get the user request into the pipmessage
   if ( copy_from_user(&req, (void *) pmsg->payload, sizeof(struct mafu_request)) ) {
      return -EFAULT;
   }

   // Process the command
   switch (req.cmd) {

      MAFU_COMMAND_CASE(aalui_mafucmdCreateAFU) {

         pafuresponse_evt = do_mafucmdCreateAFU(pownerSess, Message, &req);

         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuresponse_evt),
                                        Message->m_context);

      } break;

      MAFU_COMMAND_CASE(aalui_mafucmdDestroyAFU) {

         pafuresponse_evt = do_mafucmdDestroyAFU(pownerSess, Message, &req);

         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuresponse_evt),
                                        Message->m_context);
       } break;

      default: {
         ASSERT(0);
         pafuresponse_evt = uidrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                    &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumBadParameter);

         pownerSess->m_uiapi->sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuresponse_evt),
                                        Message->m_context);
         retval = -EINVAL;
      } break;
   }

   return retval;
}

