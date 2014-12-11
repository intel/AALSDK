//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2014, Intel Corporation.
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
//  Copyright(c) 2011-2014, Intel Corporation.
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
//        FILE: kernver-utils.h
//     CREATED: 09/20/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  Contain general version-specific kernel hacks and utilities
//
// HISTORY:  Derived from various header code written by Joe Grecco
//
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 09/21/2011     HM       Added kernel-version specific code
//****************************************************************************
#if 0
#ifndef _KERNVER_UTILS_H_
#define _KERNVER_UTILS_H_


//=============================================================================
//=============================================================================
// SEMAPHORES
//=============================================================================
//=============================================================================
#include <asm/uaccess.h>   // semaphore

// Removed in kernel 2.26.37
#ifndef init_MUTEX
   #define init_MUTEX(sem) sema_init(sem,1)
#endif

// Removed in kernel 2.26.37
#ifndef DECLARE_MUTEX
   #define  DECLARE_MUTEX(sem)  DEFINE_SEMAPHORE(sem)
#endif

//=============================================================================
//=============================================================================
// VERSION DEPENDENT CODE
//=============================================================================
//=============================================================================
#include <linux/version.h>    // LINUX_VERSION_CODE

//=============================================================================
// DEVICE_CREATE
//=============================================================================
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
	#define DEVICE_CREATE(pclass, pparent, devt, pdata, fmt...) \
		device_create( (pclass), (pparent), (devt), (pdata), fmt)
#else
	#define DEVICE_CREATE(pclass, pparent, devt, pdata, fmt...) \
		device_create( (pclass), (pparent), (devt), fmt)
#endif

#endif // _KERNVER_UTILS_H_
#endif

