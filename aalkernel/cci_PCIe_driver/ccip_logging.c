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
         if(NULL != ccip_list_to_ccip_device(This)) {
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
      ccip_log_fme_ras_error(pccipdev ,pccipdev->m_pfme_dev);
      //ccip_log_fme_ap_state(pccipdev ,pccipdev->m_pfme_dev);
   }


   // Search through our list of devices to find the one matching pcidev
   if ( !kosal_list_is_empty(&(pccipdev->m_portlisthead) )) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &pccipdev->m_portlisthead) {

          pportdev = cci_list_to_cci_port_device(This);

          if(NULL != cci_list_to_cci_port_device(This)) {
             ccip_log_port_apstates(pccipdev,pportdev);
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

         PERR(" FPGA Trigger AP2 state : %s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                       ccip_dev_pcie_busnum(pccipdev),
                                                                       ccip_dev_pcie_devnum(pccipdev),
                                                                       ccip_dev_pcie_fcnnum(pccipdev));
      } else {

         PERR(" FPGA Trigger AP1 state : %s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
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
   if((0x00 != ccip_fme_gerr(pfme_dev)->fme_err.csr) &&
      (ccip_fme_lastgerr(pfme_dev).fme_err.csr != ccip_fme_gerr(pfme_dev)->fme_err.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Error0 CSR: 0x%llx \n",kosal_gettimestamp(),
                                                                               ccip_dev_pcie_busnum(pccipdev),
                                                                               ccip_dev_pcie_devnum(pccipdev),
                                                                               ccip_dev_pcie_fcnnum(pccipdev),
                                                                               ccip_fme_gerr(pfme_dev)->fme_err.csr);
      log_verbose_fme_error(pfme_dev);
   }

   // FME PCIe0 Error
   if((0x00 != ccip_fme_gerr(pfme_dev)->pcie0_err.csr) &&
      (ccip_fme_lastgerr(pfme_dev).pcie0_err.csr != ccip_fme_gerr(pfme_dev)->pcie0_err.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x PCIe0 Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                 ccip_dev_pcie_busnum(pccipdev),
                                                                                 ccip_dev_pcie_devnum(pccipdev),
                                                                                 ccip_dev_pcie_fcnnum(pccipdev),
                                                                                 ccip_fme_gerr(pfme_dev)->pcie0_err.csr);
      log_verbose_fme_pcie0error(pfme_dev);
   }

   // FME PCIe1 Error
   if((0x00 != ccip_fme_gerr(pfme_dev)->pcie1_err.csr) &&
      (ccip_fme_lastgerr(pfme_dev).pcie1_err.csr != ccip_fme_gerr(pfme_dev)->pcie1_err.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x PCIe1 Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                 ccip_dev_pcie_busnum(pccipdev),
                                                                                 ccip_dev_pcie_devnum(pccipdev),
                                                                                 ccip_dev_pcie_fcnnum(pccipdev),
                                                                                 ccip_fme_gerr(pfme_dev)->pcie1_err.csr);
      log_verbose_fme_pcie1error(pfme_dev);
   }

   // FME first error
   if((0x00 != ccip_fme_gerr(pfme_dev)->fme_first_err.csr) &&
      (ccip_fme_lastgerr(pfme_dev).fme_first_err.csr != ccip_fme_gerr(pfme_dev)->fme_first_err.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME First Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                     ccip_dev_pcie_busnum(pccipdev),
                                                                                     ccip_dev_pcie_devnum(pccipdev),
                                                                                     ccip_dev_pcie_fcnnum(pccipdev),
                                                                                     ccip_fme_gerr(pfme_dev)->fme_first_err.csr);
   }

   // FME next error
   if((0x00 != ccip_fme_gerr(pfme_dev)->fme_next_err.csr) &&
      (ccip_fme_lastgerr(pfme_dev).fme_next_err.csr != ccip_fme_gerr(pfme_dev)->fme_next_err.csr )) {

      PERR(" FME Error occurred:%s B:D.F = %x:%x.%x FME Next Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                    ccip_dev_pcie_busnum(pccipdev),
                                                                                    ccip_dev_pcie_devnum(pccipdev),
                                                                                    ccip_dev_pcie_fcnnum(pccipdev),
                                                                                    ccip_fme_gerr(pfme_dev)->fme_next_err.csr);
   }


   ccip_fme_lastgerr(pfme_dev).fme_err.csr = ccip_fme_gerr(pfme_dev)->fme_err.csr ;
   ccip_fme_lastgerr(pfme_dev).pcie0_err.csr = ccip_fme_gerr(pfme_dev)->pcie0_err.csr;
   ccip_fme_lastgerr(pfme_dev).pcie1_err.csr = ccip_fme_gerr(pfme_dev)->pcie1_err.csr ;

   ccip_fme_lastgerr(pfme_dev).fme_first_err.csr = ccip_fme_gerr(pfme_dev)->fme_first_err.csr ;
   ccip_fme_lastgerr(pfme_dev).fme_next_err.csr  = ccip_fme_gerr(pfme_dev)->fme_next_err.csr ;

}


///============================================================================
/// Name:    ccip_log_fme_ras_error
/// @brief   logs fme RAS errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
///============================================================================
void ccip_log_fme_ras_error(struct ccip_device *pccipdev ,struct fme_device *pfme_dev)
{

   // RAS Green bitstream Error
   if((0x00 != ccip_fme_gerr(pfme_dev)->ras_gerr.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ras_gerr.csr != ccip_fme_gerr(pfme_dev)->ras_gerr.csr )) {

      PERR(" RAS Green bitstream Error occurred:%s B:D.F = %x:%x.%x RAS GBS Error CSR: 0x%llx \n",kosal_gettimestamp(),
                                                                                                  ccip_dev_pcie_busnum(pccipdev),
                                                                                                  ccip_dev_pcie_devnum(pccipdev),
                                                                                                  ccip_dev_pcie_fcnnum(pccipdev),
                                                                                                  ccip_fme_gerr(pfme_dev)->ras_gerr.csr);
      log_verbose_fme_rasgbserror(pfme_dev);
   }

   // RAS Blue bitstream Error
   if((0x00 != ccip_fme_gerr(pfme_dev)->ras_berror.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ras_berror.csr != ccip_fme_gerr(pfme_dev)->ras_berror.csr )) {

      PERR(" RAS Blue bitstream Error occurred:%s B:D.F = %x:%x.%x RAS BBS Error CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                                  ccip_dev_pcie_busnum(pccipdev),
                                                                                                  ccip_dev_pcie_devnum(pccipdev),
                                                                                                  ccip_dev_pcie_fcnnum(pccipdev),
                                                                                                  ccip_fme_gerr(pfme_dev)->ras_berror.csr);
      log_verbose_fme_rasbbserror(pfme_dev);
   }

   // FME Warning
   if((0x00 != ccip_fme_gerr(pfme_dev)->ras_warnerror.csr) &&
      (ccip_fme_lastgerr(pfme_dev).ras_warnerror.csr != ccip_fme_gerr(pfme_dev)->ras_warnerror.csr )) {

      PERR(" RAS Warning Error occurred:%s B:D.F = %x:%x.%x RAS Warning CSR:  0x%llx \n",kosal_gettimestamp(),
                                                                                         ccip_dev_pcie_busnum(pccipdev),
                                                                                         ccip_dev_pcie_devnum(pccipdev),
                                                                                         ccip_dev_pcie_fcnnum(pccipdev),
                                                                                         ccip_fme_gerr(pfme_dev)->ras_warnerror.csr);
      log_verbose_fme_raswarnerror(pfme_dev);
   }



   ccip_fme_lastgerr(pfme_dev).ras_gerr.csr = ccip_fme_gerr(pfme_dev)->ras_gerr.csr ;
   ccip_fme_lastgerr(pfme_dev).ras_berror.csr = ccip_fme_gerr(pfme_dev)->ras_berror.csr;
   ccip_fme_lastgerr(pfme_dev).ras_warnerror.csr = ccip_fme_gerr(pfme_dev)->ras_warnerror.csr ;

}

///============================================================================
/// Name:    ccip_log_port_apstates
/// @brief   logs AP states to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
///============================================================================
void ccip_log_port_apstates(struct ccip_device *pccipdev ,struct port_device *pport_dev)
{

   // Trigger AP6 State
   if((0x00 != ccip_port_err(pport_dev)->ccip_port_error.csr) &&
      (ccip_port_lasterr(pport_dev).ccip_port_error.ap6_event != ccip_port_err(pport_dev)->ccip_port_error.ap6_event )) {

      PERR(" FPGA Trigger AP6 state :%s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                   ccip_dev_pcie_busnum(pccipdev),
                                                                   ccip_dev_pcie_devnum(pccipdev),
                                                                   ccip_dev_pcie_fcnnum(pccipdev));
   }

   // Trigger AP1 State
   if((0x00 != ccip_port_hdr(pport_dev)->ccip_port_status.csr) &&
      (ccip_port_laststatus(pport_dev).ap1_event != ccip_port_hdr(pport_dev)->ccip_port_status.ap1_event )) {

      PERR(" FPGA Trigger AP1 state :%s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                   ccip_dev_pcie_busnum(pccipdev),
                                                                   ccip_dev_pcie_devnum(pccipdev),
                                                                   ccip_dev_pcie_fcnnum(pccipdev));
   }

   // Trigger AP2 State
   if((0x00 != ccip_port_hdr(pport_dev)->ccip_port_status.csr) &&
      (ccip_port_laststatus(pport_dev).ap2_event != ccip_port_hdr(pport_dev)->ccip_port_status.ap2_event )) {

      PERR(" FPGA Trigger AP2 state :%s for B:D.F = %x:%x.%x  \n", kosal_gettimestamp(),
                                                                   ccip_dev_pcie_busnum(pccipdev),
                                                                   ccip_dev_pcie_devnum(pccipdev),
                                                                   ccip_dev_pcie_fcnnum(pccipdev));
   }

   // Save Port Error and Status CSR
   ccip_port_lasterr(pport_dev).ccip_port_error.csr       = ccip_port_err(pport_dev)->ccip_port_error.csr ;
   ccip_port_laststatus(pport_dev).csr                    = ccip_port_hdr(pport_dev)->ccip_port_status.csr ;


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
      log_verbose_port_error(pport_dev);
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

   if( 0x00 != ccip_port_err(pport_dev)->ccip_port_error.csr)  {

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
/// Name:    log_verbose_fme_error
/// @brief   logs fme errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_error(struct fme_device *pfme_dev)
{
   struct CCIP_FME_ERROR0  fme_error0  ;
   if(NULL == pfme_dev){
      return ;
   }

   fme_error0.csr  = (ccip_fme_gerr(pfme_dev)->fme_err.csr ) &
                     (~ccip_fme_lastgerr(pfme_dev).fme_err.csr);

   // FME Error
   if(fme_error0.fabric_err) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_FAB);
   }

   if(fme_error0.fabFifo_uoflow) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_FAB_UNDEROVERFLOW);
   }

   if(fme_error0.pcie0_poison_detected) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_PCIE0_POISON_DETECT);
   }

   if(fme_error0.pcie1_poison_detected) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_PCIE1_POISON_DETECT);
   }

   if(fme_error0.iommu_parity_error) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_IOMMU_PARITY);
   }

   if(fme_error0.afuerr_access_mismatch) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_AFUMISMATCH_DETECT);
   }

   if(fme_error0.mbp_event) {
      PERR(" FME Error: %s \n",AAL_ERR_FME_MBPEVENT);
   }
}

///============================================================================
/// Name:    log_verbose_fme_pcie0error
/// @brief   logs pcie0 errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_pcie0error(struct fme_device *pfme_dev)
{
   struct CCIP_FME_PCIE0_ERROR pcie0_error ;
   if(NULL == pfme_dev){
      return ;
   }

   pcie0_error.csr  = (ccip_fme_gerr(pfme_dev)->pcie0_err.csr ) &
                      (~ccip_fme_lastgerr(pfme_dev).pcie0_err.csr);

   // PCIe0 Error
   if(pcie0_error.formattype_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_FORMAT);
   }

   if(pcie0_error.MWAddr_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_MWADDR);
   }

   if(pcie0_error.MWAddrLength_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_MWLEN);
   }

   if(pcie0_error.MRAddr_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_MRADDR);
   }

   if(pcie0_error.MRAddrLength_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_MRLEN);
   }

   if(pcie0_error.cpl_tag_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_COMPTAG);
   }

   if(pcie0_error.cpl_status_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_COMPSTAT);
   }

   if(pcie0_error.cpl_timeout_err) {
      PERR(" PCIe0 Error: %s \n",AAL_ERR_PCIE0_TIMEOUT);
   }

}

///============================================================================
/// Name:    log_verbose_fme_pcie1error
/// @brief   logs pcie1 errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_pcie1error(struct fme_device *pfme_dev)
{
   struct CCIP_FME_PCIE1_ERROR pcie1_error ;
   if(NULL == pfme_dev){
      return ;
   }

   pcie1_error.csr  = (ccip_fme_gerr(pfme_dev)->pcie1_err.csr ) &
                      (~ccip_fme_lastgerr(pfme_dev).pcie1_err.csr);

   // PCIe1 Error
   if(pcie1_error.formattype_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_FORMAT);
   }

   if(pcie1_error.MWAddr_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_MWADDR);
   }

   if(pcie1_error.MWAddrLength_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_MWLEN);
   }

   if(pcie1_error.MRAddr_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_MRADDR);
   }

   if(pcie1_error.MRAddrLength_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_MRLEN);
   }

   if(pcie1_error.cpl_tag_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_COMPTAG);
   }

   if(pcie1_error.cpl_status_err) {
      PERR(" PCIe1 Error: %s \n",AAL_ERR_PCIE1_COMPSTAT);
   }

}

///============================================================================
/// Name:    log_verbose_fme_rasgbserror
/// @brief   logs ras gbs errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_rasgbserror(struct fme_device *pfme_dev)
{
   struct CCIP_FME_RAS_GERROR  ras_gerr ;
   if(NULL == pfme_dev){
      return ;
   }

   ras_gerr.csr  = (ccip_fme_gerr(pfme_dev)->ras_gerr.csr ) &
                   (~ccip_fme_lastgerr(pfme_dev).ras_gerr.csr);

   // RAS Green bitstream
   if(ras_gerr.temp_trash_ap1) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_TEMPAP1);
   }

   if(ras_gerr.temp_trash_ap2) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_TEMPAP2);
   }

   if(ras_gerr.pcie_error) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_PCIE);
   }

   if(ras_gerr.afufatal_error) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_AFUFATAL);
   }

   if(ras_gerr.afu_access_mismatch) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_AFUACCESS_MODE);
   }

   if(ras_gerr.pcie_poison_error) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_PCIEPOISON);
   }

   if(ras_gerr.gb_crc_err) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_GBCRC);
   }

   if(ras_gerr.temp_trash_ap6) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_TEMPAP6);
   }

   if(ras_gerr.power_trash_ap1) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_POWERAP1);
   }

   if(ras_gerr.power_trash_ap2) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_POWERAP2);
   }

   if(ras_gerr.mbp_error) {
      PERR(" RAS GBS Error: %s \n",AAL_ERR_RAS_MDP);
   }


}

///============================================================================
/// Name:    log_verbose_fme_rasbbserror
/// @brief   logs ras bbs errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_rasbbserror(struct fme_device *pfme_dev)
{
   struct CCIP_FME_RAS_BERROR ras_berror ;
   if(NULL == pfme_dev){
      return ;
   }

   ras_berror.csr  = (ccip_fme_gerr(pfme_dev)->ras_berror.csr ) &
                     (~ccip_fme_lastgerr(pfme_dev).ras_berror.csr);


   // RAS Blue bitstream Error
   if(ras_berror.ktilink_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_GBCRC);
   }

   if(ras_berror.tagcch_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_TAGCCH_FATAL);
   }

   if(ras_berror.cci_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_CCI_FATAL);
   }

   if(ras_berror.ktiprpto_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_KTIPROTO_FATAL);
   }

   if(ras_berror.dma_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_DMA_FATAL);
   }

   if(ras_berror.iommu_fatal_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_IOMMU_FATAL);
   }

   if(ras_berror.iommu_catast_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_IOMMU_CATAS);
   }

   if(ras_berror.crc_catast_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_RAS_CRC_CATAS);
   }

   if(ras_berror.therm_catast_err) {
      PERR(" RAS BBS Error: %s \n",AAL_ERR_PCIE1_COMPSTAT);
   }

}

///============================================================================
/// Name:    log_verbose_fme_raswarnerror
/// @brief   logs ras warning errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
///============================================================================
void log_verbose_fme_raswarnerror(struct fme_device *pfme_dev)
{
   struct CCIP_FME_RAS_WARNERROR ras_warnerror;
   if(NULL == pfme_dev){
      return ;
   }

   ras_warnerror.csr  = (ccip_fme_gerr(pfme_dev)->ras_warnerror.csr ) &
                        (~ccip_fme_lastgerr(pfme_dev).ras_warnerror.csr);

   if(ras_warnerror.event_warn_err) {
      PERR(" RAS Warning  Error: %s \n",AAL_ERR_RAS_WARNING);
   }

}

///============================================================================
/// Name:    log_verbose_port_error
/// @brief   logs port errors description to kernel logger.
///
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
///============================================================================
void log_verbose_port_error(struct port_device *pport_dev)
{
   struct CCIP_PORT_ERROR port_error ;
   if(NULL == pport_dev){
      return ;
   }

   port_error.csr  = (~ccip_port_lasterr(pport_dev).ccip_port_error.csr ) &
                     ( ccip_port_err(pport_dev)->ccip_port_error.csr);

   if(port_error.tx_ch0_overflow) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH0_OVERFLOW);
   }

   if(port_error.tx_ch0_invalidreq) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH0_INVALIDREQ);
   }

   if(port_error.tx_ch0_req_cl_len3) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH0_REQ_CL_LEN3);
   }

   if(port_error.tx_ch0_req_cl_len2) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH0_REQ_CL_LEN2);
   }

   if(port_error.afummio_rdrecv_portreset) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_AFUMMIO_RDRECV_PORTRESET);
   }

   if(port_error.afummio_wrrecv_portreset) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_AFUMMIO_WRRECV_PORTRESET);
   }

   if(port_error.tx_ch0_req_cl_len4) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH0_REQ_CL_LEN4);
   }

   if(port_error.tx_ch1_overflow) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_OVERFLOW);
   }

   if(port_error.tx_ch1_invalidreq) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_INVALIDREQ);
   }

   if(port_error.tx_ch1_req_cl_len3) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_REQ_CL_LEN3);
   }

   if(port_error.tx_ch1_req_cl_len2) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_REQ_CL_LEN2);
   }

   if(port_error.tx_ch1_req_cl_len4) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_REQ_CL_LEN4);
   }

   if(port_error.tx_ch1_insuff_datapayload) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_INSUFF_DATAPYL);
   }

   if(port_error.tx_ch1_datapayload_overrun) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_DATAPYL_OVERRUN);
   }

   if(port_error.tx_ch1_incorr_addr) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_INCORR_ADDR);
   }

   if(port_error.tx_ch1_sop_detcted) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH1_SOP_DETECTED);
   }

   if(port_error.mmioread_timeout) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_MMIOREAD_TIMEOUT);
   }

   if(port_error.tx_ch2_fifo_overflow) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_TX_CH2_FIFO_OVERFLOW);
   }

   if(port_error.unexp_mmio_resp) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_UNEXP_MMIORESP);
   }

   if(port_error.num_pending_req_overflow) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_NUM_PENDREQ_OVERFLOW);
   }

   if(port_error.llpr_smrr_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_LLPR_SMRR);
   }

   if(port_error.llpr_smrr2_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_LLPR_SMRR2);
   }

   if(port_error.llpr_mesg_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_LLPR_MSG);
   }

   if(port_error.genport_range_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_GENPORT_RANGE);
   }

   if(port_error.legrange_low_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_LEGRANGE_LOW);
   }

   if(port_error.legrange_hight_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_LEGRANGE_HIGH);
   }

   if(port_error.vgmem_range_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_VGAMEM_RANGE);
   }

   if(port_error.page_fault_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_PAGEFAULT);
   }

   if(port_error.pmr_err) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_PMRERROR);
   }

   if(port_error.ap6_event) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_AP6EVENT);
   }

   if(port_error.vfflr_accesseror) {
      PERR(" PORT Error: %s \n",AAL_ERR_PORT_VFFLR_ACCESS);
   }

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
