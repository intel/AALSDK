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
/// @file ccip_perfmon.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_perfmon.h
//     CREATED: Sept 24, 2015
//      AUTHOR:
//
// PURPOSE:
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///


#ifndef __AALKERNEL_CCIP_PERFMON_DEF_H_
#define __AALKERNEL_CCIP_PERFMON_DEF_H_

#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/ccipdriver.h>
#include "cci_pcie_driver_internal.h"
#include "ccip_defs.h"

BEGIN_NAMESPACE(AAL)

#define CACHE_EVENT_COUNTER_MAX_TRY 30
#define PERF_MONITOR_VERSION 1
#define PERF_MONITOR_COUNT 11

#define PMONITOR_VERSION        "version"
#define NUM_COUNTERS            "number of counters"
#define CACHE_READ_HIT          "Read_Hit"
#define CACHE_WRITE_HIT         "Write_Hit"
#define CACHE_READ_MISS         "Read_Miss"
#define CACHE_WRITE_MISS        "Write_Miss"
#define CACHE_EVICTIONS         "Evictions"
#define FABRIC_PCIE0_READ       "PCIe 0 Read"
#define FABRIC_PCIE0_WRITE      "PCIe 0 Write"
#define FABRIC_PCIE1_READ       "PCIe 1 Read"
#define FABRIC_PCIE1_WRITE      "PCIe 1 Write"
#define FABRIC_UPI_READ         "UPI Read"
#define FABRIC_UPI_WRITE        "UPI Write"
#define VTD_COUNTER              "VT-d"


/// Name:    create_perfmonitor
/// @brief   creates performance monitor
///
/// @param[in] ppcidev  pci device  pointer.
/// @param[in] pfme_dev fme device pointer.
/// @return    error code
bt32bitInt create_perfmonitor(struct pci_dev* ppcidev,
                              struct fme_device* pfme_dev);

/// Name:    remove_perfmonitor
/// @brief   removes perfoemanceee counters
///
/// @param[in] ppcidev  pci device  pointer.
/// @return    error code
bt32bitInt remove_perfmonitor(struct pci_dev* ppcidev);


/// Name:    get_perfmonitor_counters
/// @brief   get snapshot of performance counters
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf performance counters pointer
/// @return    error code
bt32bitInt get_perfmonitor_snapshot(struct fme_device *pfme_dev,
                                    struct CCIP_PERF_COUNTERS* pPerf);

/// Name:    update_fabric_event_counters
/// @brief   get Fabric performance counters
///
/// @param[in] event_code performance counters event device pointer.
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf performance counters pointer
/// @return    error code
bt32bitInt update_fabric_event_counters(bt32bitInt event_code ,
                                       struct fme_device *pfme_dev,
                                       struct CCIP_PERF_COUNTERS* pPerf);

/// Name:    update_cache_event_counters
/// @brief   get cache performance counters
///
/// @param[in] event_code performance counters event device pointer.
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf performance counters pointer
/// @return    error code
bt32bitInt update_cache_event_counters(bt32bitInt event_code ,
                                       struct fme_device *pfme_dev,
                                       struct CCIP_PERF_COUNTERS* pPerf);

/// Name:    get_perfmon_counters
/// @brief   get  performance counters
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf performance counters pointer
/// @return    error code
bt32bitInt get_perfmon_counters(struct fme_device* pfme_dev,
                                struct CCIP_PERF_COUNTERS* pPerfCounter);
END_NAMESPACE(AAL)

#endif //__AALKERNEL_CCIP_PERFMON_DEF_H_
