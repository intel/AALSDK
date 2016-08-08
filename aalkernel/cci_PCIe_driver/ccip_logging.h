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


#ifndef __AALKERNEL_CCIP_LOOGING_DEF_H_
#define __AALKERNEL_CCIP_LOOGING_DEF_H_

#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/ccip_defs.h>
#include "cci_pcie_driver_internal.h"


BEGIN_NAMESPACE(AAL)

// logging work queue status.
typedef enum
{
   logging_timer_idle = 0x0,
   logging_timer_Running,
   logging_timer_Stopped

} logging_wq_status_e;

struct logging_msg
{
   // Logging worker queue
   kosal_work_queue                 m_workq_logging;

   // Logging work object
   struct kosal_work_object         m_workobject;

   // logging timer value
   unsigned long                    m_logging_timer_value;
   // semaphore
   kosal_semaphore                  m_logging_sem;

   logging_wq_status_e              m_logginf_wq_status;
};

#define logging_msg_wq(pdev)            ((pdev).m_workq_logging)
#define logging_msg_wobj(pdev)          ((pdev).m_workobject)
#define logging_msg_time(pdev)          ((pdev).m_logging_timer_value)
#define logging_msg_sem(pdev)           (&(pdev).m_logging_sem)
#define logging_msg_wq_status(pdev)     ((pdev).m_logginf_wq_status)

/// Name:    create_logging_timer
/// @brief   creates ccip logging polling time.
///
/// @return    error code
int create_logging_timer(void);

/// Name:    remove_logging_timer
/// @brief   remove ccip logging polling timer.
///
/// @return    error code
int remove_logging_timer(void);

/// Name:    start_logging_timer
/// @brief   starts logging timer.
///
/// @return    error code
int start_logging_timer(void);

/// Name:    stop_logging_timer
/// @brief   stops logging timer.
///
/// @return    error code
int stop_logging_timer(void);

/// Name:    error_logging_callback
/// @brief   Worker queue logging timer callback.
///
/// @param[in] pwork  work queue object pointer.
/// @return    no return value
void error_logging_callback(struct kosal_work_object *pwork);

/// Name:    ccip_log_error
/// @brief   enumerates fpga device list.
///
/// @param[in] pccipdev  ccip device pointer.
/// @return    no return value
void ccip_check_for_error(struct ccip_device *pccipdev);

/// Name:    ccip_log_fme_ap_state
/// @brief   logs fme app state change status to kernel logger.
///
/// @param[in] pccipdev  ccip device  pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
void ccip_log_fme_ap_state(struct ccip_device *pccipdev,
                           struct fme_device *pfme_dev);

/// Name:    ccip_log_fme_error
/// @brief   logs fme errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
void ccip_log_fme_error(struct ccip_device *pccipdev,
                        struct fme_device *pfme_dev);

/// Name:    ccip_log_fme_ras_error
/// @brief   logs fme RAS errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pfme_dev  fme device  pointer.
/// @return    no return value
void ccip_log_fme_ras_error(struct ccip_device *pccipdev ,
                           struct fme_device *pfme_dev);

/// Name:    ccip_log_port_error
/// @brief   logs port errors to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
void ccip_log_port_error(struct ccip_device *pccipdev,
                         struct port_device *pport_dev);

/// Name:    ccip_log_pr_error
/// @brief   logs pr errors to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void ccip_log_pr_error(struct fme_device   *pfme_dev );

/// Name:    ccip_log_bad_vkey
/// @brief   logs bad v-key value.
///
/// @param[in] no input parameter.
/// @return    no return value
void ccip_log_bad_vkey(void);

END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_LOOGING_DEF_H_ */
