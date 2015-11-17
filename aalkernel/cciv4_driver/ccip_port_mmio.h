//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
//  Copyright(c) 2011-2015, Intel Corporation.
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
/// @file ccip_def.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_port_mmio.h
//     CREATED: Sept 24, 2015
//      AUTHOR:
//
// PURPOSE:   This file contains the definations of the CCIP Port
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///

#ifndef __AALKERNEL_CCIP_PORT_DEF_H_
#define __AALKERNEL_CCIP_PORT_DEF_H_

#include <aalsdk/kernel/aaltypes.h>
#include "cciv4_driver_internal.h"

BEGIN_NAMESPACE(AAL)

/// @brief   reads PORT mmio region
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_mmio(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio );

/// @brief   reads PORT Header
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_header(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio );

/// @brief   reads PORT feature list
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_featurelist(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio );

/// @brief   reads PORT error  CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_err_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio);

/// @brief   reads PORT UMSG CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_umsg_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio);


/// @brief   reads PORT PR CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_pr_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio);


/// @brief   reads PORT signaltap CSR
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt get_port_stap_rev0(struct port_device *pport_dev,btVirtAddr pkvp_port_mmio);

/// @brief   freee Port feature list memory
///
/// @param[in] pport_dev port device pointer .
/// @return    void
void ccip_port_mem_free(struct port_device *pport_dev );


/// @brief   Port Quiesce Reset
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_quiesce_reset(btVirtAddr pkvp_port_mmio );

/// @brief   Reset port
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_port_reset(btVirtAddr pkvp_port_mmio );

/// @brief   Port Re Enable
///
/// @param[in] pport_dev port device pointer.
/// @param[in] pkvp_port_mmio port mmio virtual address
/// @return    error code
bt32bitInt afu_re_enable(btVirtAddr pkvp_port_mmio );

END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_PORT_DEF_H_ */
