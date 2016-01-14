//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
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
/// @file ccip_pr.c
/// @brief  Definitions for ccip Partial Reconfiguration AAL device.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_port_mmio.c
//     CREATED: Nov 11, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the implementation of the CCIP PR
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

extern struct cci_aal_device   *
                       cci_create_AAL_UAFU_Device( struct port_device  *,
                                                   btPhysAddr,
                                                   struct CCIP_AFU_Header *,
                                                   struct aal_device_id *);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////           AAL SUPPORT FUNCTIONS          ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int CommandHandler( struct aaldev_ownerSession *,
                           struct aal_pipmessage*);
static int cci_mmap(struct aaldev_ownerSession *pownerSess,
                           struct aal_wsid *wsidp,
                           btAny os_specific);

///=============================================================================
/// Name: cci_PRpip
/// @brief Physical Interface Protocol Interface for the PR AFU
///              kernel based AFU engine.
///=============================================================================
struct aal_ipip cci_PRpip = {
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


//=============================================================================
// Name: program_afu
// Description:
// Interface: private
// Returns: N/A
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================

inline void GetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *pcsrval){
   bt32bitCSR *p32 = (bt32bitCSR *)ptr;

   *pcsrval = *p32;
}

inline void SetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *csrval)
{
   bt32bitCSR *p32 = (bt32bitCSR *)ptr;

   *p32 = *csrval;
}

//void program_afu( pwork_object pwork)
int program_afu( struct cci_aal_device *pdev,  btVirtAddr kptr, btWSSize len )
{

#if 0
   struct cci_aal_device *pdev = kosal_get_object_containing( pwork,
                                                              struct cci_aal_device,
                                                              task_handler );
#endif
   struct fme_device *pfme_dev = cci_dev_pfme(pdev);
   struct CCIP_FME_DFL_PR     *pr_dev = ccip_fme_pr(pfme_dev);
   bt32bitCSR csr = 0;
   btBool bPR_Ready = 0;

   PVERBOSE("kptr =%lx", kptr);
   PVERBOSE("len =%d", len);

   // Program the AFU
   // For BDX-P only FME initiated PR is supported. So, CSR_FME_PR_CONTROL[0] = 0
   // ---------------------------------------------------------------------------


  csr=0;
  // WILL THIS WORK??
  // pr_dev->ccip_fme_pr_control.pr_port_access = 1;

  GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);

  csr &=0xFFFFFFFE;
  PVERBOSE("Setting up PR access mode to FME initiated PR");
  SetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);


  // Step 1 - Check FME_PR_STATUS[27:24] == 4'h0
  // -------------------------------------------
  // Both Initially as well as after PR, HW updates PR status to this state
  {


     PVERBOSE("Waiting for PR Resource in HW to be initialized and ready");
     do {
        csr=0;
        GetCSR(&pr_dev->ccip_fme_pr_status.csr,&csr);
        bPR_Ready = ( (csr>>24 & 0xF) == 0x0 );
     }
     while(!bPR_Ready);
     PVERBOSE("HW is ready for PR");
  }


  // Step 2 - Check FME_PR_STATUS[16] for any previous PR errors
  // --------------------------------------------------------------
  // FME_PR_STATUS[16] is PR PASS or FAIL bit. Different from SAS.
  // FME_PR_STATUS[16] is RO from SW point of view.
  // TODO : Update SAS
  {
     bt32bitCSR PR_Timeout_err = 0;
     bt32bitCSR PR_FIFO_err = 0;
     bt32bitCSR PR_IP_err = 0;
     bt32bitCSR PR_bitstream_err = 0;
     bt32bitCSR PR_CRC_err = 0;
     bt32bitCSR PR_clear_status_err = 0;

     csr=0;
     PVERBOSE("Checking for errors in previous PR operation");
     GetCSR(&pr_dev->ccip_fme_pr_status.csr,&csr);

     // Step 3 - Clear FME_PR_ERROR[5:0] - if needed based on Step - 2
     // -----------------------------------------------------------------
     // Upon failure, FME_PR_ERROR[5:0] gives additional error info.
     // FME_PR_ERROR[5:0] - All 6 bits are RW1CS - HW writes 1 upon error to set the bit. SW writes 1 to clear the bit
     // Once All error bits set in FME_PR_ERROR[5:0] are cleared by SW, HW will clear FME_PR_STATUS[16]
     // TODO: This step is different from SAS flow. Update SAS
     // NOTE: Error Logging and propagation might change based on SKX RAS

     if ((csr>>16 & 0x1) == 0x1){
        csr=0;
        PVERBOSE("Errors found: Collecting error info ...");
        GetCSR(&pr_dev->ccip_fme_pr_err.csr,&csr);
        PR_Timeout_err = csr;
        PR_FIFO_err = csr >> 1;
        PR_IP_err =  csr >> 2;
        PR_bitstream_err = csr >> 3;
        PR_CRC_err = csr >> 4;
        PR_clear_status_err = csr >> 5;

        if((PR_Timeout_err & 0x1) == 0x1) PVERBOSE("\tPR Timeout Error Detected");
        if((PR_FIFO_err & 0x1) == 0x1)    PVERBOSE("\tPR FIFO Error Detected");
        if((PR_IP_err & 0x1) == 0x1)   PVERBOSE("\tPR IP Error Detected");
        if((PR_bitstream_err & 0x1) == 0x1) PVERBOSE("\tPR incompatible bitstream Error Detected");
        if((PR_CRC_err & 0x1) == 0x1) PVERBOSE("\tPR CRC Error Detected");
        if((PR_clear_status_err & 0x1) == 0x1)  PVERBOSE("\tPrevious PR clean-up Error Detected");

        // Clearing all Errors
        csr |= 0x0000003F;
        SetCSR(&pr_dev->ccip_fme_pr_err.csr,&csr);
        PVERBOSE("Previous PR errors cleared");
     }else{
        PVERBOSE("No previous PR errors Detected");
     }

     // Step 4 - Write PR Region ID to FME_PR_CONTROL[9:8]
     // --------------------------------------------------
     // NOTE: For BDX-P only 1 AFU is supported in HW. So, CSR_FME_PR_CONTROL[9:8] should always be 0
     // This will change for SKX

     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);
     csr &=0xFFFFFCFF;
     PVERBOSE("Writing PR region ID = 0");
     SetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);

     // TODO: SW Driver is expected to hold the Port undergoing PR in Reset before initiating PR.
     // SW has to bring back the port out of reset after successful PR and discover the AFU behind that PORT
     // Ignored here

     // Step 5 - Initiate PR - Write 1 to FME_PR_CONTROL[12]
     // ---------------------------------------------------
     // SW can only initiate PR. HW will auto clear this bit upon failure or success or timeout
	 PVERBOSE("Initiate PR");
     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);
     csr |=0x00001000;
    
     SetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);


     // Sanity check - Make sure that PR request from SW has reached HW
     // ---------------------------------------------------------------
     // Strictly speaking, the following step is neither required nor has any impact from HW point of view.
     // But if no SW requests are reaching the HW, there is no way for the HW to notify the SW and might result in false PR PASS info in SW.
     // So, Read the PR Start bit (FME_PR_CONTROL[12]) after writing to it and make sure that it is set.
     // Once this bit is set, HW will make sure that it returns PR PASS / PR FAIL / PR timeout and notify the SW

     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);
     if ((csr >> 12 & 0x1) != 0x1)
     {
        PVERBOSE("PR requests from SW did not reach HW. Try again..");
        return 1;
     }

     // Step 6 - Check available credits: FME_PR_STATUS[8:0] for PR data push and push Data to FME_PR_DATA[31:0]
     // -------------------------------------------------------------------------------------------------------
     // For instance,
     // if FME_PR_STATUS[8:0] read yields 511, SW can perform 511 32-bit writes from rbf file to FME_PR_DATA[31:0] and check credits again
     {
        bt32bitCSR PR_FIFO_credits = 0;
        uint32_t  *byteRead = (uint32_t  *)kptr;
        csr=0;
        GetCSR(&pr_dev->ccip_fme_pr_status.csr, &csr);
        PR_FIFO_credits = csr & 0x000001FF;

     //   PVERBOSE("Pushing Data from rbf to HW \n");


        while(len >0) {
             if (PR_FIFO_credits <= 1)
             {
               do {
               csr = 0;
               PR_FIFO_credits = 0;
               GetCSR(&pr_dev->ccip_fme_pr_status.csr, &csr);
               PR_FIFO_credits = csr & 0x000001FF;
               }
               while (PR_FIFO_credits <=1);
             }

             SetCSR(&pr_dev->ccip_fme_pr_data.csr, byteRead);
             PR_FIFO_credits --;
             byteRead++;
			 len = len - 4;
        }
     }
     // Step 7 - Notify the HW that bitstream push is complete
     // ------------------------------------------------------
     // Write 1 to CSR_FME_PR_CONTROL[13]. This bit is RW1S. SW writes 1 to set this bit.
     // Hardware will auto clear this bit upon PR completion
     // TODO: This step is currently not defined in SAS. Update SAS

     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);
     csr |=0x00002000;
     SetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);

     PVERBOSE("Green bitstream push complete \n");

     // Step 8 - Once all data is pushed from rbf file, wait for PR completion
     // ----------------------------------------------------------------------
     // Check FME_PR_CONTROL[12] == 0.
     // Note: PR status bits are valid only when FME_PR_CONTROL[12] == 0.
     // FME_PR_CONTROL[12] is an atomic status check bit for initiating PR and also checking for PR completion
     // This bit set to 0 essentially means that HW has released the PR resource either due to PR PASS or PR FAIL.

     PVERBOSE("Waiting for HW to release PR resource");
     kosal_mdelay(10);
     bPR_Ready = 0;
     do {
     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_control.csr, &csr);
     bPR_Ready = ( (csr>>12 & 0x1) == 0x0);
     }
     while(!bPR_Ready);
     PVERBOSE("PR operation complete, checking Status ...");

     // Step 9 - Check PR PASS / FAIL
     // -----------------------------
     // Read the FME_PR_STATUS[16] to check PR success / fail
     // FME_PR_STATUS[16] = 0 implies PR Passed
     // FME_PR_STATUS[16] = 1 implies PR Failed. Read FME_PR_ERROR for more info upon failure
     // TODO: This step is different from SAS. Update SAS
     // NOTE: Error Register updating/ clearing may change based on SKX RAS requirement

     csr=0;
     GetCSR(&pr_dev->ccip_fme_pr_status.csr, &csr);

     if (csr >> 16)
     {
        PVERBOSE("PR FAILED with following Errors:");
        csr=0;
        GetCSR(&pr_dev->ccip_fme_pr_err.csr,&csr);
        PR_Timeout_err = csr;
        PR_FIFO_err = csr >> 1;
        PR_IP_err =  csr >> 2;
        PR_bitstream_err = csr >> 3;
        PR_CRC_err = csr >> 4;
        PR_clear_status_err = csr >> 5;

        if((PR_Timeout_err & 0x1) == 0x1) PVERBOSE("\tPR Timeout Error Detected");
        if((PR_FIFO_err & 0x1) == 0x1) PVERBOSE("\tPR FIFO Error Detected");
        if((PR_IP_err & 0x1) == 0x1) PVERBOSE("\tPR Engine Error Detected");
        if((PR_bitstream_err & 0x1) == 0x1) PVERBOSE("\tPR incompatible bitstream Error Detected");
        if((PR_CRC_err & 0x1) == 0x1) PVERBOSE("\tPR CRC Error Detected");
        if((PR_clear_status_err & 0x1) == 0x1) PVERBOSE("\tPR Engine clean-up Error Detected");
     }

     else
     {
        PVERBOSE("PR PASSED\n");
     }

  }
  return 0;
}

///============================================================================
/// Name: cci_create_AAL_PR_Device
/// @brief Creates and registers PR objects (resources) we want to
///        expose through AAL.
///
/// @param[in] pportdev - Port device
/// @param[in] paalid - Base AAL ID for this device.
/// @return    AAL Device pointer
///============================================================================
struct cci_aal_device   *
               cci_create_AAL_PR_Device( struct port_device  *pportdev,
                                          struct aal_device_id *paalid)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   int ret;

   PTRACEIN;

   PVERBOSE("Creating Signal Tap device\n");

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   ASSERT(NULL != pcci_aaldev);
   if( NULL == pcci_aaldev){
      return NULL;
   }

   // Make it a User AFU
   cci_dev_type(pcci_aaldev) = cci_dev_PR;

   // Record parentage
   cci_dev_pport(pcci_aaldev)    = pportdev;       // Save its port
   cci_dev_pfme(pcci_aaldev)     = ccip_port_dev_fme(pportdev);

   // Device Address is the same as the Port. Set the AFU ID information
   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(*paalid)            = CCIP_PR_GUIDL;
   aaldevid_afuguidh(*paalid)            = CCIP_PR_GUIDH;
   aaldevid_pipguid(*paalid)             = CCIP_PR_PIPIID;

   // Setup the MMIO region parameters
   cci_dev_kvp_afu_mmio(pcci_aaldev)   = (btVirtAddr)ccip_port_pr(pportdev);
   cci_dev_len_afu_mmio(pcci_aaldev)   = sizeof(struct CCIP_PORT_DFL_PR);
   cci_dev_phys_afu_mmio(pcci_aaldev)  = kosal_virt_to_phys((btVirtAddr)ccip_port_pr(pportdev));

   // Create the AAL device and attach it to the CCI device object
   cci_aaldev_to_aaldev(pcci_aaldev) =  aaldev_create( "CCIPPR",           // AAL device base name
                                                       paalid,             // AAL ID
                                                       &cci_PRpip);

   // Set up reverse look up. Use aaldev_to_cci_aal_device() to access
   aaldev_context(cci_aaldev_to_aaldev(pcci_aaldev)) = pcci_aaldev;

   //===========================================================
   // Set up the optional aal_device attributes
   //
#if 0
   // Initialize the worker thread
   cci_dev_workq(pcci_aaldev) = kosal_create_workqueue( cci_aaldev_to_aaldev(pcci_aaldev) );


   KOSAL_INIT_WORK(cci_dev_task_handler(pcci_aaldev),task_poller);
#endif
   // Set how many owners are allowed access to this device simultaneously
   cci_aaldev_to_aaldev(pcci_aaldev)->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;

   if( cci_dev_allow_map_mmior_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

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
      PERR("Failed to initialize AAL Device for CCIPPR[%d:%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                     aaldevid_devaddr_devnum(*paalid),
                                                                     aaldevid_devaddr_fcnnum(*paalid),
                                                                     aaldevid_devaddr_subdevnum(*paalid));
      cci_destroy_aal_device(pcci_aaldev);
      return NULL;
   }
   PTRACEOUT;
   return pcci_aaldev;
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
   btWSSize         respBufSize     = Message->m_respbufSize;

   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error = uid_errnumInvalidRequest;

   PVERBOSE("In CCI Command handler, AFUCommand().\n");

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

      struct ccipdrv_event_afu_response_event *pafuws_evt       = NULL;
      AFU_COMMAND_CASE(ccipdrv_deactivateAFU) {

         // Port for this AAL PR object
         struct port_device  *pportdev = cci_dev_pport(pdev);

         // Find the AFU device associated with this port
         if(NULL == ccip_port_uafu_dev(pportdev)){
            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespDeactivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumNoAFU);

            ccidrv_sendevent(pownerSess->m_UIHandle,
                             pownerSess->m_device,
                             AALQIP(pafuws_evt),
                             Message->m_context);

            goto ERROR;
         }

         // Make sure device is not in use. If it is notify user and start time out timer.
         if(NULL == ccip_port_uafu_dev(pportdev)){
            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespDeactivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumNoAFU);

            ccidrv_sendevent(pownerSess->m_UIHandle,
                             pownerSess->m_device,
                             AALQIP(pafuws_evt),
                             Message->m_context);

            goto ERROR;

         }
         // TODO FOR NOW JUST DO IT
         cci_unpublish_aaldevice(ccip_port_uafu_dev(pportdev));
         cci_destroy_aal_device( ccip_port_uafu_dev(pportdev) );
         ccip_port_uafu_dev(pportdev) = NULL;
         pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespDeactivateComplete,
                                                                  pownerSess->m_device,
                                                                  &Message->m_tranID,
                                                                  Message->m_context,
                                                                  uid_errnumOK);

         ccidrv_sendevent(pownerSess->m_UIHandle,
                          pownerSess->m_device,
                          AALQIP(pafuws_evt),
                          Message->m_context);



      } break;

      AFU_COMMAND_CASE(ccipdrv_activateAFU) {

         // Port for this AAL PR object
         struct port_device  *pportdev = cci_dev_pport(pdev);

         // Find the AFU device associated with this port
         if(NULL != ccip_port_uafu_dev(pportdev)){
            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespActivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumAFUActivated);

            ccidrv_sendevent(pownerSess->m_UIHandle,
                             pownerSess->m_device,
                             AALQIP(pafuws_evt),
                             Message->m_context);

            goto ERROR;
         }

         // TODO FOR NOW JUST DO IT
         {
              // Get the AFU header pointer by adding the offset to the port header address
              struct CCIP_AFU_Header        *pafu_hdr = (struct CCIP_AFU_Header *)(((btVirtAddr)ccip_port_hdr(pportdev) ) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset);
              btPhysAddr                     pafu_phys = ccip_port_phys_mmio(pportdev) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset;
              struct cci_aal_device         *pcci_aaldev = NULL;
              struct aal_device_id           aalid;
              struct aal_device             *paaldevice = NULL;

              // Get the address of the PR. User AFU instance number always follows PR.
              paaldevice = cci_aaldev_to_aaldev(pdev);
              aalid = aaldev_devid(paaldevice);


              // If the device is present
              if(~0ULL != pafu_hdr->ccip_dfh.csr){

                 // Instantiate it
                 aaldevid_devaddr_instanceNum(aalid)++;
                 pcci_aaldev = cci_create_AAL_UAFU_Device(  pportdev,
                                                            pafu_phys,
                                                            pafu_hdr,
                                                           &aalid);
                 ASSERT(NULL != pcci_aaldev);

                 if(NULL == pcci_aaldev){
                    PDEBUG("ERROR: Creating User AFU device\n");
                    return false;     // TODO This is a BUG if we get here but should cleanup correctly.
                 }

                 // Add the device to the CCI Board device's device list
                 kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

              } // End if(~0ULL == pafu_hdr->ccip_dfh.csr){
           } //End block




         pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespActivateComplete,
                                                                  pownerSess->m_device,
                                                                  &Message->m_tranID,
                                                                  Message->m_context,
                                                                  uid_errnumOK);

         ccidrv_sendevent(pownerSess->m_UIHandle,
                          pownerSess->m_device,
                          AALQIP(pafuws_evt),
                          Message->m_context);



      } break;

      AFU_COMMAND_CASE(ccipdrv_configureAFU) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
         btWSSize buflen = preq->ahmreq.u.mem_uv2id.size;
         btVirtAddr uptr = preq->ahmreq.u.mem_uv2id.vaddr;
         btVirtAddr kptr = NULL;

         // Get a copy of the bitfile image from user space.
         //  This function returns a safe pointer to the user data.
         //  This may involve copying into kernel space.

         if((NULL == uptr) || (0 == buflen)){
            PERR("AFU reprogramming failed\n");
            pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumBadParameter);
         }else{
            kptr = kosal_get_user_buffer(uptr, buflen);
            if(NULL == kptr ){
               PERR("kosal_get_user_buffer returned NULL");
               pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                                pownerSess->m_device,
                                                                &Message->m_tranID,
                                                                Message->m_context,
                                                                uid_errnumBadParameter);
            }else{
/* Test Code
               printk (KERN_INFO DRV_NAME "[%d]%s\n",buflen,kptr);
            }

            pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumOK);
*/

            // Program the afu  TODO   kosal_queue_delayed_work(cci_dev_workq(pdev), cci_dev_task_handler(pdev), 0);
               if(0 != program_afu(pdev,  kptr, buflen )){
                  PERR("AFU reprogramming failed\n");
                  pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                                   pownerSess->m_device,
                                                                   &Message->m_tranID,
                                                                   Message->m_context,
                                                                   uid_errnumNoAFU);
               }else {
                  pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                                   pownerSess->m_device,
                                                                   &Message->m_tranID,
                                                                   Message->m_context,
                                                                   uid_errnumOK);
               }
               kosal_free_user_buffer(kptr, buflen);
            }
         }
         ccidrv_sendevent(pownerSess->m_UIHandle,
                          pownerSess->m_device,
                          AALQIP(pafuws_evt),
                          Message->m_context);
      }break;

      // Returns a workspace ID for the Config Space

      AFU_COMMAND_CASE(ccipdrv_getMMIORmap) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
         struct aalui_WSMEvent WSID;
         struct aal_wsid   *wsidp            = NULL;

         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Failed ccipdrv_getMMIOR map Permission\n");
            PERR("Direct API access not permitted on this device\n");
            Message->m_errcode = uid_errnumPermission;
            break;
         }

         //------------------------------------------------------------
         // Create the WSID object and add to the list for this session
         //------------------------------------------------------------
         if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
            PERR("Failed ccipdrv_getMMIOR map Parameter\n");

            PERR("Bad WSID on ccipdrv_getMMIORmap\n");
            Message->m_errcode = uid_errnumBadParameter;
            break;
         }

         wsidp = ccidrv_getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
         if ( NULL == wsidp ) {
            PERR("Could not allocate workspace\n");
            retval = -ENOMEM;
            /* generate a failure event back to the caller? */
            goto ERROR;
         }

         wsidp->m_type = WSM_TYPE_MMIO;
         PDEBUG("Getting CSR %s Aperature WSID %p using id %llx .\n",
                   ((WSID_CSRMAP_WRITEAREA == preq->ahmreq.u.wksp.m_wsid) ? "Write" : "Read"),
                   wsidp,
                   preq->ahmreq.u.wksp.m_wsid);

         PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_afu_mmio(pdev), (int)cci_dev_len_afu_mmio(pdev));

         // Set up the return payload
         WSID.evtID           = uid_wseventMMIOMap;
         WSID.wsParms.wsid    = pwsid_to_wsidhandle(wsidp);
         WSID.wsParms.physptr = cci_dev_phys_afu_mmio(pdev);
         WSID.wsParms.size    = cci_dev_len_afu_mmio(pdev);

         // Make this atomic. Check the original response buffer size for room
         if(respBufSize >= sizeof(struct aalui_WSMEvent)){
            *((struct aalui_WSMEvent*)Message->m_response) = WSID;
            Message->m_respbufSize = sizeof(struct aalui_WSMEvent);
         }
         PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
         Message->m_errcode = uid_errnumOK;

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

      } break;

      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_ALLOC)
      {
         struct ccidrvreq    *preq        = (struct ccidrvreq *)pmsg->payload;
         btVirtAddr           krnl_virt   = NULL;
         struct aal_wsid     *wsidp       = NULL;

         // Normal flow -- create the needed workspace.
         krnl_virt = (btVirtAddr)kosal_alloc_contiguous_mem_nocache(preq->ahmreq.u.wksp.m_size);
         if (NULL == krnl_virt) {
            pafuws_evt = ccipdrv_event_afu_afuallocws_create(pownerSess->m_device,
                                                           (btWSID) 0,
                                                           NULL,
                                                           (btPhysAddr)NULL,
                                                           preq->ahmreq.u.wksp.m_size,
                                                           Message->m_tranID,
                                                           Message->m_context,
                                                           uid_errnumNoMem);

            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message->m_context);

            goto ERROR;
         }

         //------------------------------------------------------------
         // Create the WSID object and add to the list for this session
         //------------------------------------------------------------

         wsidp = ccidrv_getwsid(pownerSess->m_device, (btWSID)krnl_virt);
         if ( NULL == wsidp ) {
            PERR("Couldn't allocate task workspace\n");
            retval = -ENOMEM;
            /* send a failure event back to the caller? */
            goto ERROR;
         }

         wsidp->m_size = preq->ahmreq.u.wksp.m_size;
         wsidp->m_type = WSM_TYPE_VIRTUAL;
         PDEBUG("Creating Physical WSID %p.\n", wsidp);

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

         PINFO("CCI WS alloc wsid=0x%" PRIx64 " phys=0x%" PRIxPHYS_ADDR  " kvp=0x%" PRIx64 " size=%" PRIu64 " success!\n",
                  preq->ahmreq.u.wksp.m_wsid,
                  kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                  wsidp->m_id,
                  wsidp->m_size);

         // Create the event
         pafuws_evt = ccipdrv_event_afu_afuallocws_create(
                                               aalsess_aaldevicep(pownerSess),
                                               pwsid_to_wsidhandle(wsidp), // make the wsid appear page aligned for mmap
                                               NULL,
                                               kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                                               preq->ahmreq.u.wksp.m_size,
                                               Message->m_tranID,
                                               Message->m_context,
                                               uid_errnumOK);

         PVERBOSE("Sending the WKSP Alloc event.\n");
         // Send the event
         ccidrv_sendevent(aalsess_uiHandle(pownerSess),
                                        aalsess_aaldevicep(pownerSess),
                                        AALQIP(pafuws_evt),
                                        Message->m_context);

      } break; // case fappip_afucmdWKSP_VALLOC


      //============================
      //  Free Workspace
      //============================
      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_FREE) {
         struct ccidrvreq    *preq        = (struct ccidrvreq *)pmsg->payload;
         btVirtAddr           krnl_virt   = NULL;
         struct aal_wsid     *wsidp       = NULL;

         ASSERT(0 != preq->ahmreq.u.wksp.m_wsid);
         if ( 0 == preq->ahmreq.u.wksp.m_wsid ) {
            PDEBUG("WKSP_IOC_FREE: WS id can't be 0.\n");
            // Create the exception event
            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message->m_tranID,
                                                           Message->m_context,
                                                           uid_errnumBadParameter);

            // Send the event
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message->m_context);
            retval = -EFAULT;
            goto ERROR;
         }

         // Get the workspace ID object
         wsidp = ccidrv_valwsid(preq->ahmreq.u.wksp.m_wsid);

         ASSERT(wsidp);
         if ( NULL == wsidp ) {
            // Create the exception event
            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message->m_tranID,
                                                           Message->m_context,
                                                           uid_errnumBadParameter);

            PDEBUG("Sending WKSP_FREE Exception\n");
            // Send the event
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message->m_context);

            retval = -EFAULT;
            goto ERROR;
         }

         // Free the buffer
         if(  WSM_TYPE_VIRTUAL != wsidp->m_type ) {
            PDEBUG( "Workspace free failed due to bad WS type. Should be %d but received %d\n",WSM_TYPE_VIRTUAL,
                  wsidp->m_type);

            pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                           Message->m_tranID,
                                                           Message->m_context,
                                                           uid_errnumBadParameter);
            ccidrv_sendevent(pownerSess->m_UIHandle,
                                           pownerSess->m_device,
                                           AALQIP(pafuws_evt),
                                           Message->m_context);

            retval = -EFAULT;
            goto ERROR;
         }

         krnl_virt = (btVirtAddr)wsidp->m_id;

         kosal_free_contiguous_mem(krnl_virt, wsidp->m_size);

         // remove the wsid from the device and destroy
         list_del_init(&wsidp->m_list);
         ccidrv_freewsid(wsidp);

         // Create the  event
         pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message->m_tranID,
                                                        Message->m_context,
                                                        uid_errnumOK);

         PVERBOSE("Sending the WKSP Free event.\n");
         // Send the event
         ccidrv_sendevent(pownerSess->m_UIHandle,
                                        pownerSess->m_device,
                                        AALQIP(pafuws_evt),
                                        Message->m_context);
      } break; // case fappip_afucmdWKSP_FREE

      default: {
         struct ccipdrv_event_afu_response_event *pafuresponse_evt = NULL;

         PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);

         pafuresponse_evt = ccipdrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     request_error);

        ccidrv_sendevent( pownerSess->m_UIHandle,
                          pownerSess->m_device,
                          AALQIP(pafuresponse_evt),
                          Message->m_context);

         retval = -EINVAL;
      } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

ERROR:
   return retval;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             CCI SIM PIP MMAP             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: csr_vmaopen
// Description: Called when the vma is mapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaopen(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA OPEN.\n" );
}
#endif


//=============================================================================
// Name: wksp_vmaclose
// Description: called when vma is unmapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaclose(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA CLOSE.\n" );
}
#endif

#ifdef NOT_USED
static struct vm_operations_struct csr_vma_ops =
{
   .open    = csr_vmaopen,
   .close   = csr_vmaclose,
};
#endif


//=============================================================================
// Name: cci_mmap
// Description: Method used for mapping kernel memory to user space. Called by
//              uidrv.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: This method front ends all operations that require mapping shared
//           memory. It examines the wsid to determine the appropriate service
//           to perform the map operation.
//=============================================================================
int
cci_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific)
{

   struct vm_area_struct     *pvma = (struct vm_area_struct *) os_specific;

   struct cci_PIPsession   *pSess = NULL;
   struct cci_aal_device       *pdev = NULL;
   unsigned long              max_length = 0; // mmap length requested by user
   int                        res = -EINVAL;

   ASSERT(pownerSess);
   ASSERT(wsidp);

   // Get the spl2 aal_device and the memory manager session
   pSess = (struct cci_PIPsession *) aalsess_pipHandle(pownerSess);
   ASSERT(pSess);
   if ( NULL == pSess ) {
      PDEBUG("CCIV4 Simulator mmap: no Session");
      goto ERROR;
   }

   pdev = cci_PIPsessionp_to_ccidev(pSess);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PDEBUG("CCIV4 Simulator mmap: no device");
      goto ERROR;
   }

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   pvma->vm_ops = NULL;

   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if ( WSM_TYPE_MMIO == wsidp->m_type ) {
      void *ptr;
      size_t size;
      switch ( wsidp->m_id )
      {
            case WSID_CSRMAP_WRITEAREA:
            case WSID_CSRMAP_READAREA:
            case WSID_MAP_MMIOR:
            case WSID_MAP_UMSG:
            break;
         default:
            PERR("Attempt to map invalid WSID type %d\n", (int) wsidp->m_id);
            goto ERROR;
      }

      // Verify that we can fulfill the request - we set flags at create time.
      if ( WSID_CSRMAP_WRITEAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_write_space(pdev));

         if ( !cci_dev_allow_map_csr_write_space(pdev) ) {
            PERR("Denying request to map CSR Write space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_CSRMAP_READAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_read_space(pdev));

         if ( !cci_dev_allow_map_csr_read_space(pdev) ) {
            PERR("Denying request to map CSR Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_MAP_MMIOR == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_mmior_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_mmio(pdev);
         size = cci_dev_len_afu_mmio(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,               // Virtual Memory Area
            pvma->vm_start,                        // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT,   // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped MMR region.
         return 0;
      }

      if ( WSID_MAP_UMSG == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_umsg_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_umsg_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_umsg(pdev);
         size = cci_dev_len_afu_umsg(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,                             // Virtual Memory Area
            pvma->vm_start,                   // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped UMSG region.
         return 0;
      }

      // TO REST OF CHECKS

      // Map the PCIe BAR as the CSR region.
      ptr = (void *) cci_dev_phys_afu_mmio(pdev);
      size = cci_dev_len_afu_mmio(pdev);

      PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
         ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
         ptr,
         size,
         (void *)pvma->vm_start);

      // Map the region to user VM
      res = remap_pfn_range(pvma,                             // Virtual Memory Area
         pvma->vm_start,                   // Start address of virtual mapping
         ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
         size,
         pvma->vm_page_prot);

      if ( unlikely(0 != res) ) {
         PERR("remap_pfn_range error at CSR mmap %d\n", res);
         goto ERROR;
      }

      // Successfully mapped CSR region.
      return 0;
   }

   //------------------------
   // Map normal workspace
   //------------------------

   max_length = min(wsidp->m_size, (btWSSize)(pvma->vm_end - pvma->vm_start));

   PVERBOSE( "MMAP: start 0x%lx, end 0x%lx, KVP 0x%p, size=%" PRIu64 " 0x%" PRIx64 " max_length=%ld flags=0x%lx\n",
      pvma->vm_start, pvma->vm_end, (btVirtAddr)wsidp->m_id, wsidp->m_size, wsidp->m_size, max_length, pvma->vm_flags);

   res = remap_pfn_range(pvma,                              // Virtual Memory Area
      pvma->vm_start,                    // Start address of virtual mapping, from OS
      (kosal_virt_to_phys((btVirtAddr) wsidp->m_id) >> PAGE_SHIFT),   // physical memory backing store in pfn
      max_length,                        // size in bytes
      pvma->vm_page_prot);               // provided by OS
   if ( unlikely(0 != res) ) {
      PERR("remap_pfn_range error at workspace mmap %d\n", res);
      goto ERROR;
   }

   ERROR:
   return res;
}

