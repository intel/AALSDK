//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2016, Intel Corporation.
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
//  Copyright(c) 2011-2016, Intel Corporation.
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
/// @file ccip_perfmon_linux.h
/// @brief  Definitions for ccip performance counter.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_perfmon.h
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel Corporation
//              Joseph Grecco, Intel Corporation
// 
// PURPOSE:
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/19/2016     JG       Refactored OS specific portion
//****************************************************************************///
#ifndef __AALKERNEL_CCIP_PERFMON_LINUX_H_
#define __AALKERNEL_CCIP_PERFMON_LINUX_H_

#include <ccip_perfmon.h>


/// Name:    create_perfmonitor
/// @brief   creates performance monitor
///
/// @param[in] ppcidev  pci device  pointer.
/// @param[in] pfme_dev fme device pointer.
/// @return    error code
bt32bitInt create_perfmonitor(kosal_pci_dev* ppcidev,
                              struct fme_device* pfme_dev);

/// Name:    remove_perfmonitor
/// @brief   removes perfoemanceee counters
///
/// @param[in] ppcidev  pci device  pointer.
/// @return    error code
bt32bitInt remove_perfmonitor(kosal_pci_dev* ppcidev);



#endif //__AALKERNEL_CCIP_PERFMON_LINUX_H_
