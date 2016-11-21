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
#include "ccip_pr.h"
#include "ccip_pwr.h"

#include "cci_pcie_driver_PIPsession.h"

#define BS_WRITE_CSR_LEN  4

extern btUnsigned32bitInt sim;
extern kosal_list_head g_device_list;
extern kosal_semaphore g_dev_list_sem;


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
extern int cci_mmap( struct aaldev_ownerSession *pownerSess,
                     struct aal_wsid *wsidp,
                     btAny os_specific );



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




inline void GetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *pcsrval){

   volatile bt32bitCSR *p32 = (bt32bitCSR *)ptr;
   *pcsrval = *p32;
}

inline void SetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *csrval) {

   volatile bt32bitCSR *p32 = (bt32bitCSR *)ptr;
   *p32 = *csrval;
}

inline void Get64CSR(btUnsigned64bitInt *ptr, bt64bitCSR *pcsrval){

   volatile bt64bitCSR *p64 = (bt64bitCSR *)ptr;
   *pcsrval = *p64;
}

inline void Set64CSR(btUnsigned64bitInt *ptr, bt64bitCSR *csrval)
{
   volatile bt64bitCSR *p64 = (bt64bitCSR *)ptr;
   *p64 = *csrval;
}
//=============================================================================
// Name: program_afu
// Description:
// Interface: private
// Returns: N/A
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
int program_afu( struct cci_aal_device *pdev,  btVirtAddr kptr, btWSSize len )
{

#if 0
   struct cci_aal_device *pdev = kosal_get_object_containing( pwork,
                                                              struct cci_aal_device,
                                                              task_handler );
#endif
   struct fme_device *pfme_dev = cci_aaldev_pfme(pdev);
   struct CCIP_FME_DFL_PR     *pr_dev = ccip_fme_pr(pfme_dev);
   bt32bitCSR csr = 0;
   btBool bPR_Ready = 0;

   PDEBUG("kptr =%p", kptr);
   PDEBUG("len =%d\n", (unsigned)len);

   // Don't do anything under simulation
   if(0 != sim){
      PDEBUG("Simulated reprogram\n");
      return 0;
   }

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
     PDEBUG("Initiate PR");
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
        btUnsigned32bitInt  *byteRead = (btUnsigned32bitInt  *)kptr;
        csr=0;
        GetCSR(&pr_dev->ccip_fme_pr_status.csr, &csr);
        PR_FIFO_credits = csr & 0x000001FF;

        PDEBUG("Pushing Data from rbf to HW \n");


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
             len -= 4;
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
     kosal_mdelay(10);        // Workaround for potential HW timing issue
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
/// Name: ccipdrv_event_afu_aysnc_pr_release_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
///============================================================================
void ccipdrv_event_afu_aysnc_pr_release_send(struct pr_program_context *ppr_program_ctx,
                                             uid_errnum_e  eno)
{
   struct ccipdrv_event_afu_response_event *pafuws_evt      = NULL;
   PTRACEIN;
   // return if PR program context is NULL
   ASSERT(NULL != ppr_program_ctx);
   if(NULL == ppr_program_ctx) {
      return ;
   }

#ifdef PWRMGR
   //PR failed, Movning Idle cores to online
   if ((ppr_program_ctx->m_cmd == ccipdrv_configureAFU) &&
      (uid_errnumOK != eno)) {
      send_pr_power_event(ppr_program_ctx, 0);
   }
#endif

   pafuws_evt = ccipdrv_event_afu_aysnc_pr_release_create( ppr_program_ctx->m_respID,
                                                           ppr_program_ctx->m_pownerSess->m_device,
                                                           ppr_program_ctx->m_pownerSess->m_ownerContext,
                                                           eno);

   ccidrv_sendevent(ppr_program_ctx->m_pownerSess,
                    AALQIP(pafuws_evt));

 
   if((ppr_program_ctx->m_cmd == ccipdrv_configureAFU) &&
        (NULL != ppr_program_ctx->m_kbufferptr)) {
        kosal_free_user_buffer(ppr_program_ctx->m_kbufferptr, ppr_program_ctx->m_bufferlen);
        ppr_program_ctx->m_kbufferptr = NULL;
     }


   kosal_sem_put(cci_dev_pr_sem(ppr_program_ctx->m_pPR_dev));
   PDEBUG("UN-LOCK RECONF \n");

   if(NULL != ppr_program_ctx){
      PDEBUG(" Free PR PRogram context \n");
      kosal_kfree(ppr_program_ctx, sizeof(struct pr_program_context));
   }

   PTRACEOUT;
}

///============================================================================
/// Name: ccipdrv_event_reconfig_event_create_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
///============================================================================
void ccipdrv_event_reconfig_event_create_send( struct pr_program_context *ppr_program_ctx,
                                               uid_afurespID_e            respID,
                                               btObjectType               devhandle,
                                               stTransactionID_t          *tranID,
                                               btObjectType               context,
                                               uid_errnum_e               errnum)
{
   struct ccipdrv_event_afu_response_event *pafuws_evt      = NULL;
   PTRACEIN;
   // return if PR program context is NULL
   ASSERT(NULL != ppr_program_ctx);
   if(NULL == ppr_program_ctx) {
      return ;
   }

#ifdef PWRMGR
   //PR failed, Movning Idle cores to online
   if ((ppr_program_ctx->m_cmd == ccipdrv_configureAFU) &&
      (uid_errnumOK != errnum)) {
      send_pr_power_event(ppr_program_ctx, 0);
   }
#endif

   pafuws_evt = ccipdrv_event_reconfig_event_create(respID,
                                                    devhandle,
                                                    tranID,
                                                    context,
                                                    errnum);

   ccidrv_sendevent(ppr_program_ctx->m_pownerSess,
                    AALQIP(pafuws_evt));


   if(NULL!= ppr_program_ctx->m_kbufferptr ) {
      kosal_free_user_buffer(ppr_program_ctx->m_kbufferptr, ppr_program_ctx->m_bufferlen);
   }

   kosal_sem_put(cci_dev_pr_sem(ppr_program_ctx->m_pPR_dev));
   PDEBUG("UN-LOCK RECONF \n");

   if(NULL != ppr_program_ctx){
      kosal_kfree(ppr_program_ctx, sizeof(struct pr_program_context));
   }
   PTRACEOUT;
}

///============================================================================
/// Name: prprogram_ctx_free
/// @brief frees PR program contxt
///
/// @param[in] pr_context - pr program context
/// @return    void
///============================================================================
void prprogram_ctx_free( struct pr_program_context *ppr_program_ctx)
{

   PTRACEIN;

   if( NULL == ppr_program_ctx) {
      return ;
   }

   if(NULL!= ppr_program_ctx->m_kbufferptr ) {
      kosal_free_user_buffer(ppr_program_ctx->m_kbufferptr, ppr_program_ctx->m_bufferlen);
   }

   kosal_sem_put(cci_dev_pr_sem(ppr_program_ctx->m_pPR_dev));
   PDEBUG("UN-LOCK RECONF \n");

   if(NULL != ppr_program_ctx){
      kosal_kfree(ppr_program_ctx, sizeof(struct pr_program_context));
   }
   PTRACEOUT;
}

///============================================================================
/// Name: ccipdrv_event_activationchange_event_create_send
/// @brief sends event to application
///
/// @param[in] pr_context - pr program context
/// @param[in] respID - pr response ID
/// @param[in] devhandle - device handle
/// @param[in] tranID -  transaction  ID
/// @param[in] context -  application context
/// @param[in] eno - pr error
/// @return    void
///============================================================================
void  ccipdrv_event_activationchange_event_create_send( struct pr_program_context *ppr_program_ctx,
                                                        uid_afurespID_e            respID,
                                                        btObjectType               devhandle,
                                                        stTransactionID_t          *tranID,
                                                        btObjectType               context,
                                                        uid_errnum_e               errnum)
{

   struct ccipdrv_event_afu_response_event *pafuws_evt      = NULL;
   PTRACEIN;
   // return if PR program context is NULL
   ASSERT(NULL != ppr_program_ctx);
   if(NULL == ppr_program_ctx) {
      return ;
   }

   pafuws_evt = ccipdrv_event_activationchange_event_create(respID,
                                                            devhandle,
                                                            tranID,
                                                            context,
                                                            errnum);

   ccidrv_sendevent(ppr_program_ctx->m_pownerSess,
                    AALQIP(pafuws_evt));

   kosal_sem_put(cci_dev_pr_sem(ppr_program_ctx->m_pPR_dev));
   PDEBUG("UN-LOCK RECONF \n");

   if(NULL != ppr_program_ctx){
      kosal_kfree(ppr_program_ctx, sizeof(struct pr_program_context));
   }
   PTRACEOUT;
}

///============================================================================
/// Name: program_afu_callback
/// @brief Reconfigures  AFU with bitstream
///
///
/// @param[in] pr_context - pr program context
/// @return    void
///============================================================================
void program_afu_callback(struct kosal_work_object * pwork)
{
   btVirtAddr   kptr                         = NULL;
   btWSSize   len                             = 0;
   btUnsigned64bitInt   PR_FIFO_credits       = 0;
   btUnsigned32bitInt   *byteRead             = NULL;
   uid_errnum_e   errnum                      = uid_errnumOK;
   btUnsigned64bitInt   counter               = 0;
   struct fme_device   *pfme_dev              = NULL;
   struct CCIP_FME_DFL_PR   *pr_dev           = NULL;
   struct pr_program_context *ppr_program_ctx = NULL;

   struct CCIP_FME_PR_CONTROL pr_control_local;
   struct CCIP_FME_PR_STATUS  pr_status_local;
   struct CCIP_FME_PR_ERROR  pr_err_local;
   ktime_t timeout;


   PTRACEIN;

   ppr_program_ctx = (struct pr_program_context*) kosal_get_object_containing( pwork, struct pr_program_context, m_workobject );

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   pfme_dev = cci_aaldev_pfme(ppr_program_ctx->m_pPR_dev);
   pr_dev   = ccip_fme_pr(pfme_dev);

   kptr = ppr_program_ctx->m_kbufferptr;
   len  = ppr_program_ctx->m_bufferlen;

#ifdef PWRMGR
    // Check Boundrys of bitstrem buffer TBD
    kptr = kptr + sizeof(struct CCIP_GBS_HEADER);
    len  = len - sizeof(struct CCIP_GBS_HEADER);
#endif

   PDEBUG("kptr =%p", kptr);
   PDEBUG("len =%d\n", (unsigned)len);
   PDEBUG("prregion_id =%d\n",ppr_program_ctx->m_prregion_id);

   // Don't do anything under simulation
   if(0 != sim){

      PDEBUG("Simulated reprogram \n");

      if(!ppr_program_ctx->m_leaveDeactivated){
         reconfigure_activateAFU(ppr_program_ctx->m_pportdev,ppr_program_ctx->m_pPR_dev);
      }

      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);
      return ;
   }
   
   // if BBS PR DFH revision is not 1 return InCompatiable bitstream error
   if (0x1 != pr_dev->ccip_pr_dflhdr.Feature_rev) {
   
    PERR(" PR InCompatiable BBS bitstream \n");
    errnum = uid_errnumIncompatibleBlueBitstream;
    goto ERR;
   }

   // Program the AFU
   // Reset PR Engine CSR_FME_PR_CONTROL[0] = 0x1 before Initiating PR
   // ---------------------------------------------------------------------------
   PVERBOSE("Resetting PR before initiated PR \n");
   Get64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   //CSR_FME_PR_CONTROL[0] = 0x1 and wait for reset ack bit set CSR_FME_PR_CONTROL[4]
   pr_control_local.pr_reset = 0x1;
   Set64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   timeout = ktime_add_us(ktime_get(), PR_OUTSTADREQ_TIMEOUT);

   do {
      // Sleep
      usleep_range((PR_OUTSTADREQ_DELAY >> 4) + 1, PR_OUTSTADREQ_DELAY);
      Get64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

      // if total delay is more then PR_OUTSTADREQ_TIMEOUT, returns  Timeout error
      if (ktime_compare(ktime_get(), timeout) > 0) {
          PERR("PR Reset Timeout Error  \n");
          errnum=uid_errnumPRTimeout;
          goto ERR;
      }

   } while( 0x1 != pr_control_local.pr_reset_ack);

   // Clear reset reset bit
   pr_control_local.pr_reset = 0x0;
   Set64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);
  
  
   // Step 1 - Check FME_PR_STATUS[27:24] == 4'h0
   // -------------------------------------------
   // Both Initially as well as after PR, HW updates PR status to this state
   PVERBOSE("Waiting for PR Resource in HW to be initialized and ready \n");

   timeout = ktime_add_us(ktime_get(), PR_OUTSTADREQ_TIMEOUT);
   do {
      // Sleep
      usleep_range((PR_OUTSTADREQ_DELAY >> 4) + 1, PR_OUTSTADREQ_DELAY);
      Get64CSR(&pr_dev->ccip_fme_pr_status.csr,&pr_status_local.csr);

      // if total delay is more then PR_OUTSTADREQ_TIMEOUT, returns  Timeout error
      if (ktime_compare(ktime_get(), timeout) > 0) {
          PERR("PR Status Timeout Error  \n");
          errnum=uid_errnumPRTimeout;
          goto ERR;
      }

   } while( CCIP_PORT_PR_Idle != pr_status_local.pr_host_status);

   PVERBOSE("HW is ready for PR \n");

   Get64CSR(&pr_dev->ccip_fme_pr_status.csr,&pr_status_local.csr);
   // Step 2 - Check FME_PR_STATUS[16] for any previous PR errors
   // --------------------------------------------------------------
   // FME_PR_STATUS[16] is PR PASS or FAIL bit. Different from SAS.
   // FME_PR_STATUS[16] is RO from SW point of view.
   PVERBOSE("Checking for errors in previous PR operation");

   if(0x1 == pr_status_local.pr_status)  {

      // Step 3 - Clear FME_PR_ERROR[5:0] - if needed based on Step - 2
      // -----------------------------------------------------------------
      // Upon failure, FME_PR_ERROR[5:0] gives additional error info.
      // FME_PR_ERROR[5:0] - All 6 bits are RW1CS - HW writes 1 upon error to set the bit. SW writes 1 to clear the bit
      // Once All error bits set in FME_PR_ERROR[5:0] are cleared by SW, HW will clear FME_PR_STATUS[16]
      // TODO: This step is different from SAS flow. Update SAS
      // NOTE: Error Logging and propagation might change based on SKX RAS

      Get64CSR(&pr_dev->ccip_fme_pr_err.csr,&pr_err_local.csr);

      if(0x1 == pr_err_local.PR_operation_err ) {
         pr_err_local.PR_operation_err =0x1;
         PERR(" PR Previous PR Operation Error  Detected \n");
      }

      if(0x1 == pr_err_local.PR_CRC_err ) {
         pr_err_local.PR_CRC_err =0x1;
         PERR(" PR Previous CRC Error Detected \n");
      }

      if(0x1 == pr_err_local.PR_bitstream_err ) {
         pr_err_local.PR_bitstream_err =0x1;
         PERR(" PR Previous InCompatiable bitstream Error  Detected \n");
      }

      if(0x1 == pr_err_local.PR_IP_err ) {
         pr_err_local.PR_IP_err =0x1;
         PERR(" PR Previous IP Protocol Error Detected \n");
      }

      if(0x1 == pr_err_local.PR_FIFIO_err ) {
         pr_err_local.PR_FIFIO_err =0x1;
         PERR(" PR  Previous FIFO Overflow Error Detected \n");
      }

      if(0x1 == pr_err_local.PR_timeout_err ) {
         pr_err_local.PR_timeout_err =0x1;
         PERR(" PR Previous Timeout  Error Detected \n");
      }

      if(0x1 == pr_err_local.PR_secure_load_err ) {
         pr_err_local.PR_secure_load_err =0x1;
         PERR(" PR Previous Secure Load  Error Detected \n");
      }
      PVERBOSE("Previous PR errors cleared \n ");

      Set64CSR(&pr_dev->ccip_fme_pr_err.csr,&pr_err_local.csr);

   } else
   {
      PVERBOSE("NO Previous PR errors \n ");
   }


 
   // Step 4 - Write PR Region ID to FME_PR_CONTROL[9:8]
   // --------------------------------------------------
   // NOTE: For BDX-P only 1 AFU is supported in HW. So, CSR_FME_PR_CONTROL[9:8] should always be 0
   // This will change for SKX

   Get64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   // set PR Region ID
   pr_control_local.pr_regionid = 0x0;


   // Step 5 - Initiate PR - Write 1 to FME_PR_CONTROL[12]
   // ---------------------------------------------------
   // SW can only initiate PR. HW will auto clear this bit upon failure or success or timeout
   PDEBUG("Initiate PR");
   // PR Start Request
   pr_control_local.pr_start_req = 0x1;

   Set64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   // Step 6 - Check available credits: FME_PR_STATUS[8:0] for PR data push and push Data to FME_PR_DATA[31:0]
   // -------------------------------------------------------------------------------------------------------
   // For instance,
   // if FME_PR_STATUS[8:0] read yields 511, SW can perform 511 32-bit writes from rbf file to FME_PR_DATA[31:0] and check credits again

   Get64CSR(&pr_dev->ccip_fme_pr_status.csr,&pr_status_local.csr);
   PR_FIFO_credits = pr_status_local.pr_credit;
   byteRead = (btUnsigned32bitInt  *)kptr;
   PDEBUG("Pushing Data from rbf to HW \n");


   while(len >0) {

      if (PR_FIFO_credits <= 1)  {
         do {

             Get64CSR(&pr_dev->ccip_fme_pr_status.csr,&pr_status_local.csr);
             PR_FIFO_credits = pr_status_local.pr_credit;

             // counter increment
             counter ++;

             // if counter value is more then PR_COUNTER_MAX_TRY,returns Timeout error.
             if (counter > PR_COUNTER_MAX_TRY)   {
                PERR("PR FIFI Credits Timeout Error \n");
                errnum=uid_errnumPRTimeout;
                goto ERR;
             }

          } while (PR_FIFO_credits <=1);
      }
      counter =0;
      // set 4 bytes bitstream
      if(len >=4) {
         SetCSR(&pr_dev->ccip_fme_pr_data.csr, byteRead);
         PR_FIFO_credits --;
         byteRead++;
         len -= 4;
      }else {
         // set remaining bits
         PERR(" Wrong Bitstream Size \n");
         errnum = uid_errnumPROperation;
         goto ERR;
      }
   } // end while

   // Step 7 - Notify the HW that bitstream push is complete
   // ------------------------------------------------------
   // Write 1 to CSR_FME_PR_CONTROL[13]. This bit is RW1S. SW writes 1 to set this bit.
   // Hardware will auto clear this bit upon PR completion
   // TODO: This step is currently not defined in SAS. Update SAS

   Get64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   pr_control_local.pr_push_complete = 0x1;

   Set64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

   PVERBOSE("Green bitstream push complete \n");

   // Step 8 - Once all data is pushed from rbf file, wait for PR completion
   // ----------------------------------------------------------------------
   // Check FME_PR_CONTROL[12] == 0.
   // Note: PR status bits are valid only when FME_PR_CONTROL[12] == 0.
   // FME_PR_CONTROL[12] is an atomic status check bit for initiating PR and also checking for PR completion
   // This bit set to 0 essentially means that HW has released the PR resource either due to PR PASS or PR FAIL.

   PVERBOSE("Waiting for HW to release PR resource \n");

   timeout = ktime_add_us(ktime_get(), PR_OUTSTADREQ_TIMEOUT);

   do {
      // Sleep
      usleep_range((PR_OUTSTADREQ_DELAY >> 4) + 1, PR_OUTSTADREQ_DELAY);
      Get64CSR(&pr_dev->ccip_fme_pr_control.csr,&pr_control_local.csr);

      // if total delay is more then PR_OUTSTADREQ_TIMEOUT, returns  Timeout error
      if (ktime_compare(ktime_get(), timeout) > 0) {
          PERR("PR Completion Timeout Error  \n");
          errnum=uid_errnumPRTimeout;
          goto ERR;
      }

   } while(0x0 != pr_control_local.pr_start_req);

   PVERBOSE("PR operation complete, checking Status \n");


   // Step 9 - Check PR PASS / FAIL
   // -----------------------------
   // Read the FME_PR_STATUS[16] to check PR success / fail
   // FME_PR_STATUS[16] = 0 implies PR Passed
   // FME_PR_STATUS[16] = 1 implies PR Failed. Read FME_PR_ERROR for more info upon failure
   // TODO: This step is different from SAS. Update SAS
   // NOTE: Error Register updating/ clearing may change based on SKX RAS requirement

   Get64CSR(&pr_dev->ccip_fme_pr_status.csr,&pr_status_local.csr);

   if(0x1 == pr_status_local.pr_status)  {

      Get64CSR(&pr_dev->ccip_fme_pr_err.csr,&pr_err_local.csr);

      if(0x1 == pr_err_local.PR_operation_err ) {
         PERR(" PR PR Operation Error  Detected \n");
         errnum=uid_errnumPROperation;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_CRC_err ) {
         PERR(" PR CRC Error Detected \n");
         errnum=uid_errnumPRCRC;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_bitstream_err ) {
         PERR(" PR Incomparable bitstream Error  Detected \n");
         errnum=uid_errnumPRIncompatibleBitstream;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_IP_err ) {
         PERR(" PR IP Protocol Error Detected \n");
         errnum=uid_errnumPRIPProtocal;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_FIFIO_err ) {
         PERR(" PR  FIFO Overflow Error Detected \n");
         errnum=uid_errnumPRFIFO;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_timeout_err ) {
         PERR(" PR Timeout  Error Detected \n");
         errnum=uid_errnumPRTimeout;
         goto ERR;
      }

      if(0x1 == pr_err_local.PR_secure_load_err ) {
         PERR(" PR Secure Load  Error Detected \n");
         errnum=uid_errnumPRSecureLoad;
         goto ERR;
      }

   } else  {

      PVERBOSE("PR Done Successfully\n");
   }

   // No need to AFU ReActive
   if( ppr_program_ctx->m_leaveDeactivated )   {

      PVERBOSE( " Not Activated AFU \n");
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);

   } else if(reconfigure_activateAFU(ppr_program_ctx->m_pportdev,ppr_program_ctx->m_pPR_dev) ) {

      PVERBOSE( " Activated AFU \n");
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);

   }
   else
   {
      errnum=uid_errnumAFUActivationFail;
      goto ERR;
   }

   PTRACEOUT;
   return ;

 ERR:

   // PR Failed and send Error event to application
   PVERBOSE( " PR Failed \n");
   ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,errnum);

   PTRACEOUT;
   return  ;
}

///============================================================================
/// Name: reconfigure_activateAFU
/// @brief Activates afu after reconfiguration
///
///
/// @param[in] pportdev - Port device pointer
/// @param[in] pdev - cci aal device pointer
/// @return    AFU activate status
///============================================================================
btBool  reconfigure_activateAFU(struct port_device  *pportdev,struct cci_aal_device  *pdev )
{

    // Get the AFU header pointer by adding the offset to the port header address
    struct cci_aal_device         *pcci_aaldev = NULL;
    struct aal_device_id           aalid;
    struct aal_device             *paaldevice = NULL;
    struct CCIP_AFU_Header        *pafu_hdr = (struct CCIP_AFU_Header *)(((btVirtAddr)ccip_port_hdr(pportdev) ) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset);
    btPhysAddr                     pafu_phys = ccip_port_phys_mmio(pportdev) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset;

    PTRACEIN;

    // Get the address of the PR. User AFU instance number always follows PR.
    paaldevice = cci_aaldev_to_aaldev(pdev);
    aalid = aaldev_devid(paaldevice);

    // enable afu
    port_afu_Enable(pportdev);
    // If the device is present
    if(~0ULL != pafu_hdr->ccip_dfh.csr){

       aaldevid_devaddr_busnum(aalid) = ccip_port_busnum(pportdev);
       aaldevid_devaddr_devnum(aalid) = ccip_port_devnum(pportdev);
       aaldevid_devaddr_fcnnum(aalid) = ccip_port_fcnnum(pportdev);
       aaldevid_devaddr_subdevnum(aalid) = 0x1;   

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
       kosal_list_add( &cci_aaldev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

    } // End if(~0ULL == pafu_hdr->ccip_dfh.csr){

    PTRACEOUT;
    return true;
}


///============================================================================
/// Name: afu_request_release_sendevent
/// @brief Sends Release event to Resource Owners
///
/// @param[in] pr_context -pr configuration context
/// @return    void
///============================================================================
void afu_request_release_revoke_sendevent(void* pr_context, btBool releaseAFU)
{
   struct aal_device *pafu_aal_dev = NULL;
   kosal_list_head *pitr           = NULL;
   kosal_list_head *temp           = NULL;
   struct aaldev_owner *pOwner     = NULL;

   struct pr_program_context  *ppr_program_ctx          = NULL;
   struct ccipdrv_event_afu_response_event *pafuws_evt  = NULL;

   PTRACEIN;

   ppr_program_ctx = (struct pr_program_context *)pr_context;
   pafu_aal_dev = ppr_program_ctx->m_pAFU_dev->m_aaldev;


   PVERBOSE( "AFU owners count= %d \n",pafu_aal_dev->m_numowners);

   // Lock the list from any updates
   if ( unlikely( kosal_sem_get_user_alertable(&pafu_aal_dev->m_sem) ) ) {
      PDEBUG("kosal_sem_get_user_alertable interrupted \n");
      return ;
   }


   // Send Release event to AFU owner application
   if ( !kosal_list_is_empty(&pafu_aal_dev->m_ownerlist) )  {

      // Loop through the list looking for a match
      kosal_list_for_each_safe(pitr, temp, &pafu_aal_dev->m_ownerlist) {

      // finds afu owner
      pOwner = kosal_container_of(pitr, struct aaldev_owner, m_ownerlist);

      PVERBOSE(  "AFU Owner pid= %d \n" , pOwner->m_pid);

      if( true == releaseAFU) {

      // Sends AFU Release Event to APP rspid_AFU_PR_Release_Request_Event
      pafuws_evt =ccipdrv_event_afu_aysnc_pr_request_release_create( ppr_program_ctx->m_respID,
                                                                     pOwner->m_sess.m_device,
                                                                     pOwner->m_sess.m_ownerContext,
                                                                     ppr_program_ctx->m_reconfAction,
                                                                     ppr_program_ctx->m_reconfTimeout,
                                                                     uid_errnumOK);
      }  else {

         pafuws_evt =ccipdrv_event_afu_aysnc_pr_revoke_create( ppr_program_ctx->m_respID,
                                                               pOwner->m_sess.m_device,
                                                               pOwner->m_sess.m_ownerContext,
                                                               uid_errnumOK);

      }
      ccidrv_sendevent(&(pOwner->m_sess),
                       AALQIP(pafuws_evt));
      } // end list

   }// endif

    kosal_sem_put(&pafu_aal_dev->m_sem);

   PTRACEOUT;
   return;

}

///============================================================================
/// Name: sigtap_revoke_sendevent
/// @brief Sends Revoke event to Resource Owners
///
/// @param[in] pr_context -pr configuration context
/// @return    void
///============================================================================
void sigtap_revoke_sendevent(void* pr_context)
{
   struct aal_device *psigtap_aal_dev = NULL;
   kosal_list_head *pitr              = NULL;
   kosal_list_head *temp              = NULL;
   struct aaldev_owner *pOwner        = NULL;

   struct pr_program_context  *ppr_program_ctx          = NULL;
   struct ccipdrv_event_afu_response_event *pafuws_evt  = NULL;

   PTRACEIN;

   ppr_program_ctx = (struct pr_program_context *)pr_context;
   psigtap_aal_dev = ccip_port_stap_dev(ppr_program_ctx->m_pportdev)->m_aaldev;

   PVERBOSE( "sigtap_revoke_sendevent  owners count= %d \n",psigtap_aal_dev->m_numowners);


   if ( unlikely( kosal_sem_get_user_alertable(&psigtap_aal_dev->m_sem) ) ) {
      PDEBUG("kosal_sem_get_user_alertable interrupted \n");
      return ;
   }


   // Send Release event to AFU owner application
   if ( !kosal_list_is_empty(&psigtap_aal_dev->m_ownerlist) )  {

      // Loop through the list looking for a match
      kosal_list_for_each_safe(pitr, temp, &psigtap_aal_dev->m_ownerlist) {

      // finds afu owner
      pOwner = kosal_container_of(pitr, struct aaldev_owner, m_ownerlist);

      PVERBOSE(  "AFU Owner pid= %d \n" , pOwner->m_pid);

      // Sends AFU Release Event to APP rspid_AFU_PR_Release_Request_Event

      pafuws_evt = ccipdrv_event_afu_aysnc_pr_revoke_create( ppr_program_ctx->m_respID,
                                                             pOwner->m_sess.m_device,
                                                             pOwner->m_sess.m_ownerContext,
                                                             uid_errnumOK);
      ccidrv_sendevent(&(pOwner->m_sess),
                       AALQIP(pafuws_evt));
      } // end list

   }// endif

   kosal_sem_put(&psigtap_aal_dev->m_sem);

   PTRACEOUT;
   return;

}
///============================================================================
/// Name: deactiavted_afu_device
/// @brief unpublish and deactivates AFU
///
/// @param[in] pportdev -port device pointer
/// @return    void
///============================================================================
void deactiavted_afu_device(struct port_device  *pportdev)
{
   PTRACEIN;

   port_afu_quiesce_and_halt(pportdev);
   cci_unpublish_aaldevice(ccip_port_uafu_dev(pportdev));
   cci_destroy_aal_device( ccip_port_uafu_dev(pportdev) );
   ccip_port_uafu_dev(pportdev) = NULL;

   PTRACEOUT;
}


///============================================================================
/// Name: afu_revoke_callback
/// @brief Revokes AFU from applications
///
/// @param[in] pr_context -pr configuration context
/// @param[in] ptr - null pointer.
/// @return    void
///============================================================================
void afu_revoke_callback(struct kosal_work_object *pwork)
{
   struct aal_device   *pafu_aal_dev                   = NULL;
   struct pr_program_context  *ppr_program_ctx         = NULL;

   PTRACEIN;
   ppr_program_ctx = (struct pr_program_context*) kosal_get_object_containing( pwork, struct pr_program_context, m_workobject );

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   if( 0 == ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners) {
      PVERBOSE(" No PR ALLDevice owners  \n" );
      prprogram_ctx_free(ppr_program_ctx);
      return ;
   }

   pafu_aal_dev = ppr_program_ctx->m_pAFU_dev->m_aaldev;

   // AFU has no owner (AFU revoked from application),Deactivate and Reconfigure AFU.
   if( 0 == pafu_aal_dev->m_numowners) {
      deactiavted_afu_device(ppr_program_ctx->m_pportdev);

      if(ppr_program_ctx->m_cmd == ccipdrv_configureAFU) {

         program_afu_callback(&(ppr_program_ctx->m_workobject));
         PTRACEOUT;
         return;
      }

      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);

   } else if (ppr_program_ctx->m_afuRevokeCount < PR_AFU_REVOKE_MAX_TRY) {
      // App Failed to release Resource with in timeout, try again.
      ppr_program_ctx->m_afuRevokeCount ++;

      KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),afu_revoke_callback);
      kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                               &(ppr_program_ctx->m_workobject),
                               PR_WQ_REVOKE_TIMEOUT);

   } else {
      PERR(" AFU Revoke Fail revoke_count=%lld  \n",ppr_program_ctx->m_afuRevokeCount );
      goto ERR;
   }

   PTRACEOUT;
   return ;
ERR:

   PVERBOSE(" AFU  Revoke Fail  \n" );
   ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumSigtapRevokeTimeout);

   PTRACEOUT;
   return;
}

///============================================================================
/// Name: pwrmgr_timeout_callback
/// @brief Power manager  timeout callback
///
/// @param[in] pr_context -pr configuration context
/// @return    void
///============================================================================
void pwrmgr_timeout_callback(struct kosal_work_object * pwork)
{
   struct pr_program_context  *ppr_program_ctx  = NULL;
   PTRACEIN;

   ppr_program_ctx = (struct pr_program_context*) kosal_get_object_containing( pwork, struct pr_program_context, m_workobject );

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   if( 0 == ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners) {
      PVERBOSE(" No PR ALLDevice owners  \n" );
      prprogram_ctx_free(ppr_program_ctx);
      return ;
   }

   cci_aaldev_pfme(ppr_program_ctx->m_pPR_dev)->m_pr_program_context = NULL;

   ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumPRPowerMgrTimeout);

   PTRACEOUT;

}

///============================================================================
/// Name: reconfigure_bitstream_cancelPwrmgrTimer
/// @brief Core Idle is done Start PR
///
/// @param[in] pr_context -pr configuration context
/// @param[in] pwr_status -Core Idle Status
/// @return    void
///============================================================================
void  reconfigure_bitstream_cancelPwrmgrTimer(struct pr_program_context* ppr_program_ctx,
                                              int pr_pwrmgmt_status)
{
   int isWQBusy =  0;

   PTRACEIN;

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   PERR(" reconfigure_bitstream_cancelPwrmgrTimer Enter \n");

   isWQBusy = work_busy(&(ppr_program_ctx->m_workobject.workobj.work));

   PDEBUG("isWQBusy %d\n",isWQBusy);

   // Cancel  Power Manager timer/ Work Queue
   if(isWQBusy) {
      PDEBUG("program_bs_callback CANCELED  \n");
      kosal_cancel_workqueue( &(ppr_program_ctx->m_workobject.workobj));
   }

   // If PR APP got closed, stop PR
   if( 0 == ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners) {
      PVERBOSE(" No PR ALLDevice owners  \n" );
      prprogram_ctx_free(ppr_program_ctx);
      return ;
   }

   // if Power Managemt Fails send Error message
   if(0 != pr_pwrmgmt_status) {

      PERR( "PR Power Manager Error =%d\n", pr_pwrmgmt_status);
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumPRPowerMgrCoreIdleFail);
      return ;
   }


   reconfigure_bitstream(ppr_program_ctx);

}

void  reconfigure_bitstream(struct pr_program_context* ppr_program_ctx)
{
   PTRACEIN;

   // CASE 1
   // if Signal tap object has owners.
   // sends signal tap release event and Do pr in worker queue
   // --------------------------------------------------------------------
   if(ppr_program_ctx->m_pSigtap_dev->m_aaldev->m_numowners  !=0) {

      PDEBUG("Signaltap Owners count %d \n",ppr_program_ctx->m_pSigtap_dev->m_aaldev->m_numowners);


      // Send Revoke event to signal tap Service
      sigtap_revoke_sendevent((void*)ppr_program_ctx);
      KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),sigtap_revoke_callback);

      kosal_queue_delayed_work( cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                               &(ppr_program_ctx->m_workobject),
                               AFU_RES_RELEASE_TIMEOUT);
      PTRACEOUT;
      return ;

   } // if end

   // CASE 2
   // if afu device object is null or Deactivated .
   // Do PR in worker queue
   // --------------------------------------------------------------------
   if(NULL == ccip_port_uafu_dev(ppr_program_ctx->m_pportdev)) {

      // afu DeActivated  and  Reconfigure AFU with bitstream
      PVERBOSE(" AFU is Deactivated  \n" );

      if(ppr_program_ctx->m_cmd == ccipdrv_configureAFU) {

         KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),program_afu_callback);
         kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                                  &(ppr_program_ctx->m_workobject),
                                  PR_WQ_TIMEOUT);
         PTRACEOUT;
         return;
      }

      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);
      PTRACEOUT;
      return ;
   } // if end

   ppr_program_ctx->m_pAFU_dev = ccip_port_uafu_dev(ppr_program_ctx->m_pportdev);
   PDEBUG(" paaldev->m_numowners=%d  \n",ppr_program_ctx->m_pAFU_dev->m_aaldev->m_numowners);

   // CASE 3
   // if afu has no owner.
   // Do PR in worker queue
   if( 0 == ppr_program_ctx->m_pAFU_dev->m_aaldev->m_numowners)  {

      PDEBUG(" AFU has no owner. \n");
      // if afu has no owner.
      // Do PR in worker queue
      // --------------------------------------------------------------------
      // AFU has no owner , unpublish AFU  and Configure with bitstream
      // AFU has no owner, DeActivate AFU ,Reconfigure AFU with bitstream
      deactiavted_afu_device(ppr_program_ctx->m_pportdev);

      if(ppr_program_ctx->m_cmd == ccipdrv_configureAFU)  {

         KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),program_afu_callback);
         kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                                  &(ppr_program_ctx->m_workobject),
                                 PR_WQ_TIMEOUT);
         PTRACEOUT;
         return;
      }

      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);
      PTRACEOUT;
      return ;

      // CASE 4
      // if afu has owner and reconfigure timeout is zero.
      // Sends afu release event and Deactivated timeout error event.
   } else if (( ppr_program_ctx->m_pAFU_dev->m_aaldev->m_numowners  >0) &&
             (0 == ppr_program_ctx->m_reconfTimeout) ) {

      // if afu has owner and reconfigure timeout is zero.
      // Sends afu release event and Deactivated timeout error event.
      // --------------------------------------------------------------------

      // AFU has  owners and Reconfigure timeout is 0 seconds , send Error Message  Deactivate timeout

      PDEBUG(" AFU has owner and reconfTimeout timeout. \n");
      // Send release event to application and timeout
      afu_request_release_revoke_sendevent((void*)ppr_program_ctx, true);
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumDeActiveTimeout);

      PTRACEOUT;
      return ;

      // CASE 4
      // if afu has owner and reconfigure timeout is greater then zero.
      // Sends afu release event and wait for AFU release in worker queue
   } else if ((ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners >0) &&
             (ppr_program_ctx->m_reconfTimeout >0) ) {

      // if afu has owner and reconfigure timeout is greater then zero.
      // Sends afu release event and wait for AFU release in worker queue
      // --------------------------------------------------------------------
      // AFU has  owners , unpublish AFU and configure with bitstream in worker thread

      PDEBUG(" AFU has owner and reconfTimeout timeout >0 \n");

         // Send release event to application
         afu_request_release_revoke_sendevent((void*)ppr_program_ctx, true);

         // starts Reconfigure timer worker thread if Reconfigure timeout is more then 0 seconds
         KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),afu_release_timeout_callback);

         kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                                  &(ppr_program_ctx->m_workobject),
                                  ppr_program_ctx->m_timeElapsed);


      PTRACEOUT;
      return ;
   } else {

      PERR(" PR Reconfiguration BadParameter \n");
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumBadParameter);


      PTRACEOUT;
      return ;
   } // end if else loop

   PTRACEOUT;
   return ;

}

int send_event_to_pwrmgr(struct pr_program_context* ppr_context, int power_required)
{
   int res = 0;

   PTRACEIN;

   // Send event to
   res = send_pr_power_event(ppr_context,power_required);
   if( 0 != res) {
      PERR(" No  Power device hasn't allocated \n");
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_context,uid_errnumNoPRPowerMgrDemon);
      return res;
   }

   KOSAL_INIT_WORK(&(ppr_context->m_workobject),pwrmgr_timeout_callback);
   kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_context->m_pPR_dev),
                            &(ppr_context->m_workobject),
                            PWRMGR_RESPONSE_TIMEOUT);

   PTRACEOUT_INT(res);

   return res;
}
///============================================================================
/// Name: sigtap_revoke_callback
/// @brief Revokes AFU from applications
///
/// @param[in] pr_context -pr configuration context
/// @return    void
///============================================================================
void sigtap_revoke_callback(struct kosal_work_object *pwork)
{
   struct aal_device          *psigtap_aal_dev          = NULL;
   struct pr_program_context  *ppr_program_ctx          = NULL;

   PTRACEIN;
   ppr_program_ctx = (struct pr_program_context*) kosal_get_object_containing( pwork, struct pr_program_context, m_workobject );

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   if( 0 == ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners) {
      PVERBOSE(" No PR ALLDevice owners  \n" );
      prprogram_ctx_free(ppr_program_ctx);
      PTRACEOUT;
      return ;
   }

   psigtap_aal_dev = ccip_port_stap_dev(ppr_program_ctx->m_pportdev)->m_aaldev;
   PVERBOSE(" psigtap_aal_dev->m_numowners %d = \n",psigtap_aal_dev->m_numowners );

   // AFU has no owner (AFU revoked from application),Deactivate and Reconfigure AFU.
   if( 0 == psigtap_aal_dev->m_numowners)  {

      // NO Singal Tap owners
      reconfigure_bitstream(ppr_program_ctx);

      PVERBOSE(" AFU is Deactivated  \n" );

   }else if(ppr_program_ctx->m_sigtapRevokeCount  < PR_AFU_REVOKE_MAX_TRY) {

      // App Failed to release Resource with in timeout, try again.
      ppr_program_ctx->m_sigtapRevokeCount ++ ;
      KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),sigtap_revoke_callback);

      kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                              &(ppr_program_ctx->m_workobject),
                              PR_WQ_REVOKE_TIMEOUT);
   } else {

      PERR(" AFU Revoke Fail revoke_count=%lld  \n",ppr_program_ctx->m_sigtapRevokeCount  );
      goto ERR;
   }

   PTRACEOUT;
   return ;

ERR:
   ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumDeActiveTimeout);
   PTRACEOUT;
   return;
}

///============================================================================
/// Name: afu_release_timeout_callback
/// @brief AFU Deactivate /reconfigure timeout callback
///
/// @param[in] pr_context -pr configuration context
/// @return    void
///============================================================================
void afu_release_timeout_callback(struct kosal_work_object *pwork)
{
   struct aal_device          *pafu_aal_dev            = NULL;
   struct aal_device          *ppr_aal_dev             = NULL;
   struct pr_program_context  *ppr_program_ctx         = NULL;
   btUnsigned64bitInt callbacktimeout                  = 0;

  // PTRACEIN;
   ppr_program_ctx = (struct pr_program_context*) kosal_get_object_containing( pwork, struct pr_program_context, m_workobject );

   if(NULL == ppr_program_ctx) {
      PERR("Invalid PR Context\n");
      return ;
   }

   if( 0 == ppr_program_ctx->m_pPR_dev->m_aaldev->m_numowners) {

      PVERBOSE(" No PR ALLDevice owners  \n" );
      prprogram_ctx_free(ppr_program_ctx);
      return ;
   }

   pafu_aal_dev = ppr_program_ctx->m_pAFU_dev->m_aaldev;
   ppr_aal_dev =  ppr_program_ctx->m_pPR_dev->m_aaldev;

   //PVERBOSE( "AFU count %d \n",pafu_aal_dev->m_numowners);

   // AFU has no owner , Deactivate and Reconfigure AFU
   if( 0 == pafu_aal_dev->m_numowners)  {

      // Deactivate and Reconfigure AFU
      deactiavted_afu_device(ppr_program_ctx->m_pportdev);

      if(ppr_program_ctx->m_cmd == ccipdrv_configureAFU)  {
         program_afu_callback(&(ppr_program_ctx->m_workobject));
         PTRACEOUT;
         return;
      }

      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumOK);
      PVERBOSE(" AFU is Deactivated  \n" );

      PTRACEOUT;
      return ;

    } // end if

   // if total time elapsed is less then reconfigure timeout.
   if(ppr_program_ctx->m_timeElapsed < ppr_program_ctx->m_reconfTimeout) {

      //PVERBOSE(" Try again  \n" );

      if((ppr_program_ctx->m_timeElapsed +AFU_RES_RELEASE_TIMEOUT) <= ppr_program_ctx->m_reconfTimeout) {

         // workqueue callback called after AFU_RES_RELEASE_TIMEOUT
         ppr_program_ctx->m_timeElapsed = ppr_program_ctx->m_timeElapsed +AFU_RES_RELEASE_TIMEOUT;
         callbacktimeout =AFU_RES_RELEASE_TIMEOUT;
      }
      else {
         // workqueue callback called after callbacktimeout
         callbacktimeout = (ppr_program_ctx->m_reconfTimeout - ppr_program_ctx->m_timeElapsed);
      }

      KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),afu_release_timeout_callback);
      kosal_queue_delayed_work(cci_aaldev_workq_reconf(ppr_program_ctx->m_pPR_dev),
                               &(ppr_program_ctx->m_workobject),
                               callbacktimeout);

     // PTRACEOUT;
      return ;
   }

   // AFU has owner, Reconfigure action Honor Owner
   // If we are told to honor then since owner did not relinquish AFU timeout request
   if((pafu_aal_dev->m_numowners >0 ) &&
      (ReConf_Action_Honor_Owner == ppr_program_ctx->m_reconfAction)) {

      PVERBOSE("AFU  DeActiavated Timeout  \n" );
      ccipdrv_event_afu_aysnc_pr_release_send(ppr_program_ctx,uid_errnumDeviceBusy);

    } else {

      PVERBOSE("Revoke AFU form Application  \n" );
      // Reconfigure action Honor Request, sends Revoke event to AAL Service
      // AAL Frame work failed to release AFU, driver send error event
      afu_request_release_revoke_sendevent((void*)ppr_program_ctx,false);
      afu_revoke_callback(&(ppr_program_ctx->m_workobject));

    }
   PTRACEOUT;
   return;
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
               cci_create_AAL_PR_Device( struct fme_device  *pfmedev,
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
   cci_aaldev_type(pcci_aaldev) = cci_dev_PR;

   // Record parentage
   cci_aaldev_pfme(pcci_aaldev)     = pfmedev;
   cci_aaldev_pci_dev(pcci_aaldev)  = ccip_fme_dev_pci_dev(pfmedev) ;

   // Device Address is the same as the Port. Set the AFU ID information
   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(*paalid)            = CCIP_PR_GUIDL;
   aaldevid_afuguidh(*paalid)            = CCIP_PR_GUIDH;
   aaldevid_pipguid(*paalid)             = CCIP_PR_PIPIID;

   // Setup the MMIO region parameters
   cci_aaldev_kvp_afu_mmio(pcci_aaldev)   = (btVirtAddr)ccip_fme_pr(pfmedev);
   cci_aaldev_len_afu_mmio(pcci_aaldev)   = sizeof(struct CCIP_FME_DFL_PR);
   cci_aaldev_phys_afu_mmio(pcci_aaldev)  = kosal_virt_to_phys((btVirtAddr)ccip_fme_pr(pfmedev));

   // Create the AAL device and attach it to the CCI device object
   cci_aaldev_to_aaldev(pcci_aaldev) =  aaldev_create( "CCIPPR",           // AAL device base name
                                                       paalid,             // AAL ID
                                                       &cci_PRpip);

   //CCI device object create fails, delete PR AAL device
   if(NULL == cci_aaldev_to_aaldev(pcci_aaldev) ){
      cci_destroy_aal_device(pcci_aaldev);
      return NULL;
   }

   // Set up reverse look up. Use aaldev_to_cci_aal_device() to access
   aaldev_context(cci_aaldev_to_aaldev(pcci_aaldev)) = pcci_aaldev;

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Initialize the worker thread
   cci_aaldev_workq_reconf( pcci_aaldev )        = kosal_create_workqueue( "ReconfWQ", cci_aaldev_to_aaldev( pcci_aaldev ) );

   // Set how many owners are allowed access to this device simultaneously
   cci_aaldev_to_aaldev(pcci_aaldev)->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;

   if( cci_aaldev_allow_map_mmior_space(pcci_aaldev) ){
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

   if ( unlikely( kosal_sem_get_user_alertable(cci_dev_pr_sem(pdev)))) {
         PDEBUG("kosal_sem_get_user_alertable interrupted \n");
         return -EINVAL ;
      }

   PDEBUG("LOCK RECONF \n");

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {


      AFU_COMMAND_CASE(ccipdrv_deactivateAFU) {
         struct ccipdrv_event_afu_response_event *pafuws_evt = NULL;
         struct aal_device        *paaldev                   = NULL;
         struct port_device       *pportdev                  = NULL;
         btTime                   reconfTimeout              = 0;
         btUnsigned64bitInt       reconfAction               = 0;
         btBool                   leaveDeactivated           = 0;
         struct pr_program_context* ppr_program_ctx          = NULL;
         struct aal_device        *psigtapdev                = NULL;

         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;

         // Extract arguments
         reconfTimeout     = preq->ahmreq.u.pr_config.reconfTimeout;
         reconfAction      = RECONF_ACTION_HONOR_PARAMETER(preq->ahmreq.u.pr_config.reconfAction);
         leaveDeactivated  = RECONF_ACTION_ACTIVATE_PARAMETER(preq->ahmreq.u.pr_config.reconfAction);

         PDEBUG("reconfTimeout=%lld\n",reconfTimeout);
         PDEBUG("reconfAction=%lld\n",reconfAction);

         // Port for this AAL PR object
         //pportdev = cci_aaldev_pport(pdev);
         pportdev = getport_device(pdev,0);

         // Find the AFU device associated with this port
         // Make sure device is not in use. If it is notify user and start time out timer.

         // Case 1 - if AFU device object is null.
         // Sends No afu error event
         // --------------------------------------------------------------------
         if(NULL == ccip_port_uafu_dev(pportdev)){

            PDEBUG("No AFU Device \n");
            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespDeactivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumNoAFU);

            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));

            goto ERROR;

         }

         // Determine if applications currently have the AFU in use
         paaldev = cci_aaldev_to_aaldev(ccip_port_uafu_dev(pportdev));

         ppr_program_ctx = (struct pr_program_context*)kosal_kzmalloc( sizeof(struct pr_program_context) );


         // Case 2 - if PR context Memory allocation filed.
         // Sends  No Memory error event
         // --------------------------------------------------------------------
         if(NULL == ppr_program_ctx){
            PERR("Unable to allocate system memory for pr context\n");

            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespDeactivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumNoMem);
             ccidrv_sendevent(pownerSess,
                              AALQIP(pafuws_evt));
             goto ERROR;
         }

         // Assign PR context
         ppr_program_ctx->m_cmd               = ccipdrv_deactivateAFU;
         ppr_program_ctx->m_pPR_dev           = pdev;
         ppr_program_ctx->m_pAFU_dev          = ccip_port_uafu_dev(pportdev);
         ppr_program_ctx->m_pportdev          = pportdev;
         ppr_program_ctx->m_pownerSess        = pownerSess;
         ppr_program_ctx->m_respID            = uid_afurespDeactivateComplete;
         ppr_program_ctx->m_kbufferptr        = NULL;
         ppr_program_ctx->m_bufferlen         = 0;
         ppr_program_ctx->m_leaveDeactivated  = leaveDeactivated;
         ppr_program_ctx->m_reconfTimeout     = reconfTimeout;
         ppr_program_ctx->m_reconfAction      = reconfAction;
         ppr_program_ctx->m_afuRevokeCount    = 0;
         ppr_program_ctx->m_timeElapsed       = AFU_RES_RELEASE_TIMEOUT;
         ppr_program_ctx->m_sigtapRevokeCount = 0;
         ppr_program_ctx->m_afuRevokeCount    = 0;

         // set timeout
         if(reconfTimeout <= AFU_RES_RELEASE_TIMEOUT) {
            ppr_program_ctx->m_timeElapsed =reconfTimeout;
         }

         // Signal Tap Resource
         psigtapdev = ccip_port_stap_dev(pportdev)->m_aaldev;


         // Case 3 - if Signal tap object has owners.
         // sends signal tap release event and Do pr in worker queue
         // --------------------------------------------------------------------
         if(psigtapdev->m_numowners  !=0) {

            PDEBUG("Signaltap Owners count %d \n",psigtapdev->m_numowners);
            // Send Revoke event to signal tap Service
            sigtap_revoke_sendevent((void*)ppr_program_ctx);
            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),sigtap_revoke_callback);

            kosal_queue_delayed_work( cci_aaldev_workq_reconf(pdev),
                                     &(ppr_program_ctx->m_workobject),
                                     AFU_RES_RELEASE_TIMEOUT);

            return 0;

         } // if end


         // Case 4 - if AFU has no owner.
         // Sends OK event
         // --------------------------------------------------------------------
         if( 0 == paaldev->m_numowners)  {

            PDEBUG("AFU Owner Count=%d\n", paaldev->m_numowners);

            // AFU is free so deactivate
            //(TODO;  We need to make sure there are no races of something trying allocate this.  May need to lock the device
            // from getting a bind request.)
            deactiavted_afu_device(pportdev);
            ccipdrv_event_activationchange_event_create_send(ppr_program_ctx,
                                                             uid_afurespDeactivateComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumOK);
            goto CLEANUP;

         } else if (0 == reconfTimeout) {

            // Case 5 - if AFU has owner and zero timeout.
            // Sends deactivate timeout error event
            // --------------------------------------------------------------------

            // Number of owner is more then 0 but Reconfiguration timeout is 0 Seconds
            //   We generate an event to the application to release (TODO) and time out the deactivate will timeout
            PDEBUG("reconfTimeout is 0 \n");
            // generate an event to the application to release AFU
            // time out the deactivate will timeout
            //afu_request_release_sendevent((void*)ppr_program_ctx);
            afu_request_release_revoke_sendevent((void*)ppr_program_ctx, true);

            ccipdrv_event_activationchange_event_create_send(ppr_program_ctx,
                                                             uid_afurespDeactivateComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumDeActiveTimeout);

            goto ERROR;

          } else if(reconfTimeout >0) {

             // Case 6 - if  AFU has owner and timeout.
             // Deactivate afu in worker queue
             // --------------------------------------------------------------------
            // AFU in use and a timeout has been specified
            // Number of owner is more then 0 but Reconfiguration timeout is more then Seconds
            // Send Release Event to the application to release AFU and start deactivate timer

            PDEBUG("Deactivate timeout thread \n");
            afu_request_release_revoke_sendevent((void*)ppr_program_ctx, true);

            // starts Reconfigure timer worker thread if Reconfigure timeout is more then 0 seconds
            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),afu_release_timeout_callback);

            kosal_queue_delayed_work( cci_aaldev_workq_reconf(pdev),
                                   &(ppr_program_ctx->m_workobject),
                                   ppr_program_ctx->m_timeElapsed);

            return 0;
          } else {

             // Case 7 - Invalid parameter.
             // sends Bad Parameter error event
             // --------------------------------------------------------------------
             PERR("Deactivate Bad Parameter \n");
             ccipdrv_event_activationchange_event_create_send(ppr_program_ctx,
                                                              uid_afurespDeactivateComplete,
                                                              pownerSess->m_device,
                                                              &Message->m_tranID,
                                                              Message->m_context,
                                                              uid_errnumBadParameter);

             goto ERROR;

          } // end of if else

      } break;

      AFU_COMMAND_CASE(ccipdrv_activateAFU) {

         // Port for this AAL PR object
         //struct port_device  *pportdev                         = cci_aaldev_pport(pdev);
         struct port_device  *pportdev = getport_device(pdev,0);
         struct ccipdrv_event_afu_response_event *pafuws_evt   = NULL;
         // Find the AFU device associated with this port
         if(NULL != ccip_port_uafu_dev(pportdev)){
            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespActivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumAFUNotActivated);

            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));

            goto ERROR;
         }



         if(!reconfigure_activateAFU(pportdev,pdev))
         {

            pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespActivateComplete,
                                                                     pownerSess->m_device,
                                                                     &Message->m_tranID,
                                                                     Message->m_context,
                                                                     uid_errnumAFUNotActivated);

            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));
            goto ERROR;
         }


         pafuws_evt = ccipdrv_event_activationchange_event_create(uid_afurespActivateComplete,
                                                                  pownerSess->m_device,
                                                                  &Message->m_tranID,
                                                                  Message->m_context,
                                                                  uid_errnumOK);

         ccidrv_sendevent(pownerSess,
                          AALQIP(pafuws_evt));

         goto CLEANUP;

      } break;

      AFU_COMMAND_CASE(ccipdrv_configureAFU) {
         struct ccipdrv_event_afu_response_event *pafuws_evt = NULL;
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;

         // Get a copy of the bitfile image from user space.
         //  This function returns a safe pointer to the user data.
         //  This may involve copying into kernel space.

         //------------------------------------------------------------------------------------------------------
         struct aal_device        *paaldev          = NULL;
         struct port_device       *pportdev         = NULL;
         btTime                   reconfTimeout     = 0;
         btUnsigned64bitInt       reconfAction      = 0;
         btBool                   leaveDeactivated  = 0;
         btVirtAddr               kptr              = NULL;
         struct aal_device        *psigtapdev       = NULL;
         struct pr_program_context* ppr_program_ctx = NULL;

         btWSSize buflen = preq->ahmreq.u.pr_config.size;
         btVirtAddr uptr = preq->ahmreq.u.pr_config.vaddr;

         // Extract arguments
         reconfTimeout     = preq->ahmreq.u.pr_config.reconfTimeout;
         reconfAction      = RECONF_ACTION_HONOR_PARAMETER(preq->ahmreq.u.pr_config.reconfAction);
         leaveDeactivated  = RECONF_ACTION_ACTIVATE_PARAMETER(preq->ahmreq.u.pr_config.reconfAction);

         PVERBOSE( "ccipdrv_configureAFU  \n");
         PDEBUG("reconfTimeout=%lld\n",reconfTimeout);
         PDEBUG("reconfAction=%lld\n" ,reconfAction);

         //pportdev = cci_aaldev_pport(pdev);
         pportdev = getport_device(pdev,0);

         // Case 1 - if bitstream buffer is null or buffer length is zero.
         // sends bad parameter error event .
         // --------------------------------------------------------------------
         if((NULL == uptr) || (0 == buflen)){

            PERR("AFU reprogramming failed\n");
            pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumBadParameter);

            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));
            goto ERROR;
         }

         kptr = kosal_get_user_buffer(uptr, buflen);

         // Case 2 - if kernel memory allocation for bitstream fails.
         // sends no memory error event.
         // --------------------------------------------------------------------
         // Allocation Fails send No Memory error to application
         if(NULL == kptr ){
            PERR("kosal_get_user_buffer returned NULL");
            pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                             pownerSess->m_device,
                                                             &Message->m_tranID,
                                                             Message->m_context,
                                                             uid_errnumNoMem);
            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));
            goto ERROR;
         }

         PVERBOSE( " PR BitStream Buffer allocated  \n");
         ppr_program_ctx = (struct pr_program_context*) kosal_kzmalloc( sizeof(struct pr_program_context) );

         // Case 3 - if kernel memory allocation for PR context fails.
         // sends no memory error event.
         // --------------------------------------------------------------------
         if(NULL == ppr_program_ctx){

            PERR("Unable to allocate system memory for pr context\n");
            if(NULL != kptr)
               kosal_free_user_buffer(kptr, buflen);

            pafuws_evt = ccipdrv_event_reconfig_event_create(uid_afurespConfigureComplete,
                                                            pownerSess->m_device,
                                                            &Message->m_tranID,
                                                            Message->m_context,
                                                            uid_errnumNoMem);
            ccidrv_sendevent(pownerSess,
                            AALQIP(pafuws_evt));
            goto ERROR;
         }

         // Assign PR context
         ppr_program_ctx->m_cmd               = ccipdrv_configureAFU;
         ppr_program_ctx->m_pPR_dev           = pdev;
         ppr_program_ctx->m_pportdev          = pportdev;
         ppr_program_ctx->m_pownerSess        = pownerSess;
         ppr_program_ctx->m_respID            = uid_afurespConfigureComplete;
         ppr_program_ctx->m_kbufferptr        = kptr;
         ppr_program_ctx->m_bufferlen         = buflen;
         ppr_program_ctx->m_leaveDeactivated  = leaveDeactivated;
         ppr_program_ctx->m_reconfTimeout     = reconfTimeout;
         ppr_program_ctx->m_reconfAction      = reconfAction;
         ppr_program_ctx->m_afuRevokeCount    = 0;
         ppr_program_ctx->m_timeElapsed       = AFU_RES_RELEASE_TIMEOUT;
         ppr_program_ctx->m_sigtapRevokeCount = 0;
         ppr_program_ctx->m_afuRevokeCount    = 0;
         ppr_program_ctx->m_pSigtap_dev       = ccip_port_stap_dev(pportdev);
         // Milliseconds
         if(reconfTimeout <= AFU_RES_RELEASE_TIMEOUT)
            ppr_program_ctx->m_timeElapsed =reconfTimeout;

         // Signal Tap Resource
         psigtapdev = ccip_port_stap_dev(pportdev)->m_aaldev;

#ifdef PWRMGR
         // Green bitstream Header
         ppr_program_ctx->m_gbs_header = (struct CCIP_GBS_HEADER *) kptr;

         PDEBUG("GBS Power %d \n",ppr_program_ctx->m_gbs_header->m_md_afu_power);
         PDEBUG("GBS Clock %d \n",ppr_program_ctx->m_gbs_header->m_md_clknum);

         // Sends Event to power Manger
         send_event_to_pwrmgr(ppr_program_ctx,ppr_program_ctx->m_gbs_header->m_md_afu_power);
         return 0;

#else
          // Case 4 - if Signal tap object has owners.
         // sends signal tap release event and Do pr in worker queue
         // --------------------------------------------------------------------
         if(psigtapdev->m_numowners  !=0) {

            PDEBUG("Signaltap Owners count %d \n",psigtapdev->m_numowners);
            // Send Revoke event to signal tap Service
            sigtap_revoke_sendevent((void*)ppr_program_ctx);
            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),sigtap_revoke_callback);

            kosal_queue_delayed_work( cci_aaldev_workq_reconf(pdev),
                                     &(ppr_program_ctx->m_workobject),
                                     AFU_RES_RELEASE_TIMEOUT);

            return 0;

         } // if end

         // Case 5 - if afu device object is null or Deactivated .
         // Do PR in worker queue
         // --------------------------------------------------------------------
         if(NULL == ccip_port_uafu_dev(pportdev)) {


            PDEBUG("AFU Deactivated  \n");
            // afu DeActivated  and  Reconfigure AFU with bitstream

            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),program_afu_callback);

            kosal_queue_delayed_work(cci_aaldev_workq_reconf(pdev),
                                     &(ppr_program_ctx->m_workobject),
                                     PR_WQ_TIMEOUT);

            return 0;
         } // if end


         paaldev = cci_aaldev_to_aaldev(ccip_port_uafu_dev(pportdev));
         ppr_program_ctx->m_pAFU_dev = ccip_port_uafu_dev(pportdev);

         PDEBUG(" paaldev->m_numowners=%d  \n",paaldev->m_numowners);


         if( 0 == paaldev->m_numowners)  {

            // Case 6 - if afu has no owner.
            // Do PR in worker queue
            // --------------------------------------------------------------------
            // AFU has no owner , unpublish AFU  and Configure with bitstream

            // TODO FOR NOW JUST DO IT
            // AFU has no owner, DeActivate AFU ,Reconfigure AFU with bitstream
            deactiavted_afu_device(pportdev);

            PDEBUG(" AFU has no owner. \n");

            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),program_afu_callback);

            kosal_queue_delayed_work(cci_aaldev_workq_reconf(pdev),
                                    &(ppr_program_ctx->m_workobject),
                                    PR_WQ_TIMEOUT);
            return 0;

         } else if (( paaldev->m_numowners  >0) &&
                   (0 == reconfTimeout) ) {

            // Case 7 - if afu has owner and reconfigure timeout is zero.
            // Sends afu release event and Deactivated timeout error event.
            // --------------------------------------------------------------------

            // AFU has  owners and Reconfigure timeout is 0 seconds , send Error Message  Deactivate timeout

            PDEBUG(" AFU has owner and reconfTimeout timeout. \n");
            // Send release event to application and timeout
            afu_request_release_revoke_sendevent((void*)ppr_program_ctx,false);

            ccipdrv_event_reconfig_event_create_send(ppr_program_ctx,
                                                     uid_afurespConfigureComplete,
                                                     pownerSess->m_device,
                                                     &Message->m_tranID,
                                                     Message->m_context,
                                                     uid_errnumDeActiveTimeout);
            goto ERROR;

         } else if ((paaldev->m_numowners >0) &&
                   (reconfTimeout >0) ) {

            // Case 8 - if afu has owner and reconfigure timeout is greater then zero.
            // Sends afu release event and wait for AFU release in worker queue
            // --------------------------------------------------------------------

            // AFU has  owners , unpublish AFU and configure with bitstream in worker thread

            PDEBUG(" AFU has owner and reconfTimeout timeout >0 \n");
            // Send release event to application
            afu_request_release_revoke_sendevent((void*)ppr_program_ctx,false);

            // starts Reconfigure timer worker thread if Reconfigure timeout is more then 0 seconds
            KOSAL_INIT_WORK(&(ppr_program_ctx->m_workobject),afu_release_timeout_callback);

            kosal_queue_delayed_work(cci_aaldev_workq_reconf(pdev),
                                  &(ppr_program_ctx->m_workobject),
                                  ppr_program_ctx->m_timeElapsed);

            return 0;
         } else {

            PERR(" PR Reconfiguration BadParameter \n");

            // Case 9 - Invalid parameter.
            // Sends bad parameter error event to app
            // --------------------------------------------------------------------
            ccipdrv_event_reconfig_event_create_send(ppr_program_ctx,
                                                     uid_afurespConfigureComplete,
                                                     pownerSess->m_device,
                                                     &Message->m_tranID,
                                                     Message->m_context,
                                                     uid_errnumBadParameter);

            goto ERROR;
         } // end if else loop
#endif
        goto CLEANUP;

      }break;

      // Returns a workspace ID for the Config Space

      AFU_COMMAND_CASE(ccipdrv_getMMIORmap) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;
         struct aalui_WSMEvent WSID;
         struct aal_wsid   *wsidp            = NULL;

         if ( !cci_aaldev_allow_map_mmior_space(pdev) ) {
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

         PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_aaldev_phys_afu_mmio(pdev), (int)cci_aaldev_len_afu_mmio(pdev));

         // Set up the return payload
         WSID.evtID           = uid_wseventMMIOMap;
         WSID.wsParms.wsid    = pwsid_to_wsidHandle(wsidp);
         WSID.wsParms.physptr = cci_aaldev_phys_afu_mmio(pdev);
         WSID.wsParms.size    = cci_aaldev_len_afu_mmio(pdev);

         // Make this atomic. Check the original response buffer size for room
         if(respBufSize >= sizeof(struct aalui_WSMEvent)){
            *((struct aalui_WSMEvent*)Message->m_response) = WSID;
            Message->m_respbufSize = sizeof(struct aalui_WSMEvent);
         }
         PDEBUG("Buf size =  %u Returning WSID %llx\n",(unsigned int)Message->m_respbufSize, WSID.wsParms.wsid  );
         Message->m_errcode = uid_errnumOK;

         // Add the new wsid onto the session
         aalsess_add_ws(pownerSess, wsidp->m_list);

         goto CLEANUP;
      } break;

      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_ALLOC)
      {
         struct ccipdrv_event_afu_response_event *pafuws_evt = NULL;
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

            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));

            retval = -ENOMEM;
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
                                               pwsid_to_wsidHandle(wsidp), // make the wsid appear page aligned for mmap
                                               NULL,
                                               kosal_virt_to_phys((btVirtAddr)wsidp->m_id),
                                               preq->ahmreq.u.wksp.m_size,
                                               Message->m_tranID,
                                               Message->m_context,
                                               uid_errnumOK);

         PVERBOSE("Sending the WKSP Alloc event.\n");
         // Send the event
         ccidrv_sendevent(pownerSess,
                          AALQIP(pafuws_evt));
         goto CLEANUP;

      } break; // case fappip_afucmdWKSP_VALLOC


      //============================
      //  Free Workspace
      //============================
      AFU_COMMAND_CASE(ccipdrv_afucmdWKSP_FREE) {
         struct ccipdrv_event_afu_response_event *pafuws_evt = NULL;
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
            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));
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
            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));

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
            ccidrv_sendevent(pownerSess,
                             AALQIP(pafuws_evt));

            retval = -EFAULT;
            goto ERROR;
         }

         krnl_virt = (btVirtAddr)wsidp->m_id;

         kosal_free_contiguous_mem(krnl_virt, wsidp->m_size);

         // remove the wsid from the device and destroy
         kosal_list_del_init(&wsidp->m_list);
         ccidrv_freewsid(wsidp);

         // Create the  event
         pafuws_evt = ccipdrv_event_afu_afufreecws_create(pownerSess->m_device,
                                                        Message->m_tranID,
                                                        Message->m_context,
                                                        uid_errnumOK);

         PVERBOSE("Sending the WKSP Free event.\n");
         // Send the event
         ccidrv_sendevent(pownerSess,
                          AALQIP(pafuws_evt));
         goto CLEANUP;
      } break; // case fappip_afucmdWKSP_FREE

      default: {

         PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);
         Message->m_errcode = request_error;
         retval = -EINVAL;

         kosal_sem_put(cci_dev_pr_sem(pdev));

       return retval;
      } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

CLEANUP:
    kosal_sem_put(cci_dev_pr_sem(pdev));
    PDEBUG("UN-LOCK RECONF \n");
    return retval;

ERROR:
   kosal_sem_put(cci_dev_pr_sem(pdev));
   PDEBUG("UN-LOCK RECONF \n");
   return retval;
}


//=============================================================================
/// Name: getport_device
/// @brief get port device  pointer
///
/// @param[in] pdev    -aal device pointer
/// @param[in] portId  -port index
/// @return    port device pointer
//=============================================================================
struct port_device * getport_device(struct cci_aal_device  *pdev , int portId)
{
   struct ccip_device   *pccidev    = NULL;
   struct list_head     *This       = NULL;
   struct list_head     *tmp        = NULL;
   struct port_device   *pport_dev = NULL;

   PTRACEIN;

   //lock g_device_list
   //kosal_sem_get_krnl(&g_dev_list_sem);

   // Search through our list of devices to find the one matching pcidev
   if ( !kosal_list_is_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         pccidev = ccip_list_to_ccip_device(This);

         // get port device if matching Bus, Device, function of PCIe device
         if (pdev->m_pcidev->bus->number == ccip_dev_pcie_busnum(pccidev) &&
            PCI_SLOT(pdev->m_pcidev->devfn) == ccip_dev_pcie_devnum(pccidev) &&
            PCI_FUNC(pdev->m_pcidev->devfn) == ccip_dev_pcie_fcnnum(pccidev)) {

             pport_dev = pccidev->m_pport_dev[portId];
         }
         
      }
   }
   //unlock g_device_list
   //kosal_sem_put(&g_dev_list_sem);

   PTRACEOUT;
   return pport_dev;
}
