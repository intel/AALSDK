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

// FME Error definitions
#define AAL_ERR_FME_FAB                        "Fabric Error"
#define AAL_ERR_FME_FAB_UNDEROVERFLOW          "Fabric FIFO Under / Overflow error"
#define AAL_ERR_FME_PCIE0_POISON_DETECT        "PCIe0 Poison Detected"
#define AAL_ERR_FME_PCIE1_POISON_DETECT        "PCIe1 Poison Detected"
#define AAL_ERR_FME_IOMMU_PARITY               "IOMMU Parity Error"
#define AAL_ERR_FME_AFUMISMATCH_DETECT         "AFU PF/VF Access Mismatch detected"
#define AAL_ERR_FME_MBPEVENT                   "MBP error event"


#define AAL_ERR_PCIE0_FORMAT                   "PCIe0 TLP Format/type error"
#define AAL_ERR_PCIE0_MWADDR                   "PCIe0 TLP MW Address error"
#define AAL_ERR_PCIE0_MWLEN                    "PCIe0 TLP MW Length error"
#define AAL_ERR_PCIE0_MRADDR                   "PCIe0 TLP MR Address error"
#define AAL_ERR_PCIE0_MRLEN                    "PCIe0 TLP MR Length error"
#define AAL_ERR_PCIE0_COMPTAG                  "PCIe0 TLP CPL TAP error"
#define AAL_ERR_PCIE0_COMPSTAT                 "PCIe0 TLP CPL Status error"
#define AAL_ERR_PCIE0_TIMEOUT                  "PCIe0 TLP CPL Timeout error"


#define AAL_ERR_PCIE1_FORMAT                   "PCIe1 TLP Format/type error"
#define AAL_ERR_PCIE1_MWADDR                   "PCIe1 TLP MW Address error"
#define AAL_ERR_PCIE1_MWLEN                    "PCIe1 TLP MW Length error"
#define AAL_ERR_PCIE1_MRADDR                   "PCIe1 TLP MR Address error"
#define AAL_ERR_PCIE1_MRLEN                    "PCIe1 TLP MR Length error"
#define AAL_ERR_PCIE1_COMPTAG                  "PCIe1 TLP CPL TAP error"
#define AAL_ERR_PCIE1_COMPSTAT                 "PCIe1 TLP CPL Status error"
#define AAL_ERR_PCIE1_TIMEOUT                  "PCIe1 TLP CPL Timeout error"

#define AAL_ERR_PCIE_PHYFUNCERROR              "Physical function error"
#define AAL_ERR_PCIE_VIRTFUNCERROR             "Virtual function error"

#define AAL_ERR_RAS_TEMPAP1                    "Thermal threshold Triggered AP1"
#define AAL_ERR_RAS_TEMPAP2                    "Thermal threshold Triggered AP2"
#define AAL_ERR_RAS_PCIE                       "PCIe Fatal Error"
#define AAL_ERR_RAS_AFUFATAL                   "AFU Fatal error has occurred in AFU port"
#define AAL_ERR_RAS_PROCHOT                    "Indicates A Proc Hot event"
#define AAL_ERR_RAS_AFUACCESS_MODE             "AFU PF/VF access mode mismatch"
#define AAL_ERR_RAS_PCIEPOISON                 "PCIe poison port  error"
#define AAL_ERR_RAS_GBCRC                      "Green bitstream CRC Error"
#define AAL_ERR_RAS_TEMPAP6                    "Thremal threshold Triggered AP6"
#define AAL_ERR_RAS_POWERAP1                   "Power threshold Triggered AP1"
#define AAL_ERR_RAS_POWERAP2                   "Power threshold Triggered AP2"
#define AAL_ERR_RAS_MDP                        "MBP error event "

#define AAL_ERR_RAS_KTILINK_FATAL              "KTI Link layer Fatal error"
#define AAL_ERR_RAS_TAGCCH_FATAL               "tag-n-cache Fatal error"
#define AAL_ERR_RAS_CCI_FATAL                  "CCI Fatal error"
#define AAL_ERR_RAS_KTIPROTO_FATAL             "KTI Protocal Fatal error"
#define AAL_ERR_RAS_DMA_FATAL                  "DMA Fatal error"
#define AAL_ERR_RAS_INJ_FATAL                  "Injected Fatal error"
#define AAL_ERR_RAS_IOMMU_FATAL                "IOMMU Fatal error"
#define AAL_ERR_RAS_IOMMU_CATAS                "Catastrophic IOMMU Error"
#define AAL_ERR_RAS_CRC_CATAS                  "Catastrophic CRC Error"
#define AAL_ERR_RAS_THER_CATAS                 "Catastrophic Thermal Error"
#define AAL_ERR_RAS_GB_FATAL                   "Green bitstream fatal event Error"
#define AAL_ERR_RAS_INJ_CATAS                  "Injected Catastrophic error"

#define AAL_ERR_RAS_WARNING                    "RAS Warning error"
// Port Error definitions
#define AAL_ERR_PORT_TX_CH0_OVERFLOW           "Tx Channel0: Overflow"
#define AAL_ERR_PORT_TX_CH0_INVALIDREQ         "Tx Channel0: Invalid request encoding"
#define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN3        "Tx Channel0: Request with cl_len3"
#define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN2        "Tx Channel0: Request with cl_len2"
#define AAL_ERR_PORT_TX_CH0_REQ_CL_LEN4        "Tx Channel0: Request with cl_len4"

#define AAL_ERR_PORT_AFUMMIO_RDRECV_PORTRESET  "AFU MMIO RD received while PORT is in reset"
#define AAL_ERR_PORT_AFUMMIO_WRRECV_PORTRESET  "AFU MMIO WR received while PORT is in reset"

#define AAL_ERR_PORT_TX_CH1_OVERFLOW           "Tx Channel1: Overflow"
#define AAL_ERR_PORT_TX_CH1_INVALIDREQ         "Tx Channel1: Invalid request encoding"
#define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN3        "Tx Channel1: Request with cl_len3"
#define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN2        "Tx Channel1: Request with cl_len2"
#define AAL_ERR_PORT_TX_CH1_REQ_CL_LEN4        "Tx Channel1: Request with cl_len4"
#define AAL_ERR_PORT_TX_CH1_INSUFF_DATAPYL     "Tx Channel1: Insufficient data payload"
#define AAL_ERR_PORT_TX_CH1_DATAPYL_OVERRUN    "Tx Channel1: Data payload overrun"
#define AAL_ERR_PORT_TX_CH1_INCORR_ADDR        "Tx Channel1: Incorrect address"
#define AAL_ERR_PORT_TX_CH1_SOP_DETECTED       "Tx Channel1: NON-Zero SOP Detected"
#define AAL_ERR_PORT_TX_CH1_ATOMIC_REQ         "Tx Channel1: Illegal VC_SEL, atomic request VLO"

#define AAL_ERR_PORT_MMIOREAD_TIMEOUT          "MMIO Read Timeout in AFU"
#define AAL_ERR_PORT_TX_CH2_FIFO_OVERFLOW      "Tx Channel2: FIFO overflow"
#define AAL_ERR_PORT_UNEXP_MMIORESP            "MMIO read response received, with no matching pending request"
#define AAL_ERR_PORT_NUM_PENDREQ_OVERFLOW      "Number of pending Requests: counter overflow"

#define AAL_ERR_PORT_LLPR_SMRR                 "Request with Address violating SMM Range"
#define AAL_ERR_PORT_LLPR_SMRR2                "Request with Address violating second SMM Range"
#define AAL_ERR_PORT_LLPR_MSG                  "Request with Address violating ME Stolen message"
#define AAL_ERR_PORT_GENPORT_RANGE             "Request with Address violating Generic protect range"
#define AAL_ERR_PORT_LEGRANGE_LOW              "Request with Address violating Legacy Range Low"
#define AAL_ERR_PORT_LEGRANGE_HIGH             "Request with Address violating Legacy Range High"
#define AAL_ERR_PORT_VGAMEM_RANGE              "Request with Address violating VGA memory range"
#define AAL_ERR_PORT_PAGEFAULT                 "Page fault"
#define AAL_ERR_PORT_PMRERROR                  "PMR Error"
#define AAL_ERR_PORT_AP6EVENT                  "AP6 Event"
#define AAL_ERR_PORT_VFFLR_ACCESS              "VF FLR detected on Port with PF access control"

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

/// Name:    ccip_log_port_apstates
/// @brief   logs AP states to kernel logger.
///
/// @param[in] pccipdev  ccip device pointer.
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
void ccip_log_port_apstates(struct ccip_device *pccipdev ,struct port_device *pport_dev);

/// Name:    log_verbose_fme_error
/// @brief   logs fme errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_error(struct fme_device *pfme_dev);

/// Name:    log_verbose_fme_pcie0error
/// @brief   logs pcie0 errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_pcie0error(struct fme_device *pfme_dev);

/// Name:    verbose_log_fme_pcie1error
/// @brief   logs pcie1 errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_pcie1error(struct fme_device *pfme_dev);

/// Name:    log_verbose_fme_rasgbserror
/// @brief   logs ras gbs errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_rasgbserror(struct fme_device *pfme_dev);

/// Name:    log_verbose_fme_rasbbserror
/// @brief   logs ras bbs errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_rasbbserror(struct fme_device *pfme_dev);

/// Name:    log_verbose_fme_raswarnerror
/// @brief   logs ras warning errors description to kernel logger.
///
/// @param[in] pfme_dev  fme device pointer.
/// @return    no return value
void log_verbose_fme_raswarnerror(struct fme_device *pfme_dev);

/// Name:    log_verbose_port_error
/// @brief   logs port errors description to kernel logger.
///
/// @param[in] pport_dev  port device pointer.
/// @return    no return value
void log_verbose_port_error(struct port_device *pport_dev);

/// Name:    ccip_log_bad_vkey
/// @brief   logs bad v-key value.
///
/// @param[in] no input parameter.
/// @return    no return value
void ccip_log_bad_vkey(void);

END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_LOOGING_DEF_H_ */
