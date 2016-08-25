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
/// @file ccip_pwr.c
/// @brief  Definitions for ccip Power AAL device.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_pwr.c
//     CREATED: July 25, 2016
//      AUTHOR:
//
// PURPOSE:   This file contains the implementation of the CCIP Signal Tap
//             low-level function (i.e., Physical Interface Protocol driver).
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "aalsdk/kernel/AALTransactionID_s.h"
#include "aalsdk/kernel/aalbus-ipip.h"
#include "aalsdk/kernel/ccip_defs.h"

#include "aalsdk/kernel/ccipdriver.h"
#include "ccipdrv-events.h"

#include "ccip_port.h"
#include "ccip_fme.h"
#include "cci_pcie_driver_PIPsession.h"

extern btUnsigned32bitInt sim;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              PIP INTERFACE               ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int CommandHandler( struct aaldev_ownerSession *,
                           struct aal_pipmessage*);

// TODO going to try and use a common mmapper for all objects
extern int cci_mmap( struct aaldev_ownerSession *pownerSess,
                     struct aal_wsid *wsidp,
                     btAny os_specific );


///=============================================================================
/// Name: cci_PWRpip
/// @brief Physical Interface Protocol Interface for the power AFU
///              kernel based AFU engine.
///=============================================================================
struct aal_ipip cci_PWRpip = {
   .m_messageHandler = {
      .sendMessage   = CommandHandler,       // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
     .mmap = cci_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};

///============================================================================
/// Name: cci_create_AAL_power_Device
/// @brief Creates and registers Power objects (resources) we want to expose
///        Through AAL.
///
/// @param[in] pccipdev - CCI Board object .
/// @return    error code
///============================================================================
btBool cci_create_AAL_power_Device(struct ccip_device * pccipdev)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   struct aal_device_id     aalid;
   int ret;

   PTRACEIN;

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   ASSERT(NULL != pcci_aaldev);
   if( NULL == pcci_aaldev){
      return false;
   }


   // Make it a User AFU
   cci_aaldev_type(pcci_aaldev)     = cci_dev_PWR;

   // Record parentage
   cci_aaldev_pfme(pcci_aaldev) = ccip_dev_to_fme_dev(pccipdev);
   cci_aaldev_pci_dev(pcci_aaldev) = ccip_dev_to_pci_dev(pccipdev);


   // Device Address is the same as the Port. Set the AFU ID information
   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(aalid)            = CCIP_PWR_GUIDL;
   aaldevid_afuguidh(aalid)            = CCIP_PWR_GUIDH;
   aaldevid_pipguid(aalid)             = CCIP_PWR_PIPIID;

   aaldevid_devaddr_bustype(aalid)     =  ccip_dev_pcie_bustype(pccipdev);

   aaldevid_devaddr_busnum(aalid)      = ccip_dev_pcie_busnum(pccipdev);
   aaldevid_devaddr_devnum(aalid)      = ccip_dev_pcie_devnum(pccipdev);
   aaldevid_devaddr_fcnnum(aalid)      = ccip_dev_pcie_fcnnum(pccipdev);
   aaldevid_devaddr_subdevnum(aalid)   = aaldevid_devaddr_fcnnum(aalid)++;  // PWR subdevice number is constant
   aaldevid_devaddr_instanceNum(aalid) ++;  // PWR is always instance 0


   aaldevid_devtype(aalid)             = aal_devtypeAFU;

   // Setup the MMIO region parameters
   // Create the AAL device and attach it to the CCI device object
   cci_aaldev_to_aaldev(pcci_aaldev) =  aaldev_create( "CCIPPWR",         // AAL device base name
                                                       &aalid,             // AAL ID
                                                       &cci_PWRpip);

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   cci_aaldev_to_aaldev(pcci_aaldev)->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_aaldev_to_aaldev(pcci_aaldev)) = (void*)pcci_aaldev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_aaldev_to_aaldev(pcci_aaldev), cci_release_device);

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pcci_aaldev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for CCIPPWR[%d:%d:%d:%d]",aaldevid_devaddr_busnum(aalid),
                                                                       aaldevid_devaddr_devnum(aalid),
                                                                       aaldevid_devaddr_fcnnum(aalid),
                                                                       aaldevid_devaddr_subdevnum(aalid));
      cci_destroy_aal_device(pcci_aaldev);
      return false;
   }

   ccip_dev_to_fme_dev(pccipdev)->m_power_aaldev= pcci_aaldev;

   kosal_list_add(&cci_aaldev_list_head(pcci_aaldev), &ccip_aal_dev_list(pccipdev));

   return true;
}


//=============================================================================
// Name: CommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
// Outputs: none.
// Comments:
//=============================================================================
int
CommandHandler(struct aaldev_ownerSession *pownerSess,
               struct aal_pipmessage       *Message)
{
#if (1 == ENABLE_DEBUG)
#define AFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define AFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG


   // Private session object set at session bind time (i.e., when object allocated)
   struct cci_PIPsession *pSess = (struct cci_PIPsession *)aalsess_pipHandle(pownerSess);
   struct cci_aal_device  *pdev  = NULL;

   // Overall return value for this function. Set before exiting if there is an error.
   //    retval = 0 means good return.
   int retval = 0;

   // UI Driver message
   struct aalui_CCIdrvMessage *pmsg = (struct aalui_CCIdrvMessage *) Message->m_message;

   // Save original response buffer size in case we return something
   //btWSSize         respBufSize     = Message->m_respbufSize;

   // Assume returning nothing. By setting the response buffer size to 0
   //   we tell the upstream side that there is no payload to copy back.
   //   Setting it here means we don't have to set it (or forget to) in each
   //   command.  We've recorded the payload buffer size above if we do need
   //   intend to send a payload.
   Message->m_respbufSize          = 0;

   PTRACEIN;

   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cci device
   pdev = cci_PIPsessionp_to_ccidev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {

      AFU_COMMAND_CASE(ccipdrv_PwrMgrResponse) {

         btInt  pr_pwrmgmt_status = 0;
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;

         if(NULL == preq) {
            PERR("Invalid Input parameter \n");
            Message->m_errcode = uid_errnumBadParameter;
            break;
         }
         pr_pwrmgmt_status = preq->ahmreq.u.pr_pwrmgmt.pr_pwrmgmt_status;
         if(0  == pr_pwrmgmt_status) {
            PVERBOSE("Power Idle done.\n");
         } else {
            PVERBOSE("pr_pwrmgr_status:%d \n",pr_pwrmgmt_status);
         }

         // TBD check Trans ID ?


         PVERBOSE("pSess->currTranID.m_intID:%lld \n",Message->m_tranID.m_intID);

         PVERBOSE("preq->afutskTranID.m_intID:%lld \n",preq->afutskTranID.m_intID);

         if(NULL != cci_aaldev_pfme(pdev)->m_pr_program_context) {
            reconfigure_bitstream_cancelPwrmgrTimer(cci_aaldev_pfme(pdev)->m_pr_program_context,pr_pwrmgmt_status);
         }
         // Success Event
         Message->m_errcode = uid_errnumOK;

      } break;

      default: {
         // No payload
         PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);
         Message->m_respbufSize          = 0;
         Message->m_errcode = uid_errnumInvalidRequest;
         retval = -EINVAL;
      } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

   return retval;
}

///============================================================================
/// Name: send_pr_power_event
/// @brief   sends power request to power manager demon.
///
/// @param[in] pReconf_context AFU reconfiguration context
/// @param[in] power_required  Power required
/// @return    0 = success , non zero failure.
///============================================================================
bt32bitInt send_pr_power_event(struct pr_program_context* pReconf_context,
                              int power_required)
{
   kosal_list_head *pitr              = NULL;
   kosal_list_head *temp              = NULL;
   struct aaldev_owner *pOwner        = NULL;
   struct fme_device *pfme_dev        = NULL;
   btInt socketID                     = 0;
   btInt busID                        = 0;
   btInt deviceID                     = 0;
   btInt functionID                   = 0;
   int res                            = 0;

   struct ccipdrv_event_afu_response_event *pafuws_evt      = NULL;

   PTRACEIN;

   if( NULL == pReconf_context ) {
      PERR("Invalid Input parameter \n");
      res =-EINVAL;
      return res;
   }

   // Storing PR Program Context
   pfme_dev =cci_aaldev_pfme(pReconf_context->m_pPR_dev);
   pfme_dev->m_pr_program_context = pReconf_context;

   if (0 == pfme_dev->m_power_aaldev->m_aaldev->m_numowners ) {
      PERR("Power AAL Resource   m_numowners 0 \n");
      res =-EINVAL;
      return res;
   }

   PVERBOSE( "Power owners count= %d \n", ccip_dev_fme_pwraal_dev(pfme_dev)->m_aaldev->m_numowners);

   if ( unlikely( kosal_sem_get_user_alertable(&(ccip_dev_fme_pwraal_dev(pfme_dev)->m_aaldev->m_sem)) ) ) {
      PDEBUG("kosal_sem_get_user_alertable interrupted \n");
      res =-EINVAL;
      return res;
   }

   // Send PR Power Request to owner application
   if ( !kosal_list_is_empty(&(ccip_dev_fme_pwraal_dev(pfme_dev)->m_aaldev->m_ownerlist)) )  {

      // Loop through the list looking for a match
      kosal_list_for_each_safe(pitr, temp, &(pfme_dev->m_power_aaldev->m_aaldev->m_ownerlist)) {

         // finds Power device owner
         pOwner = kosal_container_of(pitr, struct aaldev_owner, m_ownerlist);
         PVERBOSE(  "Power Owner pid= %d \n" , pOwner->m_pid);

         pfme_dev->m_pr_program_context->m_pwrReqTranID.m_intID= 299;
         // Sends Power Request to application
         if(0 != sim){
            pafuws_evt =ccipdrv_event_afu_aysnc_reconf_pwr_create( uid_afurespPwrMgrResponce,
                                                                   pOwner->m_sess.m_device,
                                                                   pOwner->m_sess.m_ownerContext,
                                                                   1,
                                                                   1,
                                                                   1,
                                                                   1,
                                                                   25,
                                                                   pfme_dev->m_pr_program_context->m_pwrReqTranID,
                                                                   0);

         } else {

             busID        = ccip_dev_fme_pwraal_dev(pfme_dev)->m_pcidev->bus->number;
             deviceID     = PCI_SLOT(ccip_dev_fme_pwraal_dev(pfme_dev)->m_pcidev->devfn);
             functionID   = PCI_FUNC(ccip_dev_fme_pwraal_dev(pfme_dev)->m_pcidev->devfn);
             socketID     = pfme_dev->m_pHDR->fab_capability.socket_id;

             PDEBUG("busID %x\n", busID);
             PDEBUG("deviceID %x\n", deviceID);
             PDEBUG("functionID %x\n", functionID);
             PDEBUG("socketID %x\n", socketID);
             PDEBUG("power_required %d\n", power_required);

             // get Power from GB
             // Get Total Power  =  BB + SUM GB ?
            pafuws_evt = ccipdrv_event_afu_aysnc_reconf_pwr_create( uid_afurespPwrMgrResponce,
                                                                   pOwner->m_sess.m_device,
                                                                   pOwner->m_sess.m_ownerContext,
                                                                   socketID,
                                                                   busID,
                                                                   deviceID,
                                                                   functionID,
                                                                   power_required, // TDB
                                                                   pfme_dev->m_pr_program_context->m_pwrReqTranID,
                                                                   0);

         }

         ccidrv_sendevent(&(pOwner->m_sess),
                          AALQIP(pafuws_evt));

      } // end list

   }// endif

    kosal_sem_put(&pfme_dev->m_power_aaldev->m_aaldev->m_sem);

   PTRACEOUT_INT(res);
   return  res;
}
