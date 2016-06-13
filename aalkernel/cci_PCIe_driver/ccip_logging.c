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
/// @file ccip_logging.h
/// @brief  Definitions for ccip logging.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_logging.h
//     CREATED: June 07, 2016
//      AUTHOR: , Intel Corporation
//
//
// PURPOSE:
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "cci_pcie_driver_internal.h"
#include "aalsdk/kernel/ccip_defs.h"
#include "ccip_fme.h"
#include "ccip_port.h"
#include "ccip_logging.h"

BEGIN_NAMESPACE(AAL)


// ccip board device list
extern kosal_list_head g_device_list;
extern kosal_semaphore g_dev_list_sem;

// logging timer struct
struct logging_msg   g_logging_msg;

// Logging Timer value in milliseconds
#define  CCIP_LOGGGING_TIMER_VALUE  1000

///============================================================================
/// Name:    create_logging_timer
/// @brief   creates ccip logging polling time.
///
/// @return    error code
///============================================================================
int create_logging_timer(void)
{
   int res = 0;
   PTRACEIN;

   logging_msg_wq(g_logging_msg) = kosal_create_workqueue( "LoggingTimer",NULL);
   if(NULL ==  logging_msg_wq(g_logging_msg)) {
      res= -ENOMEM;
      return res;
   }

   kosal_mutex_init(logging_msg_sem(g_logging_msg));
   logging_msg_time(g_logging_msg) = CCIP_LOGGGING_TIMER_VALUE;


   PTRACEOUT;
   return  res;
}

///============================================================================
/// Name:    start_logging_timer
/// @brief   starts logging timer.
///
/// @return    error code
///============================================================================
int start_logging_timer(void)
{
   int res = 0;
   PTRACEIN;

   //  if work queue failed to initialize,no need to start work queue
   if( NULL == logging_msg_wq(g_logging_msg)) {

      res= -EFAULT;
      return res;
   }

   kosal_sem_get_krnl(logging_msg_sem(g_logging_msg) );

   // checking for  work queue is  running /started
   if(logging_timer_Running == logging_msg_wq_status(g_logging_msg) )  {

      kosal_sem_put( logging_msg_sem(g_logging_msg));
      res = -EBUSY ;
      return res;
   }


   // Start logging timer work queue.
   KOSAL_INIT_WORK(&(logging_msg_wobj(g_logging_msg)),error_logging_callback);

   kosal_queue_delayed_work( logging_msg_wq(g_logging_msg),
                             &(logging_msg_wobj(g_logging_msg)),
                             logging_msg_time(g_logging_msg));

   logging_msg_wq_status(g_logging_msg) = logging_timer_Running;


   kosal_sem_put( logging_msg_sem(g_logging_msg));

   PTRACEOUT;
   return  res;
}

///============================================================================
/// Name:    stop_logging_timer
/// @brief   stops logging timer.
///
/// @return    error code
///============================================================================
int stop_logging_timer(void)
{
   int res = 0;
   PTRACEIN;

   kosal_sem_get_krnl( logging_msg_sem(g_logging_msg));

   logging_msg_wq_status(g_logging_msg) = logging_timer_Stopped;


   // Stop logging timer work queue.
   if(NULL != logging_msg_wq(g_logging_msg)) {
         kosal_cancel_workqueue( &(logging_msg_wobj(g_logging_msg).workobj));
     }

   kosal_sem_put( logging_msg_sem(g_logging_msg));

   PTRACEOUT;
   return  res;
}

///============================================================================
/// Name:    remove_logging_timer
/// @brief   remove ccip logging polling timer.
///
/// @return    error code
///============================================================================
int remove_logging_timer(void)
{
   int res = 0;
   PTRACEIN;

   //lock
   kosal_sem_get_krnl( logging_msg_sem(g_logging_msg));

   if(NULL != logging_msg_wq(g_logging_msg)) {

      kosal_cancel_workqueue( &(logging_msg_wobj(g_logging_msg).workobj));
      kosal_destroy_workqueue(logging_msg_wq(g_logging_msg));
   }

   kosal_sem_put( logging_msg_sem(g_logging_msg));

   // unlock
   PTRACEOUT;
   return res;
}

///============================================================================
/// Name:    error_logging_callback
/// @brief   Worker queue logging timer callback.
///
/// @param[in] pwork  work queue object pointer.
/// @return    no return value
///============================================================================
void error_logging_callback(struct kosal_work_object *pwork)
{
   struct ccip_device   *pccidev    = NULL;
   struct list_head     *This       = NULL;
   struct list_head     *tmp        = NULL;

   //lock g_device_list
   //kosal_sem_get_krnl(&g_dev_list_sem);

   // Search through our list of devices to find the one matching pcidev
   if ( !kosal_list_is_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         pccidev = ccip_list_to_ccip_device(This);
         if(NULL != pccidev) {
            ccip_check_for_error(pccidev);
         }
      }
   }
   //unlock g_device_list
   //kosal_sem_put(&g_dev_list_sem);

   kosal_sem_get_krnl( logging_msg_sem(g_logging_msg));

   // Worker Queue
   KOSAL_INIT_WORK(&(logging_msg_wobj(g_logging_msg)),error_logging_callback);

   kosal_queue_delayed_work( logging_msg_wq(g_logging_msg),
                             &(logging_msg_wobj(g_logging_msg)),
                             logging_msg_time(g_logging_msg));

   kosal_sem_put( logging_msg_sem(g_logging_msg));

}

///============================================================================
/// Name:    ccip_check_for_error
/// @brief   enumerates fpga device list.
///
/// @param[in] pccipdev  ccip device pointer.
/// @return    no return value
///============================================================================
void ccip_check_for_error(struct ccip_device *pccipdev)
{
   struct port_device *pportdev        = NULL;
   struct list_head     *This          = NULL;
   struct list_head     *tmp           = NULL;

   if(NULL == pccipdev) {
      return ;
   }

   if( NULL != pccipdev->m_pfme_dev) {
      // logs fme errors
      ccip_log_fme_error(pccipdev ,pccipdev->m_pfme_dev);
      ccip_log_fme_ap_state(pccipdev ,pccipdev->m_pfme_dev);
   }


   // Search through our list of devices to find the one matching pcidev
   if ( !kosal_list_is_empty(&(pccipdev->m_portlisthead) )) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &pccipdev->m_portlisthead) {

          pportdev = cci_list_to_cci_port_device(This);

          if(NULL != pportdev) {
             ccip_log_port_error(pccipdev,pportdev);
          }
      }
   }

}

///============================================================================
/// Name:    ccip_log_fme_ap_state
/// @brief   logs fme app state change status to kernel logger.
///
/// @param[in] pccipdev  ccip device  pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
///============================================================================
void ccip_log_fme_ap_state(struct ccip_device *pccipdev,
                           struct fme_device *pfme_dev)
{

   if((ccip_fme_lasttherm(pfme_dev).ccip_tmp_threshold.thshold1_status != ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold1_status )) {

      if(0x0 == ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold_policy ) {

         PERR(" FPGA Trigger AP1 state : %s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                       ccip_dev_pcie_busnum(pccipdev),
                                                                       ccip_dev_pcie_devnum(pccipdev),
                                                                       ccip_dev_pcie_fcnnum(pccipdev));
      } else {

         PERR(" FPGA Trigger AP2 state : %s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                       ccip_dev_pcie_busnum(pccipdev),
                                                                       ccip_dev_pcie_devnum(pccipdev),
                                                                       ccip_dev_pcie_fcnnum(pccipdev));
      }
   }

   if( (ccip_fme_lasttherm(pfme_dev).ccip_tmp_threshold.thshold2_status != ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold2_status )){

      if(0x1 == ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold_policy ) {

         PERR(" FPGA Trigger AP6 state :%s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                      ccip_dev_pcie_busnum(pccipdev),
                                                                      ccip_dev_pcie_devnum(pccipdev),
                                                                      ccip_dev_pcie_fcnnum(pccipdev));
      }

      // PR with null bit stream
   }

   ccip_fme_lasttherm(pfme_dev).ccip_tmp_threshold.thshold1_status = ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold1_status ;
   ccip_fme_lasttherm(pfme_dev).ccip_tmp_threshold.thshold2_status = ccip_fme_therm(pfme_dev)->ccip_tmp_threshold.thshold2_status ;
}

///============================================================================
/// Name:    ccip_log_fme_error
/// @brief   logs fme errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
///============================================================================
void ccip_log_fme_error(struct ccip_device *pccipdev ,struct fme_device *pfme_dev)
{

   // FME Error0
   if((0x00 != ccip_fme_gerr(pfme_dev)->ccip_fme_error0.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ccip_fme_error0.csr != ccip_fme_gerr(pfme_dev)->ccip_fme_error0.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Error0 CSR: 0x%llx \n",kosal_gettimestamp(),
                                                                               ccip_dev_pcie_busnum(pccipdev),
                                                                               ccip_dev_pcie_devnum(pccipdev),
                                                                               ccip_dev_pcie_fcnnum(pccipdev),
                                                                               ccip_fme_gerr(pfme_dev)->ccip_fme_error0.csr);
   }

   // FME Error1
   if((0x00 != ccip_fme_gerr(pfme_dev)->ccip_fme_error1.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ccip_fme_error1.csr != ccip_fme_gerr(pfme_dev)->ccip_fme_error1.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Error1 CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                ccip_dev_pcie_busnum(pccipdev),
                                                                                ccip_dev_pcie_devnum(pccipdev),
                                                                                ccip_dev_pcie_fcnnum(pccipdev),
                                                                                ccip_fme_gerr(pfme_dev)->ccip_fme_error1.csr);
   }

   // FME Error2
   if((0x00 != ccip_fme_gerr(pfme_dev)->ccip_fme_error2.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ccip_fme_error2.csr != ccip_fme_gerr(pfme_dev)->ccip_fme_error2.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Error2 CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                ccip_dev_pcie_busnum(pccipdev),
                                                                                ccip_dev_pcie_devnum(pccipdev),
                                                                                ccip_dev_pcie_fcnnum(pccipdev),
                                                                                ccip_fme_gerr(pfme_dev)->ccip_fme_error2.csr);
   }

   // FME first error
   if((0x00 != ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ccip_fme_first_error.csr != ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME First Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                     ccip_dev_pcie_busnum(pccipdev),
                                                                                     ccip_dev_pcie_devnum(pccipdev),
                                                                                     ccip_dev_pcie_fcnnum(pccipdev),
                                                                                     ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr);
   }

   // FME next error
   if((0x00 != ccip_fme_gerr(pfme_dev)->ccip_fme_next_error.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ccip_fme_next_error.csr != ccip_fme_gerr(pfme_dev)->ccip_fme_next_error.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Next Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                    ccip_dev_pcie_busnum(pccipdev),
                                                                                    ccip_dev_pcie_devnum(pccipdev),
                                                                                    ccip_dev_pcie_fcnnum(pccipdev),
                                                                                    ccip_fme_gerr(pfme_dev)->ccip_fme_next_error.csr);
   }


   ccip_fme_lastgerr(pfme_dev).ccip_fme_error0.csr = ccip_fme_gerr(pfme_dev)->ccip_fme_error0.csr ;
   ccip_fme_lastgerr(pfme_dev).ccip_fme_error1.csr = ccip_fme_gerr(pfme_dev)->ccip_fme_error1.csr;
   ccip_fme_lastgerr(pfme_dev).ccip_fme_error2.csr = ccip_fme_gerr(pfme_dev)->ccip_fme_error2.csr ;

   ccip_fme_lastgerr(pfme_dev).ccip_fme_first_error.csr = ccip_fme_gerr(pfme_dev)->ccip_fme_first_error.csr ;
   ccip_fme_lastgerr(pfme_dev).ccip_fme_next_error.csr  = ccip_fme_gerr(pfme_dev)->ccip_fme_next_error.csr ;

}

///============================================================================
/// Name:    ccip_log_port_error
/// @brief   logs port errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
///============================================================================
void ccip_log_port_error(struct ccip_device *pccipdev ,struct port_device *pport_dev)
{

   // Port Error
   if((0x00 != ccip_port_err(pport_dev)->ccip_port_error.csr) &&
      (ccip_port_lasterr(pport_dev).ccip_port_error.csr != ccip_port_err(pport_dev)->ccip_port_error.csr )) {

      PERR(" PORT Error occurred:%s B:D.F = %x:%x.%x  PORT Error CSR: 0x%llx \n", kosal_gettimestamp(),
                                                                                  ccip_dev_pcie_busnum(pccipdev),
                                                                                  ccip_dev_pcie_devnum(pccipdev),
                                                                                  ccip_dev_pcie_fcnnum(pccipdev),
                                                                                  ccip_port_err(pport_dev)->ccip_port_error.csr);
   }

   // Port First Error
   if((0x00 != ccip_port_err(pport_dev)->ccip_port_first_error.csr) &&
      (ccip_port_lasterr(pport_dev).ccip_port_first_error.csr != ccip_port_err(pport_dev)->ccip_port_first_error.csr )) {

      PERR(" PORT Error occurred%s B:D.F = %x:%x.%x  PORT First Error CSR: 0x%llx \n",kosal_gettimestamp(),
                                                                                      ccip_dev_pcie_busnum(pccipdev),
                                                                                      ccip_dev_pcie_devnum(pccipdev),
                                                                                      ccip_dev_pcie_fcnnum(pccipdev),
                                                                                      ccip_port_err(pport_dev)->ccip_port_first_error.csr);
   }

   // Port malformed request
   if((0x00 != ( ccip_port_err(pport_dev)->ccip_port_malformed_req_0.csr) + ( ccip_port_err(pport_dev)->ccip_port_malformed_req_1.csr)) &&
      ((ccip_port_lasterr(pport_dev).ccip_port_malformed_req_0.csr  + ccip_port_lasterr(pport_dev).ccip_port_malformed_req_1.csr ) !=
        ccip_port_err(pport_dev)->ccip_port_malformed_req_0.csr + ccip_port_err(pport_dev)->ccip_port_malformed_req_1.csr ))  {

      PERR(" PORT Error occurred;%s B:D.F = %x:%x.%x PORT Malfromed req lsb CSR:0x%llx  msb CSR:0x%llx \n",kosal_gettimestamp(),
                                                                                                           ccip_dev_pcie_busnum(pccipdev),
                                                                                                           ccip_dev_pcie_devnum(pccipdev),
                                                                                                           ccip_dev_pcie_fcnnum(pccipdev),
                                                                                                           ccip_port_err(pport_dev)->ccip_port_malformed_req_0.csr ,
                                                                                                           ccip_port_err(pport_dev)->ccip_port_malformed_req_1.csr);


   }

   ccip_port_lasterr(pport_dev).ccip_port_error.csr       = ccip_port_err(pport_dev)->ccip_port_error.csr ;
   ccip_port_lasterr(pport_dev).ccip_port_first_error.csr = ccip_port_err(pport_dev)->ccip_port_first_error.csr ;

   ccip_port_lasterr(pport_dev).ccip_port_malformed_req_0.csr = ccip_port_err(pport_dev)->ccip_port_malformed_req_0.csr ;
   ccip_port_lasterr(pport_dev).ccip_port_malformed_req_1.csr = ccip_port_err(pport_dev)->ccip_port_malformed_req_1.csr ;

}

///============================================================================
/// Name:    ccip_log_pr_error
/// @brief   logs pr errors to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void ccip_log_pr_error(struct fme_device *pfme_dev )
{
   struct CCIP_FME_DFL_PR   *pPR_DFH  = NULL;

   PTRACEIN;

   if(NULL == pfme_dev) {
       return ;
   }

   pPR_DFH   = ccip_fme_pr(pfme_dev);

   // PR status bit set
   if(0x1 == pPR_DFH->ccip_fme_pr_status.pr_status)  {

      // Logging PR Host Status
      switch( pPR_DFH->ccip_fme_pr_status.pr_host_status)
      {
        case  CCIP_PORT_PR_Idle:
           PERR(" PR Host Status: Idle,Waiting for PR start.\n");
           break;

        case  CCIP_PORT_PR_RecStart:
           PERR(" PR Host Status: Received PR Start, Checking initial condition. \n");
           break;

        case  CCIP_PORT_PR_ReSet:
           PERR(" PR Host Status: Reset and Freeze port. \n");
           break;

        case  CCIP_PORT_WaitFreeze:
           PERR(" PR Host Status: Wait for Freeze Propagation. \n");
           break;

        case  CCIP_PORT_WaitPR:
           PERR(" PR Host Status: Wait for PR data \n");
           break;

        case  CCIP_PORT_SendFrst_Data:
           PERR(" PR Host Status: Send First data to PR. \n");
           break;

        case  CCIP_PORT_WaitPRReady:
           PERR(" PR Host Status: Wait for PR IP Ready.\n");
           break;

        case  CCIP_PORT_PushFIFO_IP:
           PERR(" PR Host Status: Push Data from FIFO to IP. \n");
           break;

        case  CCIP_PORT_WaitPR_Resp:
           PERR(" PR Host Status: Wait for PE IP Response. \n");
           break;

        case  CCIP_PORT_PR_Complete:
           PERR(" PR Host Status: Completion State. \n");
           break;

        case  CCIP_PORT_PR_UnFreeze:
           PERR(" PR Host Status: UnFreeze AFU .\n");
           break;

        case  CCIP_PORT_PR_DeAssert:
           PERR(" PR Host Status: De-Assert PR request bit. \n");
           break;

      }

      // Logging PR Controller Block Status
      switch( pPR_DFH->ccip_fme_pr_status.pr_contoller_status)
      {
        case  CCIP_PR_CLB_pwrup:
           PERR(" PR Controller Block: Power-up or nreset asserted\n");
           break;
        case  CCIP_PR_CLB_error:
           PERR(" PR Controller Block: Error was triggered \n");
           break;

        case  CCIP_PR_CLB_crc_err:
           PERR(" PR Controller Block: CRC error triggered \n");
           break;

        case  CCIP_PR_CLB_Incomp_bts_err:
           PERR(" PR Controller Block: Incompatible bitstream error detected \n");
           break;

        case  CCIP_PR_CLB_opr_inPros:
           PERR(" PR Controller Block: PR Operation passed. \n");
           break;
      }

      // Logging PR Errors
      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_operation_err ) {
         PERR(" PR PR Operation Error  Detected \n");
      }

      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_CRC_err ) {
         PERR(" PR CRC Error Detected \n");
      }

      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_bitstream_err ) {
         PERR(" PR Incomparable bitstream Error  Detected \n");
      }

      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_IP_err ) {
         PERR(" PR IP Protocol Error Detected \n");
      }

      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_FIFIO_err ) {
         PERR(" PR  FIFO Overflow Error Detected \n");
      }

      if(0x1 == pPR_DFH->ccip_fme_pr_err.PR_timeout_err ) {
         PERR(" PR Timeout  Error Detected \n");
      }

   }

   PTRACEOUT;
}

///============================================================================
/// Name:    ccip_log_bad_vkey
/// @brief   logs bad v-key value.
///
/// @param[in] no input parameter.
/// @return    no return value
///============================================================================
void ccip_log_bad_vkey(void)
{

   PTRACEIN;


   PTRACEOUT;
}


END_NAMESPACE(AAL)
